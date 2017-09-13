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

static_assert(MAX_PACKET_SIZE >= sizeof(GameObjectSync), "MAX_PACKET_SIZE is too low");
static_assert(PING_RATE < CONNECTION_TIMEOUT, "PING_RATE should be lower than CONNECTION_TIMEOUT");

Network::Network(IApplication* app, NetworkMode mode, const char* cmdline) :
	m_app(app),
	m_mode(NetworkMode::Disabled),
	m_sync_id_gen((uint64_t)std::time(NULL))
{
	if (mode == NetworkMode::Server)
		startServer(cmdline);
	else if (mode == NetworkMode::Client)
		startClient(cmdline);
	else
		throw raz::ThreadStop{};
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
		const Player* player = m_app->getPlayerManager()->getPlayer(e.new_player_id);
		const Client* client = reinterpret_cast<const Client*>(player->data);
		if (client)
		{
			Packet packet;
			packet.setType((raz::PacketType)EventType::SwitchPlayer);
			packet.setMode(raz::SerializationMode::SERIALIZE);
			packet(e);

			m_server.send(*client, packet);
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

void Network::startClient(const char* cmdline)
{
	char host[256];
	std::memcpy(host, cmdline, strlen(cmdline) + 1);

	uint16_t port = GAME_PORT;
	char* port_str = strchr(host, ':');
	if (port_str)
	{
		*port_str = '\0';
		port = (uint16_t)std::stoul(++port_str);
	}

	if (m_client.getBackend().open(host, port))
	{
		m_mode = NetworkMode::Client;

		Hello e;
		e.checksum = Hello::calculate();

		Packet packet;
		packet.setType((raz::PacketType)EventType::Hello);
		packet.setMode(raz::SerializationMode::SERIALIZE);
		packet(e);
		try
		{
			m_client.send(packet);
			return;
		}
		catch (raz::NetworkSocketError&)
		{

		}
	}
	
	m_app->exit(-1, "Cannot connect to server");
}

void Network::startServer(const char* cmdline)
{
	uint16_t port = cmdline ? (uint16_t)std::stoul(cmdline) : GAME_PORT;

	if (m_server.getBackend().open(port))
	{
		m_mode = NetworkMode::Server;
		return;
	}

	m_app->exit(-1, "Cannot host game");
}

void Network::updateClient()
{
	if (m_timer.peekElapsed() > CONNECTION_TIMEOUT)
	{
		if (m_app->getPlayerManager()->getLocalPlayer())
			m_app->exit(-1, "Connection timed out");
		else
			m_app->exit(-1, "Cannot connect to server");

		return;
	}

	const Player* player = m_app->getPlayerManager()->getLocalPlayer();
	if (player)
	{
		uint64_t timeout =
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player->last_updated).count();

		if (timeout >= PING_RATE)
		{
			player->last_updated = std::chrono::steady_clock::now();

			Packet packet;
			packet.setType((raz::PacketType)EventType::Ping);
			m_client.send(packet);
		}
	}

	m_data.packet.reset();
	if (!m_client.receive(m_data.packet, GAME_SYNC_RATE))
		return;

	m_data.packet.setMode(raz::SerializationMode::DESERIALIZE);
	if (handlePacket(m_data.packet, player))
		m_timer.reset();
}

void Network::updateServer()
{
	try
	{
		m_data.packet.reset();
		m_server.receive(m_data, GAME_SYNC_RATE);

		switch (m_data.state)
		{
		case ClientState::CLIENT_UNAVAILABLE:
			handleDisconnect(m_data.client);
			break;

		case ClientState::PACKET_RECEIVED:
			m_data.packet.setMode(raz::SerializationMode::DESERIALIZE);
			if (const Player* player = getPlayer(m_data.client))
				handlePacket(m_data.packet, player);
			else
				handleHello(m_data.client, m_data.packet);
			break;
		}

		if (m_timer.peekElapsed() > GAME_SYNC_RATE)
		{
			m_timer.reset();
			handleClientTimeouts(); // dont really need to run it in every cycle

			GameObjectSyncRequest e;
			e.sync_id = (uint32_t)m_sync_id_gen();
			m_app->handle(e, EventSource::Network);
		}
	}
	catch (raz::NetworkSocketError&)
	{
	}
	catch (raz::SerializationError&)
	{
	}
}

bool Network::handlePacket(Packet& packet, const Player* sender)
{
	if (packet.getType() == (raz::PacketType)EventType::Ping && sender)
	{
		sender->last_updated = std::chrono::steady_clock::now();
		return true;
	}

	return (tryHandle<Connected>(packet, sender)
		|| tryHandle<Disconnected>(packet, sender)
		|| tryHandle<SwitchPlayer>(packet, sender)
		|| tryHandle<Message>(packet, sender)
		|| tryHandle<AddGameObject>(packet, sender)
		|| tryHandle<RemoveGameObject>(packet, sender)
		|| tryHandle<GameObjectSync>(packet, sender));
}

void Network::handleHello(Client& client, Packet& packet)
{
	if (packet.getType() == (raz::PacketType)EventType::Hello)
	{
		Hello e;
		packet.setMode(raz::SerializationMode::DESERIALIZE);
		packet(e);

		if (e.checksum == Hello::calculate())
		{
			handleConnect(client);
		}
		else
		{
			Disconnected e;
			e.reason = Disconnected::Compatibility;

			Packet packet;
			packet.setType((raz::PacketType)EventType::Disconnected);
			packet.setMode(raz::SerializationMode::SERIALIZE);
			packet(e);

			m_server.send(client, packet);
		}
	}
}

void Network::handleConnect(Client& client)
{
	const Player* player = m_app->getPlayerManager()->addPlayer();
	if (player)
	{
		auto it = m_clients.insert(client).first;
		player->data = &(*it);

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
	}
}

void Network::handleDisconnect(Client& client)
{
	const Player* player = getPlayer(client);
	if (player)
	{
		RemovePlayerGameObjects e;
		e.player_id = player->player_id;
		m_app->handle(e, EventSource::Network);

		for (auto it = m_clients.begin(), end = m_clients.end(); it != end; ++it)
		{
			if (player->data == &(*it))
			{
				m_clients.erase(it);
				return;
			}
		}
		m_app->getPlayerManager()->removePlayer(player->player_id);
	}
}

void Network::handleClientTimeouts()
{
	for (auto it = m_clients.begin(); it != m_clients.end(); )
	{
		const Player* player = m_app->getPlayerManager()->findPlayer(&(*it));
		if (player)
		{
			uint64_t timeout =
				std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - player->last_updated).count();

			if (timeout > CONNECTION_TIMEOUT)
			{
				RemovePlayerGameObjects e;
				e.player_id = player->player_id;
				m_app->handle(e, EventSource::Network);

				m_app->getPlayerManager()->removePlayer(player->player_id);
				it = m_clients.erase(it);
				continue;
			}
		}

		++it;
	}
}

const Player* Network::getPlayer(Client& client)
{
	auto it = m_clients.find(client);
	if (it == m_clients.end())
		return nullptr;

	return m_app->getPlayerManager()->findPlayer(&(*it));
}
