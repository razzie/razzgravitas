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

#include "common/PlayerManager.hpp"
#include "gamewindow/GameCanvas.hpp"

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

GameCanvas::GameCanvas(IApplication* app, const Player* player) :
	m_app(app),
	m_player(player),
	m_mouse_radius(MIN_GAME_OBJECT_SIZE),
	m_mouse_x(0),
	m_mouse_y(0),
	m_mouse_drag_x(0),
	m_mouse_drag_y(0),
	m_mouse_down(false),
	m_awaiting_render(true)
{
	m_world_view.setSize(WORLD_WIDTH, WORLD_HEIGHT);
	m_world_view.setCenter(m_world_view.getSize().x / 2, m_world_view.getSize().y / 2);

	m_canvas.setSmooth(true);

	m_game_object_shape.setOutlineThickness(0.2f);

	m_mouse_shape.setRadius(m_mouse_radius);
	m_mouse_shape.setOrigin(m_mouse_radius, m_mouse_radius);
	m_mouse_shape.setOutlineThickness(0.2f);
	m_mouse_shape.setFillColor(sf::Color::Transparent);
	m_mouse_shape.setOutlineColor(player->color);

	m_clear_rect.setSize(sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));
	m_clear_rect.setFillColor(sf::Color(255, 255, 255, 8));
}

GameCanvas::~GameCanvas()
{
}

void GameCanvas::render(sf::RenderTarget& target)
{
	target.setView(m_ui_view);
	target.draw(m_canvas_quad);

	target.setView(m_world_view);
	if (m_mouse_down)
	{
		m_mouse_shape.setPosition(m_canvas.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y)));

		sf::Vertex line[] =
		{
			sf::Vertex(m_canvas.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y)), m_mouse_shape.getOutlineColor()),
			sf::Vertex(m_canvas.mapPixelToCoords(sf::Vector2i(m_mouse_x, m_mouse_y)), m_mouse_shape.getOutlineColor())
		};

		target.draw(line, 2, sf::Lines);
		target.draw(m_mouse_shape);
	}
	else
	{
		uint64_t mouse_timeout = m_mouse_idle_timer.peekElapsed();
		if (mouse_timeout < MOUSE_IDLE_TIMEOUT + 1000)
		{
			sf::Color color = m_mouse_shape.getOutlineColor();
			color.a = (mouse_timeout < MOUSE_IDLE_TIMEOUT) ? (sf::Uint8)255 : (sf::Uint8)(255 - (mouse_timeout - MOUSE_IDLE_TIMEOUT) * 255 / 1000);
			m_mouse_shape.setOutlineColor(color);
			m_mouse_shape.setPosition(m_canvas.mapPixelToCoords(sf::Vector2i(m_mouse_x, m_mouse_y)));
			target.draw(m_mouse_shape);
		}
	}

	m_awaiting_render = true;
}

void GameCanvas::render(const GameObjectState& state)
{
	float outline_radius = m_game_object_shape.getOutlineThickness();
	sf::Color color = m_app->getPlayerManager()->getPlayerColor(state.player_id);

	m_game_object_shape.setOutlineColor(color);
	m_game_object_shape.setFillColor(sf::Color(color.r / 2 + 127, color.g / 2 + 127, color.b / 2 + 127));
	m_game_object_shape.setPosition(state.position_x, state.position_y);
	m_game_object_shape.setRadius(state.radius - outline_radius);
	m_game_object_shape.setOrigin(state.radius - outline_radius, state.radius - outline_radius);

	m_canvas.draw(m_game_object_shape);
}

void GameCanvas::render(const RenderJob& job)
{
	m_canvas.draw(m_clear_rect, sf::BlendAdd);

	for (uint32_t i = 0; i < job.object_count; ++i)
		render(job.object_states[i]);

	m_canvas.display();
}

void GameCanvas::handle(const sf::Event& e)
{
	switch (e.type)
	{
	case sf::Event::Resized:
		resize(e.size.width, e.size.height);
		break;

	case sf::Event::MouseWheelMoved:
		m_mouse_radius += 0.1f * e.mouseWheel.delta;
		if (m_mouse_radius < MIN_GAME_OBJECT_SIZE)
			m_mouse_radius = MIN_GAME_OBJECT_SIZE;
		else if (m_mouse_radius > MAX_GAME_OBJECT_CREATION_SIZE)
			m_mouse_radius = MAX_GAME_OBJECT_CREATION_SIZE;
		m_mouse_shape.setRadius(m_mouse_radius);
		m_mouse_shape.setOrigin(m_mouse_radius, m_mouse_radius);
		m_mouse_idle_timer.reset();
		break;

	case sf::Event::MouseMoved:
		m_mouse_x = e.mouseMove.x;
		m_mouse_y = e.mouseMove.y;
		m_mouse_idle_timer.reset();
		break;

	case sf::Event::MouseButtonPressed:
		if (e.mouseButton.button == sf::Mouse::Left)
		{
			if (m_mouse_down == false)
			{
				m_mouse_drag_x = e.mouseButton.x;
				m_mouse_drag_y = e.mouseButton.y;
			}
			m_mouse_down = true;
		}
		else if (e.mouseButton.button == sf::Mouse::Right)
		{
			m_mouse_down = false;
		}
		m_mouse_idle_timer.reset();
		break;

	case sf::Event::MouseButtonReleased:
		auto pos = m_canvas.mapPixelToCoords(sf::Vector2i(m_mouse_x, m_mouse_y));
		if (e.mouseButton.button == sf::Mouse::Left && m_mouse_down)
		{
			m_mouse_down = false;
			auto last_pos = m_canvas.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y));
			AddGameObject e;
			e.position_x = last_pos.x;
			e.position_y = last_pos.y;
			e.radius = m_mouse_radius;
			e.velocity_x = last_pos.x - pos.x;
			e.velocity_y = last_pos.y - pos.y;
			e.player_id = m_player->player_id;
			m_app->handle(e, EventSource::GameWindow);
		}
		else if (e.mouseButton.button == sf::Mouse::Right)
		{
			RemoveGameObjectsNearMouse e;
			e.position_x = pos.x;
			e.position_y = pos.y;
			e.radius = m_mouse_radius;
			e.player_id = m_player->player_id;
			m_app->handle(e, EventSource::GameWindow);
		}
		m_mouse_idle_timer.reset();
		break;
	}
}

void GameCanvas::handle(const GameObjectSync& e)
{
	if (e.sync_id != m_job.sync_id)
	{
		if (m_awaiting_render)
		{
			render(m_job);
			m_awaiting_render = false;
		}

		m_job.sync_id = e.sync_id;
		m_job.object_count = 0;
	}

	for (uint32_t i = 0; i < e.object_count; ++i)
	{
		m_job.object_states[m_job.object_count++] = e.object_states[i];
	}
}

void GameCanvas::handle(const SwitchPlayer& e)
{
	m_player = m_app->getPlayerManager()->getPlayer(e.new_player_id);
	m_mouse_shape.setOutlineColor(m_player->color);
}

void GameCanvas::resize(unsigned width, unsigned height)
{
	m_world_view = getLetterboxView(m_world_view, width, height);

	m_ui_view.setSize((float)width, (float)height);
	m_ui_view.setCenter(m_ui_view.getSize().x / 2, m_ui_view.getSize().y / 2);

	sf::ContextSettings settings;
	settings.antialiasingLevel = ANTIALIASING_LEVEL;

	m_canvas.create(width, height, settings);
	m_canvas.clear(sf::Color::White);
	m_canvas.setView(m_world_view);

	m_canvas_quad.setTexture(m_canvas.getTexture(), true);
}
