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

#include <cstdint>
#include <exception>
#include <mutex>
#include <Box2D/Box2D.h>
#include <raz/bitset.hpp>
#include <raz/timer.hpp>
#include "common/IApplication.hpp"
#include "gameworld/GameObject.hpp"

class GameWorld : public b2ContactListener
{
public:
	GameWorld(IApplication* app);
	~GameWorld();
	void operator()(); // loop
	void operator()(AddGameObject e);
	void operator()(MergeGameObjects e);
	void operator()(RemoveGameObjectsNearMouse e);
	void operator()(RemoveGameObject e);
	void operator()(RemovePlayerGameObjects e);
	void operator()(GameObjectSync e);
	void operator()(GameObjectSyncRequest);
	void operator()(SwitchPlayer e);
	void operator()(std::exception& e);

	virtual void BeginContact(b2Contact *contact);

private:
	IApplication* m_app;
	raz::Timer m_timer;
	raz::Timer m_highscore_timer;
	float m_step_time;
	b2World m_world;
	GameObject* m_obj_db[MAX_PLAYERS][MAX_GAME_OBJECTS_PER_PLAYER];
	raz::Bitset<MAX_GAME_OBJECTS_PER_PLAYER> m_obj_slots[MAX_PLAYERS];
	uint32_t m_last_sync_id;
	mutable uint32_t m_render_counter;

	void setLevelBounds(float width, float height);
	bool findNewObjectID(uint16_t player_id, uint16_t& object_id);
	GameObject* addGameObject(const AddGameObject& e);
	GameObject* addGameObject(const AddGameObject& e, uint16_t object_id, uint32_t sync_id = 0);
	void mergeGameObjects(GameObject* obj1, GameObject* obj2);
	void removeGameObject(uint16_t player_id, uint16_t object_id);
	void removeUnsyncedGameObjects(uint32_t sync_id);
	void removeExpiredGameObjects();
	void sync(GameObjectState& state, uint32_t sync_id);
	void syncRenderer() const;
};
