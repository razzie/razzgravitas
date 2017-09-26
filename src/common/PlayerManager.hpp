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

#include <chrono>
#include <cstdint>
#include <mutex>
#include <SFML/Graphics/Color.hpp>
#include <raz/bitset.hpp>
#include "common/Config.hpp"

class Application;

struct Player
{
	uint16_t player_id;
	sf::Color color;
	mutable std::chrono::steady_clock::time_point last_updated;
	mutable uint32_t highscore;
	mutable const void* data;
};

class PlayerManager
{
public:
	PlayerManager();
	~PlayerManager();
	const Player* addPlayer();
	const Player* addLocalPlayer();
	const Player* addLocalPlayer(uint16_t player_id);
	const Player* getPlayer(uint16_t player_id) const;
	const Player* getLocalPlayer() const;
	const Player* findPlayer(const void* data);
	bool switchPlayer(uint16_t player_id, uint16_t new_player_id);
	void removePlayer(uint16_t player_id);
	sf::Color getPlayerColor(uint16_t player_id);

protected:
	friend class Application;
	void reset();

private:
	mutable std::mutex m_mutex;
	Player m_players[MAX_PLAYERS + 1];
	raz::Bitset<MAX_PLAYERS + 1> m_player_slots;
	Player* m_local_player;
	uint16_t m_last_player_id;
};
