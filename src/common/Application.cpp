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

#include "common/Application.hpp"
#include "common/Config.hpp"
#include <codecvt>
#include <Windows.h>

static_assert(MAX_PACKET_SIZE >= sizeof(GameObjectSync), "MAX_PACKET_SIZE is too low");
static_assert(PING_RATE < CONNECTION_TIMEOUT, "PING_RATE should be lower than CONNECTION_TIMEOUT");
static_assert(GAME_SYNC_RATE < CONNECTION_TIMEOUT, "GAME_SYNC_RATE should be lower than CONNECTION_TIMEOUT");

int Application::run(int argc, char** argv)
{
	return Application(argc, argv).run();
}

Application::Application(int argc, char** argv) :
	m_mode(GameMode::SingplePlay)
{
	if (argc > 1)
	{
		m_cmdline = argv[1];
		m_mode = GameMode::Client;
	}
}

Application::~Application()
{
}

GameMode Application::getGameMode() const
{
	return m_mode;
}

int Application::run()
{
	auto exit_info_future = m_exit.get_future();

	setGameMode(m_mode);

	ExitInfo exit_info = exit_info_future.get();

	m_window.stop();
	m_world.stop();
	m_network_client.stop();
	m_network_server.stop();

	if (!exit_info.exit_message.empty())
	{
		MessageBoxA(NULL, exit_info.exit_message.c_str(), "Exit message", MB_OK | MB_SYSTEMMODAL);
	}

	return exit_info.exit_code;
}

void Application::setGameMode(GameMode mode)
{
	m_window.stop();
	m_world.stop();
	m_network_client.stop();
	m_network_server.stop();

	m_window.clear();
	m_world.clear();
	m_network_client.clear();
	m_network_server.clear();

	m_player_mgr.reset();

	switch (mode)
	{
	case GameMode::SingplePlay:
		m_window.start(this, m_player_mgr.addLocalPlayer());
		m_world.start(this);
		break;

	case GameMode::Host:
		m_window.start(this, m_player_mgr.addLocalPlayer());
		m_world.start(this);
		m_network_server.start(this, m_cmdline.empty() ? nullptr : m_cmdline.c_str());
		break;

	case GameMode::Client:
		m_world.start(this);
		m_network_client.start(this, m_cmdline.c_str());
		break;

	default:
		return;
	}

	m_mode = mode;
}

bool Application::handleCommand(const std::string& cmd)
{
	// note: direct call to setGameMode causes deadlock

	m_cmdline.clear();

	if (cmd.compare("/single") == 0)
	{
		std::thread(&Application::setGameMode, this, GameMode::SingplePlay).detach();
		return true;
	}
	else if (cmd.compare(0, 6, "/host ") == 0 && cmd.size() > 6)
	{
		m_cmdline = &cmd[6];
		std::thread(&Application::setGameMode, this, GameMode::Host).detach();
		return true;
	}
	else if (cmd.compare("/host") == 0)
	{
		std::thread(&Application::setGameMode, this, GameMode::Host).detach();
		return true;
	}
	else if (cmd.compare(0, 9, "/connect ") == 0 && cmd.size() > 9)
	{
		m_cmdline = &cmd[9];
		std::thread(&Application::setGameMode, this, GameMode::Client).detach();
		return true;
	}
	else if (cmd.compare(0, 8, "/player ") == 0 && cmd.size() > 8)
	{
		SwitchPlayer e;
		e.player_id = m_player_mgr.getLocalPlayer()->player_id;
		e.new_player_id = (uint16_t)std::stoul(&cmd[8]);
		handle(e, EventSource::GameWindow);
		return true;
	}
	else if (m_mode != GameMode::Client && cmd.compare(0, 13, "/admin enable") == 0)
	{
		SwitchPlayer e;
		e.player_id = m_player_mgr.getLocalPlayer()->player_id;
		e.new_player_id = 0;
		m_window(e);
		return true;
	}
	else if (m_mode != GameMode::Client && cmd.compare(0, 14, "/admin disable") == 0)
	{
		SwitchPlayer e;
		e.player_id = 0;
		e.new_player_id = m_player_mgr.getLocalPlayer()->player_id;
		m_window(e);
		return true;
	}

	return false;
}

PlayerManager* Application::getPlayerManager()
{
	return &m_player_mgr;
}

void Application::exit(int code, const char* msg)
{
	m_exit.set_value({ code, msg ? msg : "" });
}

void Application::handle(const Connected& e, EventSource src)
{
	if (m_mode == GameMode::Client)
	{
		const Player* player = m_player_mgr.addLocalPlayer(e.player_id);
		if (player)
			m_window.start(this, player);
	}
}

void Application::handle(const Disconnected& e, EventSource src)
{
	if (m_mode == GameMode::Client)
	{
		char* msg = nullptr;

		switch (e.reason)
		{
		case Disconnected::ServerClosed:
			msg = "Server closed";
			break;

		case Disconnected::ServerFull:
			msg = "Server full";
			break;

		case Disconnected::Compatibility:
			msg = "This version is not compatible with the server";
			break;
		}

		exit(-1, msg);
	}
}

void Application::handle(const SwitchPlayer& e, EventSource src)
{
	if (src == EventSource::GameWindow) // command
	{
		if (m_mode == GameMode::Client)
		{
			m_network_client(e);
		}
		else if (m_player_mgr.switchPlayer(e.player_id, e.new_player_id))
		{
			m_window(e);
			m_world(e);

			if (m_mode == GameMode::Host)
				m_network_server(e);
		}
	}
	else if (src == EventSource::Network)
	{
		if (m_player_mgr.switchPlayer(e.player_id, e.new_player_id))
		{
			if (m_mode == GameMode::Host)
			{
				m_world(e);
				m_network_server(e);
			}
			else if (m_mode == GameMode::Client)
			{
				m_window(e);
			}
		}
	}
}

void Application::handle(const Message& e, EventSource src)
{

	if (src == EventSource::GameWindow)
	{
		bool is_command = false;

		if (e.message[0] == (uint32_t)'/')
		{
			using convert_type = std::codecvt_utf8<uint32_t>;
			std::wstring_convert<convert_type, uint32_t> converter;
			is_command = handleCommand(converter.to_bytes(e.message));
		}

		if (!is_command)
		{
			m_window(e);

			if (m_mode == GameMode::Host)
				m_network_server(e);
			else if (m_mode == GameMode::Client)
				m_network_client(e);
		}
	}
	else
	{
		m_window(e);
	}
}

void Application::handle(const AddGameObject& e, EventSource src)
{
	if (m_mode == GameMode::Client)
	{
		m_network_client(e);
	}
	else
	{
		m_world(e);
	}
}

void Application::handle(const MergeGameObjects& e, EventSource src)
{
	if (m_mode != GameMode::Client)
		m_world(e);
}

void Application::handle(const RemoveGameObjectsNearMouse& e, EventSource src)
{
	m_world(e);
}

void Application::handle(const RemoveGameObject& e, EventSource src)
{
	if (m_mode == GameMode::Client)
	{
		m_network_client(e);
	}
	else
	{
		m_world(e);
	}
}

void Application::handle(const RemovePlayerGameObjects& e, EventSource src)
{
	if (m_mode == GameMode::Host)
	{
		m_world(e);
	}
}

void Application::handle(const GameObjectSync& e, EventSource src)
{
	if (e.target == GameObjectSync::Target::GameWindow)
	{
		m_window(e);
	}
	else if (m_mode == GameMode::Client)
	{
		m_world(e);
	}
	else
	{
		m_network_server(e);
	}
}

void Application::handle(const GameObjectSyncRequest& e, EventSource src)
{
	if (m_mode == GameMode::Host)
	{
		m_world(e);
	}
}

void Application::handle(const Highscore& e, EventSource src)
{
	m_window(e);

	if (m_mode == GameMode::Host)
	{
		m_network_server(e);
	}
}
