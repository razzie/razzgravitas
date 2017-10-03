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

#pragma once

#include <set>
#include <raz/network.hpp>
#include <raz/networkbackend.hpp>
#include <raz/random.hpp>
#include <raz/timer.hpp>
#include "common/IApplication.hpp"

class NetworkServer
{
public:
	NetworkServer(IApplication* app, const char* cmdline);
	~NetworkServer();
	void operator()(); // loop
	void operator()(Message e);
	void operator()(GameObjectSync e);
	void operator()(SwitchPlayer e);
	void operator()(Highscore e);
	void operator()(std::exception& e);

private:
	typedef raz::NetworkServerUDP<MAX_PACKET_SIZE>::Client Client;
	typedef raz::NetworkServerBackendUDP<MAX_PACKET_SIZE>::ClientState ClientState;
	typedef raz::NetworkServerUDP<MAX_PACKET_SIZE>::ClientData<MAX_PACKET_SIZE> Data;
	typedef decltype(Data::packet) Packet;

	struct ClientComparator
	{
		bool operator()(const Client& a, const Client& b) const
		{
			return (std::memcmp(&a, &b, sizeof(Client)) < 0);
		}
	};

	raz::NetworkInitializer m_init;
	IApplication* m_app;
	raz::NetworkServerUDP<MAX_PACKET_SIZE> m_server;
	raz::Timer m_timeout;
	raz::Timer m_sync_timer;
	raz::Random m_sync_id_gen;
	Data m_data;
	std::set<Client, ClientComparator> m_clients;

	bool handlePacket(Packet& packet, const Player* sender);
	void handleHello(Client& client, Packet& packet);
	void handleConnect(Client& client);
	void handleDisconnect(Client& client);
	void handleClientTimeouts();
	const Player* getPlayer(Client& client);

	template <class T>
	static bool checkPlayer(const T& t, const void* player)
	{
		return true;
	}

	template <class T>
	static auto checkPlayer(const T& t, const Player* player) -> decltype((void)T::player_id, bool())
	{
		return (player && t.player_id == player->player_id);
	}

	template<class Event>
	bool tryHandle(Packet& packet, const Player* player)
	{
		if (packet.getType() == (raz::PacketType)Event::getEventType())
		{
			Event e;
			packet.setMode(raz::SerializationMode::DESERIALIZE);
			packet(e);

			if (!checkPlayer(e, player))
				return false;

			m_app->handle(e, EventSource::Network);
			return true;
		}

		return false;
	}
};
