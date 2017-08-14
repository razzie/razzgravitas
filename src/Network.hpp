/*
Copyright (C) 2017 - G�bor "Razzie" G�rzs�ny
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

#include <string>
#include <exception>
#include <raz/bitset.hpp>
#include <raz/network.hpp>
#include <raz/networkbackend.hpp>
#include <raz/timer.hpp>
#include "Events.hpp"
#include "Settings.hpp"

class Application;

enum NetworkMode
{
	Disabled,
	Client,
	Server
};

class Network
{
public:
	Network(Application* app, const std::string& cmdline);
	~Network();
	NetworkMode getMode() const;
	void operator()(); // loop
	void operator()(Connected e);
	void operator()(AddGameObject e);
	void operator()(RemoveGameObjects e);
	void operator()(GameObjectSync e);
	void operator()(std::exception& e);

private:
	typedef raz::NetworkServerTCP::Client Client;
	typedef raz::NetworkServerBackendTCP::ClientState ClientState;
	typedef raz::NetworkServerTCP::ClientData<sizeof(GameObjectSync)> Data;
	typedef decltype(Data::packet) Packet;

	Application* m_app;
	NetworkMode m_mode;
	raz::NetworkClientTCP m_client;
	raz::NetworkServerTCP m_server;
	raz::Timer m_timer;
	Data m_data;
	Client m_players[MAX_PLAYERS - 1];
	raz::Bitset<MAX_PLAYERS - 1> m_player_slots;

	void updateClient();
	void updateServer();
	void handlePacket(Packet& packet);
	void handleConnect(Client& client);
	void handleDisconnect(Client& client);
};
