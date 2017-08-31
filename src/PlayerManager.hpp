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
#include <mutex>
#include <raz/bitset.hpp>
#include "Settings.hpp"

class Application;

struct Player
{
	uint16_t player_id;
	mutable int data; // https://stackoverflow.com/questions/1953639/is-it-safe-to-cast-socket-to-int-under-win64
};

class PlayerManager
{
public:
	PlayerManager();
	~PlayerManager();
	const Player* addPlayer();
	const Player* addLocalPlayer(uint16_t player_id);
	const Player* getLocalPlayer();
	const Player* findPlayer(int data);
	bool switchPlayer(uint16_t player_id, uint16_t new_player_id);
	void removePlayer(uint16_t player_id);

protected:
	friend class Application;
	void reset();

private:
	std::mutex m_mutex;
	Player m_players[MAX_PLAYERS];
	raz::Bitset<MAX_PLAYERS> m_player_slots;
	Player* m_local_player;
};
