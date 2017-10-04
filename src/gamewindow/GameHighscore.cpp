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
#include "gamewindow/GameFont.hpp"
#include "gamewindow/GameHighscore.hpp"

GameHighscore::GameHighscore(IApplication* app, const GameFont* font) :
	m_app(app),
	m_font(font),
	m_width(0.f)
{
	m_sample_score.setFont(*m_font);
	m_sample_score.setOutlineColor(sf::Color::White);
	m_sample_score.setOutlineThickness(0.1f);
	m_sample_score.setCharacterSize(MESSAGE_CHAR_SIZE);
}

GameHighscore::~GameHighscore()
{
}

void GameHighscore::render(sf::RenderTarget& target)
{
	float y_pos = 10.f;

	for (Score& score : m_highscore)
	{
		score.text.setPosition(m_width - score.text.getLocalBounds().width - 10.f, y_pos);
		target.draw(score.text);
		y_pos += MESSAGE_CHAR_SIZE + 2;
	}
}

void GameHighscore::handle(const sf::Event& e)
{
	switch (e.type)
	{
	case sf::Event::Resized:
		resize(e.size.width, e.size.height);
		break;
	}
}

void GameHighscore::handle(const Highscore& e)
{
	m_highscore.clear();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if (e.highscore[i] < 0)
			continue;

		Score score;
		score.score = e.highscore[i];
		score.text = m_sample_score;
		score.text.setString(std::to_string(score.score));
		score.text.setFillColor(m_app->getPlayerManager()->getPlayerColor(i));

		bool inserted = false;

		for (auto it = m_highscore.begin(), end = m_highscore.end(); it != end; ++it)
		{
			if (score.score > it->score)
			{
				inserted = true;
				m_highscore.insert(it, std::move(score));
				break;
			}
		}

		if (!inserted)
		{
			m_highscore.push_back(std::move(score));
		}
	}
}

void GameHighscore::resize(unsigned width, unsigned height)
{
	m_width = (float)width;
}
