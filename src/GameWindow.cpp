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
#include <raz/color.hpp>
#include "GameWindow.hpp"

sf::Vector2u GameWindow::m_last_size = { RESOLUTION_WIDTH, RESOLUTION_HEIGHT };
sf::Vector2i GameWindow::m_last_position = { -1, -1 };

extern const char* canvas_vert;
extern const char* canvas_frag;

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

GameWindow::GameWindow(IApplication* app, uint16_t player_id) :
	m_app(app),
	m_player_id(player_id),
	m_mouse_radius(MIN_GAME_OBJECT_SIZE),
	m_mouse_drag_x(0),
	m_mouse_drag_y(0),
	m_mouse_down(false)
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	m_window.create(sf::VideoMode(m_last_size.x, m_last_size.y), {}, (sf::Style::Resize + sf::Style::Close), settings);
	m_window.setVerticalSyncEnabled(true);
	m_window.setKeyRepeatEnabled(false);

	if (m_last_position.x != -1 && m_last_position.y != -1)
		m_window.setPosition(m_last_position);

	m_world_view.setSize(WORLD_WIDTH, WORLD_HEIGHT);
	m_world_view.setCenter(m_world_view.getSize().x / 2, m_world_view.getSize().y / 2);

	m_player_colors[0] = sf::Color::Black;

	raz::ColorTable color_table;
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		raz::Color color = color_table[i];
		m_player_colors[i + 1] = sf::Color(color.r, color.g, color.b);
	}

	m_game_object_shape.setOutlineThickness(0.2f);

	m_mouse_shape.setRadius(m_mouse_radius);
	m_mouse_shape.setOutlineThickness(0.2f);
	m_mouse_shape.setFillColor(sf::Color::Transparent);

	m_clear_rect.setSize(sf::Vector2f(WORLD_WIDTH, WORLD_HEIGHT));
	m_clear_rect.setFillColor(sf::Color(255, 255, 255, 8));

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

	if (sf::Shader::isAvailable())
		m_canvas_shader.loadFromMemory(canvas_vert, canvas_frag);

	resize(m_window.getSize().x, m_window.getSize().y);
	setPlayer(player_id);
}

GameWindow::~GameWindow()
{
	m_last_position = m_window.getPosition();
}

void GameWindow::renderGameObject(float x, float y, float r, float vx, float vy, uint16_t player_id)
{
	float outline_radius = m_game_object_shape.getOutlineThickness();
	sf::Color color = m_player_colors[player_id];

	m_game_object_shape.setOutlineColor(color);
	m_game_object_shape.setFillColor(sf::Color(color.r / 2 + 127, color.g / 2 + 127, color.b / 2 + 127));
	m_game_object_shape.setPosition(x, y);
	m_game_object_shape.setRadius(r - outline_radius);
	m_game_object_shape.setOrigin(r - outline_radius, r - outline_radius);

	m_canvas.draw(m_game_object_shape);
}

void GameWindow::operator()()
{
	if (!m_window.isOpen())
		m_app->exit(0);

	//m_window.clear(sf::Color::White);
	m_window.setView(m_ui_view);

	m_canvas.draw(m_clear_rect, sf::BlendAdd);

	if (sf::Shader::isAvailable())
	{
		auto rect = m_canvas_quad.getTextureRect();
		m_canvas_shader.setUniform("texture", sf::Shader::CurrentTexture);
		m_canvas_shader.setUniform("resolution", sf::Vector2f((float)rect.width, (float)rect.height));
		m_window.draw(m_canvas_quad, &m_canvas_shader);
	}
	else
	{
		m_window.draw(m_canvas_quad);
	}

	m_window.setView(m_world_view);

	sf::Event event;
	while (m_window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed:
			m_window.close();
			break;

		case sf::Event::Resized:
			resize(event.size.width, event.size.height);
			break;

		case sf::Event::TextEntered:
			if (event.text.unicode >= 32)
				m_input.setString(m_input.getString() + event.text.unicode);
			break;

		case sf::Event::KeyPressed:
			switch (event.key.code)
			{
			case sf::Keyboard::V:
				if (event.key.control)
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
					e.player_id = m_player_id;
					e.message = m_input.getString().getData();
					m_app->handle(e, EventSource::GameWindow);
				}
				m_input.setString({});
				break;
			}
			break;

		case sf::Event::MouseWheelMoved:
			m_mouse_radius += 0.2f * event.mouseWheel.delta;
			if (m_mouse_radius < MIN_GAME_OBJECT_SIZE)
				m_mouse_radius = MIN_GAME_OBJECT_SIZE;
			else if (m_mouse_radius > MAX_GAME_OBJECT_SIZE)
				m_mouse_radius = MAX_GAME_OBJECT_SIZE;
			m_mouse_shape.setRadius(m_mouse_radius);
			m_mouse_shape.setOrigin(m_mouse_radius, m_mouse_radius);
			break;

		case sf::Event::MouseButtonPressed:
			if (m_window.hasFocus() && event.mouseButton.button == sf::Mouse::Left)
			{
				if (m_mouse_down == false)
				{
					m_mouse_drag_x = event.mouseButton.x;
					m_mouse_drag_y = event.mouseButton.y;
				}
				m_mouse_down = true;
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				m_mouse_down = false;
			}
			break;

		case sf::Event::MouseButtonReleased:
			auto pos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
			if (event.mouseButton.button == sf::Mouse::Left && m_mouse_down)
			{
				m_mouse_down = false;
				auto last_pos = m_window.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y));
				AddGameObject e;
				e.position_x = last_pos.x;
				e.position_y = last_pos.y;
				e.radius = m_mouse_radius;
				e.velocity_x = last_pos.x - pos.x;
				e.velocity_y = last_pos.y - pos.y;
				e.player_id = m_player_id;
				m_app->handle(e, EventSource::GameWindow);
			}
			else if (event.mouseButton.button == sf::Mouse::Right)
			{
				RemoveGameObjectsNearMouse e;
				e.position_x = pos.x;
				e.position_y = pos.y;
				e.radius = m_mouse_radius;
				e.player_id = m_player_id;
				m_app->handle(e, EventSource::GameWindow);
			}
			break;
		}
	}

	if (m_mouse_down)
	{
		m_mouse_shape.setPosition(m_window.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y)));

		sf::Vertex line[] =
		{
			sf::Vertex(m_window.mapPixelToCoords(sf::Vector2i(m_mouse_drag_x, m_mouse_drag_y)), m_mouse_shape.getOutlineColor()),
			sf::Vertex(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)), m_mouse_shape.getOutlineColor())
		};

		m_window.draw(line, 2, sf::Lines);
	}
	else
	{
		m_mouse_shape.setPosition(m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window)));
	}
	m_window.draw(m_mouse_shape);

	if (!m_msg_queue.empty())
	{
		if (m_msg_timer.peekElapsed() > MESSAGE_TIMEOUT / m_msg_queue.size())
		{
			m_msg_timer.reset();

			Message msg = m_msg_queue.front();
			m_msg_queue.pop();

			m_msg.setFillColor(m_player_colors[msg.player_id]);
			m_msg.setString(msg.message.c_str());
		}
	}
	else if (m_msg_timer.peekElapsed() > MESSAGE_TIMEOUT)
	{
		m_msg.setString({});
	}

	m_window.setView(m_ui_view);
	m_window.draw(m_msg);
	m_window.draw(m_input);

	m_window.display();
}

void GameWindow::operator()(IGameObjectRenderInvoker* world)
{
	//m_canvas.setView(m_world_view);
	world->render(this);
	m_canvas.display();
}

void GameWindow::operator()(Message e)
{
	m_msg_queue.push(e);
}

void GameWindow::operator()(SwitchPlayer e)
{
	setPlayer(e.new_player_id);
}

void GameWindow::resize(unsigned width, unsigned height)
{
	m_last_size.x = width;
	m_last_size.y = height;

	m_world_view = getLetterboxView(m_world_view, width, height);

	m_ui_view.setSize((float)width, (float)height);
	m_ui_view.setCenter(m_ui_view.getSize().x / 2, m_ui_view.getSize().y / 2);

	m_canvas.create(width, height);
	m_canvas.setView(m_world_view);
	m_canvas_quad.setTexture(m_canvas.getTexture(), true);

	m_msg.setPosition(10.f, 10.f);
	m_input.setPosition(10.f, height - MESSAGE_CHAR_SIZE - 10.f);

	m_canvas.clear(sf::Color::White);
	m_window.clear(sf::Color::White);
}

void GameWindow::setPlayer(uint16_t player_id)
{
	m_player_id = player_id;

	sf::Color player_color = m_player_colors[m_player_id];
	m_mouse_shape.setOutlineColor(player_color);
	m_input.setFillColor(player_color);

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
	title += std::to_string(m_player_id);
	title += ")";

	m_window.setTitle(title);
}


#define GLSL(x) "#version 120\n" #x

const char* canvas_vert = GLSL(
void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
}
);

const char* canvas_frag = GLSL(
uniform sampler2D texture;
uniform vec2 resolution;

void main()
{
	vec2 h_uv = vec2(0.5 / resolution.x, 0.0);
	vec2 v_uv = vec2(0.0, 0.5 / resolution.y);
	vec4 color = vec4(0.0);

	color += texture2D(texture, gl_TexCoord[0].xy - h_uv - v_uv);
	color += texture2D(texture, gl_TexCoord[0].xy + h_uv - v_uv);
	color += texture2D(texture, gl_TexCoord[0].xy + h_uv + v_uv);
	color += texture2D(texture, gl_TexCoord[0].xy - h_uv + v_uv);
	color /= 4.0;

	gl_FragColor = color;
}
);
