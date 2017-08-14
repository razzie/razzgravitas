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

#include "Application.hpp"
#include <Windows.h>
#include "Settings.hpp"

Application::Application(const char* cmdline) :
	m_cmdline(cmdline)
{
//#ifdef _DEBUG
//	m_cmdline = "172.0.0.1";
//#endif
}

Application::~Application()
{
}

int Application::run()
{
	auto exit_code_future = m_exit_code.get_future();

	m_network.start(this, m_cmdline);
	m_world.start(this);
	// network thread will start the window

	int exit_code = exit_code_future.get();

	m_window.stop();
	m_world.stop();
	m_network.stop();

	return exit_code;
}

void Application::exit(int code, const char* msg)
{
	m_exit_code.set_value(code);

	if (msg)
	{
		MessageBoxA(NULL, msg, "Exit message", MB_OK);
	}
}

raz::Thread<GameWindow>& Application::getGameWindow()
{
	return m_window;
}

raz::Thread<GameWorld>& Application::getGameWorld()
{
	return m_world;
}

raz::Thread<Network>& Application::getNetwork()
{
	return m_network;
}
