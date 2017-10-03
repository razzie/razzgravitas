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
	m_width(0),
	m_height(0)
{
}

GameHighscore::~GameHighscore()
{
}

void GameHighscore::render(sf::RenderTarget& target)
{
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

		// TODO: ordering!

		Score score;
		score.score = e.highscore[i];
		score.text.setFont(*m_font);
		score.text.setString(std::to_string(score.score));
		score.text.setFillColor(m_app->getPlayerManager()->getPlayerColor(i));

		m_highscore.push_back(std::move(score));
	}
}

void GameHighscore::resize(unsigned width, unsigned height)
{
	m_width = width;
	m_height = height;
}
