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
#include <string>
#include <raz/hash.hpp>
#include "common/Settings.hpp"
#include "common/GameObjectState.hpp"

enum class EventType : uint32_t
{
	Unknown,
	Hello             = (uint32_t)raz::hash("Hello"),
	Ping              = (uint32_t)raz::hash("Ping"),
	Connected         = (uint32_t)raz::hash("Connected"),
	Disconnected      = (uint32_t)raz::hash("Disconnected"),
	SwitchPlayer      = (uint32_t)raz::hash("SwitchPlayer"),
	Message           = (uint32_t)raz::hash("Message"),
	AddGameObject     = (uint32_t)raz::hash("AddGameObject"),
	RemoveGameObject  = (uint32_t)raz::hash("RemoveGameObject"),
	GameObjectSync    = (uint32_t)raz::hash("GameObjectSync")
};

enum class EventSource
{
	GameWindow,
	GameWorld,
	Network
};

template<EventType T = EventType::Unknown>
struct Event
{
	static EventType getEventType()
	{
		return T;
	}
};

struct Hello : public Event<EventType::Hello>
{
	uint64_t checksum;

	static constexpr uint64_t calculate();

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(checksum);
	}
};

struct Ping : public Event<EventType::Ping>
{
};

struct Connected : public Event<EventType::Connected>
{
	uint16_t player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id);
	}
};

struct Disconnected : public Event<EventType::Disconnected>
{
	enum Reason
	{
		ServerClosed,
		ServerFull,
		Compatibility
	};

	Reason reason;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		int& _reason = *(int*)(&reason);
		serializer(_reason);
	}
};

struct SwitchPlayer : public Event<EventType::SwitchPlayer>
{
	uint16_t player_id;
	uint16_t new_player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id)(new_player_id);
	}
};

struct Message : public Event<EventType::Message>
{
	uint16_t player_id;
	std::basic_string<uint32_t, std::char_traits<uint32_t>, std::allocator<uint32_t>> message;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id)(message);
	}
};

struct AddGameObject : public Event<EventType::AddGameObject>
{
	float radius;
	float position_x;
	float position_y;
	float velocity_x;
	float velocity_y;
	uint16_t player_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(radius)(position_x)(position_y)(velocity_x)(velocity_y)(player_id);
	}
};

struct MergeGameObjects : public Event<>
{
	uint16_t player_id[2];
	uint16_t object_id[2];
};

struct RemoveGameObject : public Event<EventType::RemoveGameObject>
{
	uint16_t player_id;
	uint16_t object_id;

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(player_id)(object_id);
	}
};

struct RemoveGameObjectsNearMouse : public Event<>
{
	float position_x;
	float position_y;
	float radius;
	uint16_t player_id;
};

struct RemovePlayerGameObjects : public Event<>
{
	uint16_t player_id;
};

struct GameObjectSyncRequest : public Event<>
{
	uint32_t sync_id;
};

struct GameObjectSync : public Event<EventType::GameObjectSync>
{
	enum Target
	{
		GameWindow,
		Network
	};

	uint32_t sync_id;
	uint32_t object_count;
	GameObjectState object_states[MAX_GAME_OBJECTS_PER_SYNC];
	Target target; // INTERNAL

	template<class Serializer>
	void operator()(Serializer& serializer)
	{
		serializer(sync_id);
		serializer(object_count);

		for (uint32_t i = 0; i < object_count; ++i)
			serializer(object_states[i]);
	}
};

constexpr uint64_t Hello::calculate()
{
	return (raz::hash(APP_NAME)
		+ WORLD_WIDTH
		+ WORLD_HEIGHT
		+ (uint64_t)WORLD_SCALE          * 1000
		+ (uint64_t)WORLD_STEP           * 1000
		+ (uint64_t)GRAVITY              * 1000
		+ (uint64_t)MIN_GAME_OBJECT_SIZE * 1000
		+ (uint64_t)MAX_GAME_OBJECT_SIZE * 1000
		+ GAME_OBJECT_EXPIRY
		+ MAX_PLAYERS
		+ MAX_GAME_OBJECTS_PER_PLAYER
		+ MAX_GAME_OBJECTS_PER_SYNC
		+ GAME_SYNC_RATE
		+ PING_RATE
		+ CONNECTION_TIMEOUT

		+ (uint64_t)EventType::Hello
		+ (uint64_t)EventType::Ping
		+ (uint64_t)EventType::Connected
		+ (uint64_t)EventType::Disconnected
		+ (uint64_t)EventType::SwitchPlayer
		+ (uint64_t)EventType::Message
		+ (uint64_t)EventType::AddGameObject
		+ (uint64_t)EventType::RemoveGameObject
		+ (uint64_t)EventType::GameObjectSync

		+ sizeof(Hello)
		+ sizeof(Ping)
		+ sizeof(Connected)
		+ sizeof(Disconnected)
		+ sizeof(SwitchPlayer)
		+ sizeof(Message)
		+ sizeof(AddGameObject)
		+ sizeof(RemoveGameObject)
		+ sizeof(GameObjectSync));
}
