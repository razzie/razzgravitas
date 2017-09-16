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

#include <SFML/Graphics.hpp>
#include "common/IApplication.hpp"

class GameCanvas
{
public:
	GameCanvas(IApplication* app, const Player* player);
	~GameCanvas();
	void render(sf::RenderTarget& target);
	void render(const GameObjectState& state);
	void handle(const sf::Event& e);
	void handle(const GameObjectSync& e);
	void handle(const SwitchPlayer& e);
	void resize(unsigned width, unsigned height);

private:
	IApplication* m_app;
	const Player* m_player;
	sf::View m_ui_view;
	sf::View m_world_view;
	sf::RenderTexture m_canvas;
	sf::Shader m_canvas_shader;
	sf::Sprite m_canvas_quad;
	sf::CircleShape m_game_object_shape;
	sf::CircleShape m_mouse_shape;
	sf::RectangleShape m_clear_rect;
	uint32_t m_last_sync_id;
	float m_mouse_radius;
	int m_mouse_x;
	int m_mouse_y;
	int m_mouse_drag_x;
	int m_mouse_drag_y;
	bool m_mouse_down;

};
