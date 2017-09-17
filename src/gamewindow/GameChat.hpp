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

#include <list>
#include <SFML/Graphics.hpp>
#include <raz/timer.hpp>
#include "common/IApplication.hpp"

class GameChat
{
public:
	GameChat(IApplication* app, const Player* player);
	~GameChat();
	void render(sf::RenderTarget& target);
	void handle(const sf::Event& e);
	void handle(const Message& e);
	void handle(const SwitchPlayer& e);
	void resize(unsigned width, unsigned height);

private:
	IApplication* m_app;
	const Player* m_player;
	sf::View m_view;
	sf::Font m_font;
	raz::Timer m_msg_timer;
	std::list<sf::Text> m_msg_queue;
	sf::Text m_input;
};
