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
#include <exception>
#include <mutex>
#include <Box2D/Box2D.h>
#include <raz/bitset.hpp>
#include <raz/timer.hpp>
#include "Events.hpp"
#include "Settings.hpp"

class Application;
class GameWindow;
struct GameObject;
struct GameObjectState;

class GameWorld
{
public:
	GameWorld(Application* app, uint16_t player_id);
	~GameWorld();
	void render(GameWindow& window) const;
	void sync(GameObjectState& state);
	void operator()(); // loop
	void operator()(AddGameObject e);
	void operator()(RemoveGameObjects e);
	void operator()(std::exception& e);

private:
	mutable std::mutex m_lock;
	Application* m_app;
	raz::Timer m_timer;
	b2World m_world;
	GameObject* m_obj_db[MAX_PLAYERS][MAX_GAME_OBJECTS_PER_PLAYER];
	raz::Bitset<MAX_GAME_OBJECTS_PER_PLAYER> m_obj_slots[MAX_PLAYERS];

	void setLevelBounds(float width, float height);
	bool findNewObjectID(uint16_t player_id, uint16_t& object_id);
};
