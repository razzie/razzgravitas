/*
Copyright (C) 2017 - Gábor "Razzie" Görzsöny
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE
*/

#include <ctime>
#include <cstring>
#include "Application.hpp"
#include "Network.hpp"

Network::Network(IApplication* app, NetworkMode mode, const char* host) :
	m_app(app),
	m_mode(NetworkMode::Disabled),
	m_sync_id_gen((uint64_t)std::time(NULL))
{
	if (mode == NetworkMode::Server)
	{
		if (m_server.getBackend().open(GAME_PORT))
			m_mode = NetworkMode::Server;
		else
			m_app->exit(-1, "Cannot host game");
	}
	else if (mode == NetworkMode::Client)
	{
		if (m_client.getBackend().open(host, GAME_PORT))
			m_mode = NetworkMode::Client;
		else
			m_app->exit(-1, "Cannot connect to server");
	}
	else
	{
		throw raz::ThreadStop{};
	}
}

Network::~Network()
{
	if (m_mode == NetworkMode::Server)
	{
		Disconnected e;
		e.reason = Disconnected::ServerClosed;

		Packet packet;
		packet.setType((raz::PacketType)EventType::Disconnected);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		for (auto slot : m_player_slots.truebits())
		{
			m_server.send(m_players[slot], packet);
		}

		m_server.getBackend().close();
	}
	else if (m_mode == NetworkMode::Client)
	{
		m_client.getBackend().close();
	}
}

void Network::operator()()
{
	if (m_mode == NetworkMode::Server)
		updateServer();
	else if (m_mode == NetworkMode::Client)
		updateClient();
}

void Network::operator()(Message e)
{
	Packet packet;
	packet.setType((raz::PacketType)EventType::Message);
	packet.setMode(raz::SerializationMode::SERIALIZE);
	packet(e);

	if (m_mode == NetworkMode::Server)
	{
		for (auto player_slot : m_player_slots.truebits())
		{
			m_server.send(m_players[player_slot], packet);
		}
	}
	else
	{
		m_client.send(packet);
	}
}

void Network::operator()(AddGameObject e)
{
	if (m_mode == NetworkMode::Client)
	{
		Packet packet;
		packet.setType((raz::PacketType)EventType::AddGameObject);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		m_client.send(packet);
	}
}

void Network::operator()(RemoveGameObject e)
{
	if (m_mode == NetworkMode::Client)
	{
		Packet packet;
		packet.setType((raz::PacketType)EventType::RemoveGameObject);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		m_client.send(packet);
	}
}

void Network::operator()(GameObjectSync e)
{
	if (m_mode == NetworkMode::Server)
	{
		Packet packet;
		packet.setType((raz::PacketType)EventType::GameObjectSync);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		for (auto player_slot : m_player_slots.truebits())
		{
			m_server.send(m_players[player_slot], packet);
		}
	}
}

void Network::operator()(std::exception& e)
{
	m_app->exit(-1, e.what());
}

void Network::updateClient()
{
	m_data.packet.reset();
	if (!m_client.receive(m_data.packet, GAME_SYNC_TIMEOUT))
		return;

	m_data.packet.setMode(raz::SerializationMode::DESERIALIZE);
	handlePacket(m_data.packet);
}

void Network::updateServer()
{
	m_data.packet.reset();
	m_server.receive(m_data, GAME_SYNC_TIMEOUT);

	switch (m_data.state)
	{
	case ClientState::CLIENT_CONNECTED:
		handleConnect(m_data.client);
		break;

	case ClientState::CLIENT_DISCONNECTED:
		handleDisconnect(m_data.client);
		break;

	case ClientState::PACKET_RECEIVED:
		m_data.packet.setMode(raz::SerializationMode::DESERIALIZE);
		handlePacket(m_data.packet);
		break;

	case ClientState::UNSET:
		GameObjectSyncRequest e;
		e.sync_id = (uint32_t)m_sync_id_gen();
		m_app->handle(e, EventSource::Network);
		break;
	}
}

bool Network::handlePacket(Packet& packet)
{
	return (tryHandle<Connected>(packet)
		|| tryHandle<Disconnected>(packet)
		|| tryHandle<Message>(packet)
		|| tryHandle<AddGameObject>(packet)
		|| tryHandle<RemoveGameObject>(packet)
		|| tryHandle<GameObjectSync>(packet));
}

void Network::handleConnect(Client& client)
{
	auto player_slots = m_player_slots.falsebits();
	auto slot = player_slots.begin();

	if (slot == player_slots.end())
	{
		Disconnected e;
		e.reason = Disconnected::ServerClosed;

		Packet packet;
		packet.setType((raz::PacketType)EventType::Disconnected);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		m_server.send(client, packet);
		m_server.getBackend().close(client);
	}
	else
	{
		m_players[*slot] = client;
		m_player_slots.set(*slot);

		Connected e;
		e.player_id = (uint16_t)(*slot + 1); // player 0 is this instance

		Packet packet;
		packet.setType((raz::PacketType)EventType::Connected);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		m_server.send(client, packet);
	}
}

void Network::handleDisconnect(Client& client)
{
	for (size_t slot : m_player_slots.truebits())
	{
		if (m_players[slot].socket == client.socket)
		{
			m_player_slots.unset(slot);

			RemovePlayerGameObjects e;
			e.player_id = (uint16_t)(slot + 1); // player 0 is this instance
			m_app->handle(e, EventSource::Network);

			return;
		}
	}
}
