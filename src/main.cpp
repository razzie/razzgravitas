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
#include <cstdint>
#include <iostream>
#include <Windows.h>
#include <SFML/Graphics.hpp>
#include <Box2D/Box2D.h>
#include <raz/timer.hpp>

constexpr double PI = 3.14159265358979323846;
constexpr float G = 10.0f;

/*
 * Source: https://github.com/SFML/SFML/wiki/Source:-Letterbox-effect-using-a-view
 */
sf::View getLetterboxView(sf::View view, int windowWidth, int windowHeight)
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

//struct GameObject
//{
//	uint16_t player;
//	uint16_t id;
//	b2Body* body;
//};

void addGameObject(b2World& world, float x, float y, float r, float speedx, float speedy)
{
	b2BodyDef def;
	def.type = b2_dynamicBody;
	def.position.Set(x, y);
	//def.fixedRotation = true;
	def.userData = new float(r);

	b2Body* body = world.CreateBody(&def);

	b2CircleShape shape;
	shape.m_p.Set(0.f, 0.f);
	shape.m_radius = r;

	b2FixtureDef fixture;
	fixture.shape = &shape;
	fixture.friction = 0.f;
	fixture.density = 0.005f;
	body->CreateFixture(&fixture);

	body->SetLinearVelocity(b2Vec2(speedx, speedy));
}

void removeGameObject(b2World& world, float x, float y)
{
	b2Vec2 mouse(x, y);

	for (b2Body* body = world.GetBodyList(); body != 0; body = body->GetNext())
	{
		if (body->GetType() != b2_dynamicBody)
			continue;

		float r = *(float*)body->GetUserData();
		b2Vec2 p = body->GetPosition();

		if ((p - mouse).LengthSquared() < r * r)
		{
			delete body->GetUserData();
			world.DestroyBody(body);
			return;
		}
	}
}

void setLevelBounds(b2World& world, float width, float height)
{
	float hwidth = 0.5f * width;
	float hheight = 0.5f * height;

	b2BodyDef def;
	def.type = b2_staticBody;
	def.position.Set(hwidth, hheight);

	b2Body* body = world.CreateBody(&def);

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

class ContactListener : public b2ContactListener
{
	void BeginContact(b2Contact* contact)
	{
		b2WorldManifold worldManifold;
		contact->GetWorldManifold(&worldManifold);

		b2Body* bodyA = contact->GetFixtureA()->GetBody();
		b2Body* bodyB = contact->GetFixtureB()->GetBody();
		b2Vec2 point = worldManifold.points[0];

	}
};

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	raz::Timer timer;
	ContactListener contactListener;
	b2World gameWorld(b2Vec2(0.f, 0.f));
	//gameWorld.SetContactListener(&contactListener);

	float radius = 8.f;
	int resX = 1366;
	int resY = 768;

	setLevelBounds(gameWorld, resX, resY);

	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	sf::RenderWindow window(sf::VideoMode(resX, resY), "RazzGravitas", (sf::Style::Resize + sf::Style::Close), settings);
	window.setVerticalSyncEnabled(true);

	sf::View view;
	view.setSize(resX, resY);
	view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
	view = getLetterboxView(view, resX, resY);

	sf::CircleShape circle(6.f);
	circle.setOutlineThickness(2.f);

	sf::RectangleShape clearRect;
	clearRect.setSize(sf::Vector2f(resX, resY));
	clearRect.setFillColor(sf::Color(255, 255, 255, 4));

	bool mouseDown = false;
	int mouseX = 0;
	int mouseY = 0;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::Resized:
				view = getLetterboxView(view, event.size.width, event.size.height);
				break;

			case sf::Event::MouseWheelMoved:
				radius += event.mouseWheel.delta * 2;
				if (radius < 2.f) radius = 2.f;
				else if (radius > 32.f) radius = 32.f;
				break;

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					if (mouseDown == false)
					{
						mouseX = event.mouseButton.x;
						mouseY = event.mouseButton.y;
					}
					mouseDown = true;
				}
				break;

			case sf::Event::MouseButtonReleased:
				{
					mouseDown = false;
					auto pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
					if (event.mouseButton.button == sf::Mouse::Left)
					{
						auto lastPos = window.mapPixelToCoords(sf::Vector2i(mouseX, mouseY));
						addGameObject(gameWorld, lastPos.x, lastPos.y, radius, pos.x - lastPos.x, pos.y - lastPos.y);
					}
					else if (event.mouseButton.button == sf::Mouse::Right)
						removeGameObject(gameWorld, pos.x, pos.y);
				}
				break;
			}
		}

		gameWorld.Step(0.001f * timer.getElapsed(), 8, 3);

		window.setView(view);
		window.draw(clearRect, sf::BlendAdd);

		circle.setFillColor(sf::Color(255, 128, 128));
		circle.setOutlineColor(sf::Color::Red);
		for (b2Body* body = gameWorld.GetBodyList(); body != 0; body = body->GetNext())
		{
			if (body->GetType() != b2_dynamicBody)
				continue;

			float r = *(float*)body->GetUserData();
			b2Vec2 p = body->GetPosition();

			for (b2Body* body2 = body->GetNext(); body2 != 0; body2 = body2->GetNext())
			{
				if (body2->GetType() != b2_dynamicBody)
					continue;

				b2Vec2 p2 = body2->GetPosition();
				b2Vec2 dir = p - p2;
				float distSq = std::sqrt(dir.Length());

				//if (std::pow(10.f * (r + r2), 2.f) > distSq)
				{
					float angle = (float)std::atan2(dir.y, dir.x) + (float)PI;

					float force = (G * body->GetMass() * body2->GetMass()) / distSq;
					b2Vec2 forceVect(std::cos(angle) * force, std::sin(angle) * force);

					body->ApplyForce(forceVect, body->GetPosition(), true);
					body2->ApplyForce(-forceVect, body2->GetPosition(), true);
				}
			}

			circle.setRadius(r);
			circle.setPosition(p.x - r, p.y - r);
			window.draw(circle);
		}

		circle.setRadius(radius);
		circle.setFillColor(sf::Color::Transparent);
		circle.setOutlineColor(sf::Color(255, 0, 0, 6));

		if (mouseDown)
		{
			circle.setPosition(window.mapPixelToCoords(sf::Vector2i(mouseX, mouseY)) - sf::Vector2f(radius, radius));

			sf::Vertex line[] =
			{
				sf::Vertex(window.mapPixelToCoords(sf::Vector2i(mouseX, mouseY)), sf::Color(255, 0, 0, 6)),
				sf::Vertex(window.mapPixelToCoords(sf::Mouse::getPosition(window)), sf::Color(255, 0, 0, 6))
			};

			window.draw(line, 2, sf::Lines);
		}
		else
		{
			circle.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)) - sf::Vector2f(radius, radius));
		}

		window.draw(circle);
		window.display();
	}

	return 0;
}
