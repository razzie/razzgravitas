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

#include <cstring>
#include <cmath>
#include <stdexcept>
#include "GameWorld.hpp"

static constexpr double PI = 3.14159265358979323846;

GameWorld::GameWorld(IApplication* app) :
	m_app(app),
	m_world(b2Vec2(0.f, 0.f)),
	m_step_time(0.f),
	m_last_sync_id(0)
{
	setLevelBounds(WORLD_WIDTH, WORLD_HEIGHT);

	if (m_app->getGameMode() != GameMode::Client)
		m_world.SetContactListener(this);

	std::memset(m_obj_db, 0, sizeof(m_obj_db));
}

GameWorld::~GameWorld()
{
}

void GameWorld::render(IGameObjectRenderer* window) const
{
	std::lock_guard<std::mutex> guard(m_lock);

	for (const b2Body* body = m_world.GetBodyList(); body != 0; body = body->GetNext())
	{
		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj == 0)
			continue;

		b2Vec2 pos = body->GetPosition();
		b2Vec2 movement = body->GetLinearVelocity();
		window->renderGameObject(pos.x, pos.y, obj->radius, movement.x, movement.y, obj->player_id);
	}
}

void GameWorld::operator()()
{
	std::lock_guard<std::mutex> guard(m_lock);

	float delta = 0.001f * m_timer.getElapsed();
	for (m_step_time += delta; m_step_time >= WORLD_STEP; m_step_time -= WORLD_STEP)
	{
		removeExpiredGameObjects();

		for (b2Body* body = m_world.GetBodyList(); body != 0; body = body->GetNext())
		{
			GameObject* obj = static_cast<GameObject*>(body->GetUserData());
			if (obj == 0)
				continue;

			b2Vec2 p = body->GetPosition();

			for (b2Body* body2 = body->GetNext(); body2 != 0; body2 = body2->GetNext())
			{
				GameObject* obj2 = static_cast<GameObject*>(body2->GetUserData());
				if (obj2 == 0)
					continue;

				b2Vec2 p2 = body2->GetPosition();
				b2Vec2 dir = p - p2;
				float dist = dir.LengthSquared();
				float angle = (float)std::atan2(dir.y, dir.x) + (float)PI;
				float force = (GRAVITY * body->GetMass() * body2->GetMass()) / dist;
				b2Vec2 force_vect(std::cos(angle) * force, std::sin(angle) * force);

				body->ApplyForce(force_vect, body->GetPosition(), true);
				body2->ApplyForce(-force_vect, body2->GetPosition(), true);
			}
		}

		m_world.Step(WORLD_STEP, 8, 3);
	}

	m_app->handle(this); // render
}

void GameWorld::operator()(AddGameObject e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	addGameObject(e);
}

void GameWorld::operator()(MergeGameObjects e)
{
	GameObject* obj1 = m_obj_db[e.player_id[0]][e.object_id[0]];
	GameObject* obj2 = m_obj_db[e.player_id[1]][e.object_id[1]];
	mergeGameObjects(obj1, obj2);
}

void GameWorld::operator()(RemoveGameObjectsNearMouse e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	b2Vec2 mouse(e.position_x, e.position_y);

	for (b2Body* body = m_world.GetBodyList(); body != 0; )
	{
		b2Body* next_body = body->GetNext();

		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj != 0)
		{
			float mouse_dist = (body->GetPosition() - mouse).Length();

			if (obj->player_id == e.player_id && (mouse_dist < e.radius || mouse_dist < obj->radius))
			{
				RemoveGameObject _e;
				_e.player_id = obj->player_id;
				_e.object_id = obj->object_id;
				m_app->handle(_e, EventSource::GameWorld);
			}
		}

		body = next_body;
	}
}

void GameWorld::operator()(RemoveGameObject e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	removeGameObject(e.player_id, e.object_id);
}

void GameWorld::operator()(RemovePlayerGameObjects e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	for (b2Body* body = m_world.GetBodyList(); body != 0; )
	{
		b2Body* next_body = body->GetNext();

		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj != 0)
		{
			if (obj->player_id == e.player_id)
			{
				removeGameObject(obj->player_id, obj->object_id);
			}
		}

		body = next_body;
	}
}

void GameWorld::operator()(GameObjectSync e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	if (m_last_sync_id != e.sync_id)
	{
		removeUnsyncedGameObjects(m_last_sync_id);
		m_last_sync_id = e.sync_id;
	}

	for (size_t i = 0; i < e.object_count; ++i)
	{
		sync(e.object_states[i], e.sync_id);
	}
}

void GameWorld::operator()(GameObjectSyncRequest e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	auto now = std::chrono::steady_clock::now();

	GameObjectSync sync;
	sync.sync_id = e.sync_id;
	sync.object_count = 0;

	for (b2Body* body = m_world.GetBodyList(); body != 0; body = body->GetNext())
	{
		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (!obj || obj->creation > now)
			continue;

		sync.object_states[sync.object_count].init(body);
		++sync.object_count;

		if (sync.object_count == MAX_GAME_OBJECTS_PER_SYNC)
		{
			m_app->handle(sync, EventSource::GameWorld);
			sync.object_count = 0; // reset counter to refill the struct
		}
	}

	m_app->handle(sync, EventSource::GameWorld);
}

void GameWorld::operator()(SwitchPlayer e)
{
	for (b2Body* body = m_world.GetBodyList(); body != 0; body = body->GetNext())
	{
		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj != 0 && obj->player_id == e.player_id)
		{
			m_obj_db[e.player_id][obj->object_id] = nullptr;
			m_obj_slots[e.player_id].unset(obj->object_id);

			if (m_obj_db[e.new_player_id][obj->object_id] != nullptr)
				throw std::runtime_error("SwitchUser error");

			m_obj_db[e.new_player_id][obj->object_id] = obj;
			m_obj_slots[e.new_player_id].set(obj->object_id);

			obj->player_id = e.new_player_id;
		}
	}
}

void GameWorld::operator()(std::exception& e)
{
	m_app->exit(-1, e.what());
}

void GameWorld::BeginContact(b2Contact* contact)
{
	if (m_app->getGameMode() == GameMode::Client)
		return;

	b2Body* body1 = contact->GetFixtureA()->GetBody();
	b2Body* body2 = contact->GetFixtureB()->GetBody();

	GameObject* obj1 = reinterpret_cast<GameObject*>(body1->GetUserData());
	GameObject* obj2 = reinterpret_cast<GameObject*>(body2->GetUserData());

	if (!obj1 || !obj2 || obj1->isExpired() || obj2->isExpired())
		return;

	b2Vec2 dir1 = body1->GetLinearVelocity(); dir1.Normalize();
	b2Vec2 dir2 = body2->GetLinearVelocity(); dir2.Normalize();
	b2Vec2 dist = body1->GetPosition() - body2->GetPosition(); dist.Normalize();

	if (b2Dot(dir1, dist) < -0.9f || b2Dot(dir2, -dist) < -0.9f)
	{
		MergeGameObjects e;
		e.player_id[0] = obj1->player_id;
		e.object_id[0] = obj1->object_id;
		e.player_id[1] = obj2->player_id;
		e.object_id[1] = obj2->object_id;
		m_app->handle(e, EventSource::GameWorld);
	}
}

void GameWorld::setLevelBounds(float width, float height)
{
	float hwidth = 0.5f * width;
	float hheight = 0.5f * height;

	b2BodyDef def;
	def.type = b2_staticBody;
	def.position.Set(hwidth, hheight);

	b2Body* body = m_world.CreateBody(&def);

	b2PolygonShape shape;

	b2FixtureDef fixture;
	fixture.friction = 0.f;
	fixture.shape = &shape;

	// left
	shape.SetAsBox(1.f, hheight + 1.f, b2Vec2(-hwidth, 0.f), 0.f);
	body->CreateFixture(&fixture);

	// right
	shape.SetAsBox(1.f, hheight + 1.f, b2Vec2(hwidth, 0.f), 0.f);
	body->CreateFixture(&fixture);

	// top
	shape.SetAsBox(hwidth + 1.f, 1.f, b2Vec2(0.f, -hheight), 0.f);
	body->CreateFixture(&fixture);

	// bottom
	shape.SetAsBox(hwidth + 1.f, 1.f, b2Vec2(0.f, hheight), 0.f);
	body->CreateFixture(&fixture);
}

bool GameWorld::findNewObjectID(uint16_t player_id, uint16_t& object_id)
{
	if (player_id >= MAX_PLAYERS)
		return false;

	auto free_slots = m_obj_slots[player_id].falsebits();
	auto slot = free_slots.begin();

	if (slot == free_slots.end())
		return false;

	object_id = (uint16_t)*slot;
	return true;
}

GameObject* GameWorld::addGameObject(const AddGameObject& e)
{
	uint16_t obj_id;
	if (!findNewObjectID(e.player_id, obj_id))
		return nullptr;

	return addGameObject(e, obj_id);
}

GameObject* GameWorld::addGameObject(const AddGameObject& e, uint16_t object_id, uint32_t sync_id)
{
	if (e.position_x < 0.f || e.position_x > WORLD_WIDTH || e.position_y < 0.f || e.position_y > WORLD_HEIGHT)
		return nullptr;

	float radius = e.radius;
	if (radius > MAX_GAME_OBJECT_SIZE)
		radius = MAX_GAME_OBJECT_SIZE;
	else if (radius < MIN_GAME_OBJECT_SIZE)
		radius = MIN_GAME_OBJECT_SIZE;

	GameObject* obj = new GameObject();
	obj->player_id = e.player_id;
	obj->object_id = object_id;
	obj->radius = radius;
	obj->last_sync_id = sync_id;
	obj->creation = std::chrono::steady_clock::now();
	obj->expiry = obj->creation + std::chrono::seconds(GAME_OBJECT_EXPIRY);

	m_obj_db[e.player_id][object_id] = obj;
	m_obj_slots[e.player_id].set(object_id);

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.position.Set(e.position_x, e.position_y);
	def.fixedRotation = true;
	def.userData = obj;

	b2Body* body = m_world.CreateBody(&def);
	obj->body = body;

	b2CircleShape shape;
	shape.m_p.Set(0.f, 0.f);
	shape.m_radius = radius;

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.friction = 0.f;
	fixture.restitution = 0.75f;
	fixture.density = 0.005f;
	body->CreateFixture(&fixture);

	body->SetLinearVelocity(b2Vec2(e.velocity_x, e.velocity_y));

	if (m_app->getGameMode() == GameMode::Client)
	{
		b2Filter filter;
		filter.categoryBits = 0;
		filter.maskBits = 0;
		body->GetFixtureList()->SetFilterData(filter);
	}

	return obj;
}

void GameWorld::mergeGameObjects(GameObject* obj1, GameObject* obj2)
{
	if (!obj1 || !obj2)
		return;

	float radius = std::sqrt(obj1->radius * obj1->radius + obj2->radius * obj2->radius);
	if (radius > MAX_GAME_OBJECT_SIZE)
		return;

	b2Body* body1 = obj1->body;
	b2Body* body2 = obj2->body;
	b2Vec2 position1 = body1->GetPosition();
	b2Vec2 position2 = body2->GetPosition();
	b2Vec2 velocity1 = body1->GetLinearVelocity();
	b2Vec2 velocity2 = body2->GetLinearVelocity();
	float mass1 = body1->GetMass();
	float mass2 = body2->GetMass();
	float mass_fract = 1.f / (mass1 + mass2);
	uint16_t player_id = (obj1->player_id == obj2->player_id) ? obj1->player_id : 0;

	AddGameObject e;
	e.player_id = player_id;
	e.radius = radius;
	e.position_x = (position1.x * mass1 + position2.x * mass2) * mass_fract;
	e.position_y = (position1.y * mass1 + position2.y * mass2) * mass_fract;
	e.velocity_x = (velocity1.x * mass1 + velocity2.x * mass2) * mass_fract;
	e.velocity_y = (velocity1.y * mass1 + velocity2.y * mass2) * mass_fract;

	if (GameObject* new_obj = addGameObject(e))
	{
		obj1->remove();
		obj2->remove();
		new_obj->creation = std::chrono::steady_clock::now() + std::chrono::milliseconds(GAME_SYNC_RATE * 2);
	}
}

void GameWorld::removeGameObject(uint16_t player_id, uint16_t object_id)
{
	if (player_id >= MAX_PLAYERS || object_id >= MAX_GAME_OBJECTS_PER_PLAYER)
		throw std::runtime_error("ID error");

	GameObject* obj = m_obj_db[player_id][object_id];
	if (!obj)
		return;

	m_obj_db[player_id][object_id] = nullptr;
	m_obj_slots[player_id].unset(object_id);

	m_world.DestroyBody(obj->body);
	delete obj;
}

void GameWorld::removeUnsyncedGameObjects(uint32_t sync_id)
{
	for (b2Body* body = m_world.GetBodyList(); body != 0; )
	{
		b2Body* next_body = body->GetNext();

		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj != 0 && obj->last_sync_id != sync_id)
		{
			removeGameObject(obj->player_id, obj->object_id);
		}

		body = next_body;
	}
}

void GameWorld::removeExpiredGameObjects()
{
	bool ignore_expiry = m_app->getGameMode() == GameMode::Client;
	auto now = std::chrono::steady_clock::now();

	for (b2Body* body = m_world.GetBodyList(); body != 0; )
	{
		b2Body* next_body = body->GetNext();

		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj)
		{
			if (!ignore_expiry && obj->expiry < now)
			{
				removeGameObject(obj->player_id, obj->object_id);
			}
		}

		body = next_body;
	}
}

void GameWorld::sync(GameObjectState& state, uint32_t sync_id)
{
	if (state.player_id >= MAX_PLAYERS || state.object_id >= MAX_GAME_OBJECTS_PER_PLAYER)
		throw std::runtime_error("Sync ID error");

	GameObject* obj = m_obj_db[state.player_id][state.object_id];
	if (obj)
	{
		state.apply(obj->body);
		obj->last_sync_id = sync_id;
	}
	else
	{
		AddGameObject e;
		e.player_id = state.player_id;
		e.position_x = state.position_x;
		e.position_y = state.position_y;
		e.radius = state.radius;
		e.velocity_x = state.velocity_x;
		e.velocity_y = state.velocity_y;

		addGameObject(e, state.object_id, sync_id);
	}
}
