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

#include <ShlObj.h>
#include "common/PlayerManager.hpp"
#include "gamewindow/GameChat.hpp"

GameChat::GameChat(IApplication* app, const Player* player) :
	m_app(app),
	m_player(player)
{
	PWSTR font_filename;
	SHGetKnownFolderPath(FOLDERID_Fonts, 0, NULL, &font_filename);
	m_font.loadFromFile(sf::String(font_filename) + "/arial.ttf");
	CoTaskMemFree(font_filename);

	m_msg.setFont(m_font);
	m_msg.setOutlineColor(sf::Color::White);
	m_msg.setOutlineThickness(0.1f);
	m_msg.setCharacterSize(MESSAGE_CHAR_SIZE);

	m_input.setFont(m_font);
	m_input.setOutlineColor(sf::Color::White);
	m_input.setOutlineThickness(0.1f);
	m_input.setCharacterSize(MESSAGE_CHAR_SIZE);
	m_input.setFillColor(m_player->color);
}

GameChat::~GameChat()
{
}

void GameChat::render(sf::RenderTarget& target)
{
	if (!m_msg_queue.empty())
	{
		if (m_msg_timer.peekElapsed() > MESSAGE_TIMEOUT / m_msg_queue.size())
		{
			m_msg_timer.reset();

			Message msg = m_msg_queue.front();
			m_msg_queue.pop();

			m_msg.setFillColor(m_app->getPlayerManager()->getPlayerColor(msg.player_id));
			m_msg.setString(msg.message.c_str());
		}
	}
	else if (m_msg_timer.peekElapsed() > MESSAGE_TIMEOUT)
	{
		m_msg.setString({});
	}

	target.setView(m_view);
	target.draw(m_msg);
	target.draw(m_input);
}

void GameChat::handle(const sf::Event& e)
{
	switch (e.type)
	{
	case sf::Event::Resized:
		resize(e.size.width, e.size.height);
		break;

	case sf::Event::TextEntered:
		if (e.text.unicode >= 32)
			m_input.setString(m_input.getString() + e.text.unicode);
		break;

	case sf::Event::KeyPressed:
		switch (e.key.code)
		{
		case sf::Keyboard::V:
			if (e.key.control)
			{
				if (OpenClipboard(NULL) != FALSE)
				{
					HANDLE clip0 = GetClipboardData(CF_UNICODETEXT);
					if (clip0 != NULL)
					{
						wchar_t *c = reinterpret_cast<wchar_t*>(GlobalLock(clip0));
						m_input.setString(m_input.getString() + sf::String(c));
						GlobalUnlock(clip0);
					}
					CloseClipboard();
				}
			}
			break;

		case sf::Keyboard::BackSpace:
			if (!m_input.getString().isEmpty())
			{
				auto msg = m_input.getString();
				msg.erase(m_input.getString().getSize() - 1);
				m_input.setString(msg);
			}
			break;

		case sf::Keyboard::Return:
			{
				Message e;
				e.player_id = m_player->player_id;
				e.message = m_input.getString().getData();
				m_app->handle(e, EventSource::GameWindow);
			}
			m_input.setString({});
			break;
		}
		break;
	}
}

void GameChat::handle(const Message& e)
{
	m_msg_queue.push(e);
}

void GameChat::handle(const SwitchPlayer& e)
{
	m_player = m_app->getPlayerManager()->getPlayer(e.new_player_id);
	m_input.setFillColor(m_player->color);
}

void GameChat::resize(unsigned width, unsigned height)
{
	m_view.setSize((float)width, (float)height);
	m_view.setCenter(m_view.getSize().x / 2, m_view.getSize().y / 2);

	m_msg.setPosition(10.f, 10.f);
	m_input.setPosition(10.f, height - MESSAGE_CHAR_SIZE - 10.f);
}
