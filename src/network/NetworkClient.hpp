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

#include "common/IApplication.hpp"
#include <raz/network.hpp>
#include <raz/networkbackend.hpp>
#include <raz/timer.hpp>

class NetworkClient
{
public:
	NetworkClient(IApplication* app, const char* cmdline);
	~NetworkClient();
	void operator()(); // loop
	void operator()(Message e);
	void operator()(AddGameObject e);
	void operator()(RemoveGameObject e);
	void operator()(SwitchPlayer e);
	void operator()(std::exception& e);

private:
	typedef raz::Packet<MAX_PACKET_SIZE> Packet;

	raz::NetworkInitializer m_init;
	IApplication* m_app;
	raz::NetworkClientUDP<MAX_PACKET_SIZE> m_client;
	raz::Timer m_timeout;

	bool handlePacket(Packet& packet);

	template<class Event>
	bool tryHandle(Packet& packet)
	{
		if (packet.getType() == (raz::PacketType)Event::getEventType())
		{
			Event e;
			packet.setMode(raz::SerializationMode::DESERIALIZE);
			packet(e);

			m_app->handle(e, EventSource::Network);
			return true;
		}

		return false;
	}
};
