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

class b2Body;

struct GameObject
{
	uint16_t player_id;
	uint16_t object_id;
	float radius;
	b2Body* body;
	uint32_t last_sync_id;
};

struct GameObjectState
{
	uint16_t player_id;
	uint16_t object_id;
	float position_x;
	float position_y;
	float radius;
	float velocity_x;
	float velocity_y;

	void init(const b2Body* body);
	void apply(b2Body* body);

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id)(object_id)(position_x)(position_y)(radius)(velocity_x)(velocity_y);
	}
};

class IGameObjectRenderer
{
public:
	virtual void renderGameObject(float x, float y, float r, uint16_t player_id) = 0;
};

class IGameObjectRenderInvoker
{
public:
	virtual void render(IGameObjectRenderer*) const = 0;
};
