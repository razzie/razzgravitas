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

		for (auto& client : m_clients)
		{
			m_server.send(client, packet);
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
		for (auto& client : m_clients)
		{
			m_server.send(client, packet);
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

		for (auto& client : m_clients)
		{
			m_server.send(client, packet);
		}
	}
}

void Network::operator()(SwitchPlayer e)
{
	if (m_mode == NetworkMode::Server)
	{
		Packet packet;
		packet.setType((raz::PacketType)EventType::SwitchPlayer);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		for (auto& client : m_clients)
		{
			m_server.send(client, packet);
		}
	}
	else if (m_mode == NetworkMode::Client)
	{
		Packet packet;
		packet.setType((raz::PacketType)EventType::SwitchPlayer);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		m_client.send(packet);
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
	handlePacket(m_data.packet, m_app->getPlayerManager()->getLocalPlayer());
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
		handlePacket(m_data.packet, m_app->getPlayerManager()->findPlayer(m_data.client.socket));
		break;

	case ClientState::UNSET:
		GameObjectSyncRequest e;
		e.sync_id = (uint32_t)m_sync_id_gen();
		m_app->handle(e, EventSource::Network);
		break;
	}
}

bool Network::handlePacket(Packet& packet, const Player* sender)
{
	return (tryHandle<Connected>(packet, sender)
		|| tryHandle<Disconnected>(packet, sender)
		|| tryHandle<SwitchPlayer>(packet, sender)
		|| tryHandle<Message>(packet, sender)
		|| tryHandle<AddGameObject>(packet, sender)
		|| tryHandle<RemoveGameObject>(packet, sender)
		|| tryHandle<GameObjectSync>(packet, sender));
}

void Network::handleConnect(Client& client)
{
	const Player* player = m_app->getPlayerManager()->addPlayer();
	if (player)
	{
		player->data = (int)client.socket;
		m_clients.push_back(client);

		Connected e;
		e.player_id = player->player_id;

		Packet packet;
		packet.setType((raz::PacketType)EventType::Connected);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);

		m_server.send(client, packet);
	}
	else
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
}

void Network::handleDisconnect(Client& client)
{
	const Player* player = m_app->getPlayerManager()->findPlayer(client.socket);
	if (player)
	{
		RemovePlayerGameObjects e;
		e.player_id = player->player_id;
		m_app->handle(e, EventSource::Network);

		for (auto it = m_clients.begin(), end = m_clients.end(); it != end; ++it)
		{
			if (player->data == (int)it->socket)
			{
				m_clients.erase(it);
				return;
			}
		}
		m_app->getPlayerManager()->removePlayer(player->player_id);
	}
}
