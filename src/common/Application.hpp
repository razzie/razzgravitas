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

#include <future>
#include <string>
#include <raz/thread.hpp>
#include "gamewindow/GameWindow.hpp"
#include "gameworld/GameWorld.hpp"
#include "network/NetworkClient.hpp"
#include "network/NetworkServer.hpp"
#include "common/PlayerManager.hpp"
#include "common/IApplication.hpp"

class Application : public IApplication
{
public:
	static int run(int argc, char** argv);

	~Application();
	virtual GameMode getGameMode() const;
	virtual PlayerManager* getPlayerManager();
	virtual void exit(int exit_code, const char* msg = nullptr);
	virtual void handle(const Connected& e, EventSource src);
	virtual void handle(const Disconnected& e, EventSource src);
	virtual void handle(const SwitchPlayer& e, EventSource src);
	virtual void handle(const Message& e, EventSource src);
	virtual void handle(const AddGameObject& e, EventSource src);
	virtual void handle(const MergeGameObjects& e, EventSource src);
	virtual void handle(const RemoveGameObjectsNearMouse& e, EventSource src);
	virtual void handle(const RemoveGameObject& e, EventSource src);
	virtual void handle(const RemovePlayerGameObjects& e, EventSource src);
	virtual void handle(const GameObjectSync& e, EventSource src);
	virtual void handle(const GameObjectSyncRequest& e, EventSource src);
	virtual void handle(const Highscore& e, EventSource src);

private:
	struct ExitInfo
	{
		int exit_code;
		std::string exit_message;
	};

	Application(int argc, char** argv);
	int run();
	void setGameMode(GameMode mode);
	bool handleCommand(const std::string& cmd);

	GameMode m_mode;
	PlayerManager m_player_mgr;
	std::string m_cmdline;
	std::promise<ExitInfo> m_exit;
	raz::Thread<GameWindow> m_window;
	raz::Thread<GameWorld> m_world;
	raz::Thread<NetworkClient> m_network_client;
	raz::Thread<NetworkServer> m_network_server;
};
