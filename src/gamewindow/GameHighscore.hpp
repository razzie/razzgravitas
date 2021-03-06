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

#pragma once

#include <list>
#include <SFML/Graphics.hpp>
#include "common/IApplication.hpp"

class GameFont;

class GameHighscore
{
public:
	GameHighscore(IApplication* app, const GameFont* font);
	~GameHighscore();
	void render(sf::RenderTarget& target);
	void handle(const sf::Event& e);
	void handle(const Highscore& e);
	void resize(unsigned width, unsigned height);

private:
	struct Score
	{
		uint32_t score;
		sf::Text text;
	};

	IApplication* m_app;
	const GameFont* m_font;
	sf::Text m_sample_score;
	std::list<Score> m_highscore;
	float m_width;
};
