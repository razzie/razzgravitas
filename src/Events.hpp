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
#include "Settings.hpp"
#include "GameObject.hpp"

enum class EventType : uint32_t
{
	Connected,
	Disconnected,
	AddGameObject,
	RemoveGameObjects,
	RemovePlayerGameObjects,
	GameObjectSync
};

struct Connected
{
	uint16_t player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id);
	}
};

struct Disconnected
{
	template<class Serializer>
	void operator()(Serializer& serializer)
	{
	}
};

struct AddGameObject
{
	float position_x;
	float position_y;
	float radius;
	float velocity_x;
	float velocity_y;
	uint16_t player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(position_x)(position_y)(radius)(velocity_x)(velocity_y)(player_id);
	}
};

struct RemoveGameObjects
{
	float position_x;
	float position_y;
	float radius;
	uint16_t player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(position_x)(position_y)(radius)(player_id);
	}
};

struct RemovePlayerGameObjects
{
	uint16_t player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id);
	}
};

struct GameObjectSync
{
	size_t object_count;
	GameObjectState object_states[MAX_GAME_OBJECTS_PER_SYNC];

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(object_count);

		for (size_t i = 0; i < object_count; ++i)
			serializer(object_states[i]);
	}
};

struct GameObjectSyncRequest
{
};
