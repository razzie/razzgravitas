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

#include "common/PlayerManager.hpp"
#include "network/NetworkClient.hpp"

NetworkClient::NetworkClient(IApplication* app, const char* cmdline) :
	m_app(app)
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

NetworkClient::~NetworkClient()
{
	m_client.getBackend().close();
}

void NetworkClient::operator()()
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

	Packet packet;
	if (!m_client.receive(packet, GAME_SYNC_RATE))
		return;

	packet.setMode(raz::SerializationMode::DESERIALIZE);
	if (handlePacket(packet))
		m_timer.reset();
}

void NetworkClient::operator()(Message e)
{
	Packet packet;
	packet.setType((raz::PacketType)EventType::Message);
	packet.setMode(raz::SerializationMode::SERIALIZE);
	packet(e);

	m_client.send(packet);
}

void NetworkClient::operator()(AddGameObject e)
{
	Packet packet;
	packet.setType((raz::PacketType)EventType::AddGameObject);
	packet.setMode(raz::SerializationMode::SERIALIZE);
	packet(e);

	m_client.send(packet);
}

void NetworkClient::operator()(RemoveGameObject e)
{
	Packet packet;
	packet.setType((raz::PacketType)EventType::RemoveGameObject);
	packet.setMode(raz::SerializationMode::SERIALIZE);
	packet(e);

	m_client.send(packet);
}

void NetworkClient::operator()(SwitchPlayer e)
{
	Packet packet;
	packet.setType((raz::PacketType)EventType::SwitchPlayer);
	packet.setMode(raz::SerializationMode::SERIALIZE);
	packet(e);

	m_client.send(packet);
}

void NetworkClient::operator()(std::exception& e)
{
	m_app->exit(-1, e.what());
}

bool NetworkClient::handlePacket(Packet& packet)
{
	return (tryHandle<Connected>(packet)
		|| tryHandle<Disconnected>(packet)
		|| tryHandle<SwitchPlayer>(packet)
		|| tryHandle<Message>(packet)
		|| tryHandle<GameObjectSync>(packet));
}
