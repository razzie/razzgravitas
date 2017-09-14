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

#include <cstdint>
#include <queue>
#include <SFML/Graphics.hpp>
#include <raz/timer.hpp>
#include "IApplication.hpp"

class GameWindow : public IGameObjectRenderer
{
public:
	GameWindow(IApplication* app, uint16_t player_id);
	~GameWindow();
	virtual void renderGameObject(float x, float y, float r, float vx, float vy, uint16_t player_id);
	void operator()(); // loop
	void operator()(IGameObjectRenderInvoker* world);
	void operator()(Message e);
	void operator()(SwitchPlayer e);

private:
	static sf::Vector2u m_last_size;
	static sf::Vector2i m_last_position;

	IApplication* m_app;
	sf::RenderWindow m_window;
	sf::RenderTexture m_canvas;
	sf::View m_world_view;
	sf::View m_ui_view;
	sf::Shader m_canvas_shader;
	sf::Sprite m_canvas_quad;
	sf::CircleShape m_game_object_shape;
	sf::CircleShape m_mouse_shape;
	sf::RectangleShape m_clear_rect;
	sf::Color m_player_colors[MAX_PLAYERS + 1];
	uint16_t m_player_id;
	std::queue<Message> m_msg_queue;
	raz::Timer m_msg_timer;
	sf::Font m_font;
	sf::Text m_msg;
	sf::Text m_input;
	float m_mouse_radius;
	int m_mouse_drag_x;
	int m_mouse_drag_y;
	bool m_mouse_down;
	bool m_canvas_updated;

	void handleEvent(const sf::Event& event);
	void resize(unsigned width, unsigned height);
	void setPlayer(uint16_t player_id);
	void updateTitle();
};
