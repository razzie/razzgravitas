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
#include "gamewindow/GameWindow.hpp"

sf::Vector2u GameWindow::m_last_size = { RESOLUTION_WIDTH, RESOLUTION_HEIGHT };
sf::Vector2i GameWindow::m_last_position = { -1, -1 };

GameWindow::GameWindow(IApplication* app, const Player* player) :
	m_app(app),
	m_player(player),
	m_canvas(app, player),
	m_chat(app, player)
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	m_window.create(sf::VideoMode(m_last_size.x, m_last_size.y), {}, (sf::Style::Resize + sf::Style::Close), settings);
	m_window.setVerticalSyncEnabled(true);
	m_window.setKeyRepeatEnabled(false);

	if (m_last_position.x != -1 && m_last_position.y != -1)
		m_window.setPosition(m_last_position);

	m_canvas.resize(m_last_size.x, m_last_size.y);
	m_chat.resize(m_last_size.x, m_last_size.y);
	updateTitle();
}

GameWindow::~GameWindow()
{
	m_last_position = m_window.getPosition();
}

void GameWindow::operator()()
{
	if (!m_window.isOpen())
		m_app->exit(0);

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			m_window.close();
			break;

		case sf::Event::Resized:
			m_last_size.x = event.size.width;
			m_last_size.y = event.size.height;
			break;
		}

		m_canvas.handle(event);
		m_chat.handle(event);
	}

	m_canvas.render(m_window);
	m_chat.render(m_window);
	m_window.display();
}

void GameWindow::operator()(GameObjectSync e)
{
	m_canvas.handle(e);
}

void GameWindow::operator()(Message e)
{
	m_chat.handle(e);
}

void GameWindow::operator()(SwitchPlayer e)
{
	m_player = m_app->getPlayerManager()->getPlayer(e.new_player_id);
	m_canvas.handle(e);
	m_chat.handle(e);
	updateTitle();
}

void GameWindow::updateTitle()
{
	sf::String title = APP_NAME;
	title += " (";

	switch (m_app->getGameMode())
	{
	case GameMode::SingplePlay:
		title += "SinglePlay : ";
		break;
	case GameMode::Host:
		title += "Host : ";
		break;
	case GameMode::Client:
		title += "Client : ";
		break;
	}

	title += "player ";
	title += std::to_string(m_player->player_id);
	title += ")";

	m_window.setTitle(title);
}
