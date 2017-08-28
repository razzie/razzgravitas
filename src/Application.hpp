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
#include "GameWindow.hpp"
#include "GameWorld.hpp"
#include "Network.hpp"
#include "IApplication.hpp"

enum class GameMode
{
	SingplePlay,
	Host,
	Client
};

class Application : public IApplication
{
public:
	static int run(int argc, char** argv);

	~Application();
	virtual void exit(int exit_code, const char* msg = nullptr);
	virtual void handle(Connected e, EventSource src);
	virtual void handle(Disconnected e, EventSource src);
	virtual void handle(Message e, EventSource src);
	virtual void handle(AddGameObject e, EventSource src);
	virtual void handle(RemoveGameObjectsNearMouse e, EventSource src);
	virtual void handle(RemoveGameObject e, EventSource src);
	virtual void handle(RemovePlayerGameObjects e, EventSource src);
	virtual void handle(GameObjectSync e, EventSource src);
	virtual void handle(GameObjectSyncRequest e, EventSource src);
	virtual void handle(IGameObjectRenderInvoker*);

private:
	Application(int argc, char** argv);
	int run();
	void setGameMode(GameMode mode);
	bool handleCommand(const std::string& cmd);

	std::string m_cmdline;
	GameMode m_mode;
	std::promise<int> m_exit_code;
	raz::Thread<GameWindow> m_window;
	raz::Thread<GameWorld> m_world;
	raz::Thread<Network> m_network;
};
