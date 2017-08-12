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

#include <cmath>
#include "Application.hpp"
#include "GameWorld.hpp"
#include "GameObject.hpp"
#include "GameWindow.hpp"
#include "Settings.hpp"

static constexpr double PI = 3.14159265358979323846;
static constexpr float G = 10.0f;

GameWorld::GameWorld(Application* app, uint16_t player_id) :
	m_app(app),
	m_world(b2Vec2(0.f, 0.f))
{
	setLevelBounds(RESOLUTION_WIDTH, RESOLUTION_HEIGHT);
}

GameWorld::~GameWorld()
{
}

void GameWorld::render(GameWindow& window)
{
	std::lock_guard<std::mutex> guard(m_lock);

	for (b2Body* body = m_world.GetBodyList(); body != 0; body = body->GetNext())
	{
		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj == 0)
			continue;

		b2Vec2 pos = body->GetPosition();
		window.drawGameObject(pos.x, pos.y, obj->radius, raz::ColorTable()[obj->player_id]);
	}
}

void GameWorld::operator()()
{
	std::lock_guard<std::mutex> guard(m_lock);

	m_world.Step(0.001f * m_timer.getElapsed(), 8, 3);

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
			float dist = std::sqrt(dir.Length());
			float angle = (float)std::atan2(dir.y, dir.x) + (float)PI;
			float force = (G * body->GetMass() * body2->GetMass()) / dist;
			b2Vec2 forceVect(std::cos(angle) * force, std::sin(angle) * force);

			body->ApplyForce(forceVect, body->GetPosition(), true);
			body2->ApplyForce(-forceVect, body2->GetPosition(), true);
		}
	}

	m_app->getGameWindow()(this);
}

void GameWorld::operator()(AddGameObject e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	GameObject* obj = new GameObject();
	obj->radius = e.radius;

	// TODO
	obj->player_id = 0;
	obj->object_id = 0;

	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.position.Set(e.position_x, e.position_y);
	//def.fixedRotation = true;
	def.userData = obj;

	b2Body* body = m_world.CreateBody(&def);
	obj->body = body;

	b2CircleShape shape;
	shape.m_p.Set(0.f, 0.f);
	shape.m_radius = e.radius;

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.friction = 0.f;
	fixture.restitution = 0.75f;
	fixture.density = 0.005f;
	body->CreateFixture(&fixture);

	body->SetLinearVelocity(b2Vec2(e.velocity_x, e.velocity_y));
}

void GameWorld::operator()(RemoveGameObjects e)
{
	std::lock_guard<std::mutex> guard(m_lock);

	b2Vec2 mouse(e.position_x, e.position_y);

	for (b2Body* body = m_world.GetBodyList(); body != 0; )
	{
		b2Body* next_body = body->GetNext();

		GameObject* obj = static_cast<GameObject*>(body->GetUserData());
		if (obj != 0)
		{
			float r = e.radius;
			b2Vec2 p = body->GetPosition();

			if ((p - mouse).LengthSquared() < r * r)
			{
				delete obj;
				m_world.DestroyBody(body);
			}
		}

		body = next_body;
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
