/*
Copyright (C) 2016 - Gábor "Razzie" Görzsöny
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
#include "GameWindow.hpp"
#include "GameWorld.hpp"
#include "Settings.hpp"

/*
 * Source: https://github.com/SFML/SFML/wiki/Source:-Letterbox-effect-using-a-view
 */
static sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight)
{
	// Compares the aspect ratio of the window to the aspect ratio of the view,
	// and sets the view's viewport accordingly in order to archieve a letterbox effect.
	// A new view (with a new viewport set) is returned.

	float windowRatio = windowWidth / (float)windowHeight;
	float viewRatio = view.getSize().x / (float)view.getSize().y;
	float sizeX = 1;
	float sizeY = 1;
	float posX = 0;
	float posY = 0;

	bool horizontalSpacing = true;
	if (windowRatio < viewRatio)
		horizontalSpacing = false;

	// If horizontalSpacing is true, the black bars will appear on the left and right side.
	// Otherwise, the black bars will appear on the top and bottom.

	if (horizontalSpacing) {
		sizeX = viewRatio / windowRatio;
		posX = (1 - sizeX) / 2.f;
	}

	else {
		sizeY = windowRatio / viewRatio;
		posY = (1 - sizeY) / 2.f;
	}

	view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));

	return view;
}

GameWindow::GameWindow(Application* app, uint16_t player_id) :
	m_app(app),
	m_mouse_radius(6.f),
	m_mouse_drag_x(0),
	m_mouse_drag_y(0),
	m_mouse_down(false)
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	m_window.create(sf::VideoMode(RESOLUTION_WIDTH, RESOLUTION_HEIGHT), "RazzGravitas", (sf::Style::Resize + sf::Style::Close), settings);
	m_window.setVerticalSyncEnabled(true);
	m_window.clear(sf::Color::White);

	m_view.setSize(RESOLUTION_WIDTH, RESOLUTION_HEIGHT);
	m_view.setCenter(m_view.getSize().x / 2, m_view.getSize().y / 2);
	m_view = getLetterboxView(m_view, RESOLUTION_WIDTH, RESOLUTION_HEIGHT);
	m_window.setView(m_view);

	raz::Color player_color = raz::ColorTable()[player_id];

	m_game_object_shape.setOutlineThickness(2.f);

	m_mouse_shape.setRadius(m_mouse_radius);
	m_mouse_shape.setOutlineThickness(2.f);
	m_mouse_shape.setOutlineColor(sf::Color(player_color.r, player_color.g, player_color.b, 6));
	m_mouse_shape.setFillColor(sf::Color::Transparent);

	m_clear_rect.setSize(sf::Vector2f(RESOLUTION_WIDTH, RESOLUTION_HEIGHT));
	m_clear_rect.setFillColor(sf::Color(255, 255, 255, 4));
}

GameWindow::~GameWindow()
{
}

void GameWindow::drawGameObject(float x, float y, float r, raz::Color color)
{
	m_game_object_shape.setOutlineColor(sf::Color(color.r, color.g, color.b));
	m_game_object_shape.setFillColor(sf::Color(color.r, color.g, color.b, 128));
	m_game_object_shape.setPosition(x - r + 1.f, y - r + 1.f);
	m_game_object_shape.setRadius(r - 1.f);
	m_window.draw(m_game_object_shape);
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
			m_view = getLetterboxView(m_view, event.size.width, event.size.height);
			m_window.setView(m_view);
			m_window.clear(sf::Color::White);
			break;

		case sf::Event::MouseWheelMoved:
			m_mouse_radius += event.mouseWheel.delta * 2;
			if (m_mouse_radius < 2.f)
				m_mouse_radius = 2.f;
			else if (m_mouse_radius > 32.f)
				m_mouse_radius = 32.f;
			m_mouse_shape.setRadius(m_mouse_radius);
			break;

		case sf::Event::MouseButtonPressed:
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				if (m_mouse_down == false)
				{
					m_mouse_drag_x = event.mouseButton.x;
					m_mouse_drag_y = event.mouseButton.y;
				}
				m_mouse_down = true;
			}
			break;

		case sf::Event::MouseButtonReleased:
			{
				m_mouse_down = false;
				auto pos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					auto last_pos = m_window.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y));
					AddGameObject e;
					e.position_x = last_pos.x;
					e.position_y = last_pos.y;
					e.radius = m_mouse_radius;
					e.velocity_x = pos.x - last_pos.x;
					e.velocity_y = pos.y - last_pos.y;
					m_app->getGameWorld()(e);
				}
				else if (event.mouseButton.button == sf::Mouse::Right)
				{
					RemoveGameObjects e;
					e.position_x = pos.x;
					e.position_y = pos.y;
					e.radius = m_mouse_radius;
					m_app->getGameWorld()(e);
				}
			}
			break;
		}
	}

	m_window.draw(m_clear_rect, sf::BlendAdd);

	if (m_mouse_down)
	{
		m_mouse_shape.setPosition(m_window.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y)) - sf::Vector2f(m_mouse_radius, m_mouse_radius));

		sf::Vertex line[] =
		{
			sf::Vertex(m_window.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y)), m_mouse_shape.getOutlineColor()),
			sf::Vertex(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)), m_mouse_shape.getOutlineColor())
		};

		m_window.draw(line, 2, sf::Lines);
	}
	else
	{
		m_mouse_shape.setPosition(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)) - sf::Vector2f(m_mouse_radius, m_mouse_radius));
	}
	m_window.draw(m_mouse_shape);

	m_window.display();
}

void GameWindow::operator()(GameWorld* world)
{
	world->render(*this);
}
