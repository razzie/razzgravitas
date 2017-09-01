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

#include "PlayerManager.hpp"

PlayerManager::PlayerManager()
{
	reset();
}

PlayerManager::~PlayerManager()
{
}

const Player* PlayerManager::addPlayer()
{
	std::lock_guard<std::mutex> guard(m_mutex);

	auto player_slots = m_player_slots.falsebits();
	auto slot = player_slots.begin();

	if (slot == player_slots.end())
	{
		return nullptr;
	}
	else
	{
		m_player_slots.set(*slot);
		return  &m_players[*slot];
	}
}

const Player* PlayerManager::addLocalPlayer(uint16_t player_id)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	if (m_local_player
		|| player_id >= MAX_PLAYERS
		|| m_player_slots.isset(player_id))
	{
		return nullptr;
	}

	m_player_slots.set(player_id);
	m_local_player = &m_players[player_id];
	return &m_players[player_id];
}

const Player* PlayerManager::getLocalPlayer()
{
	return m_local_player;
}

const Player* PlayerManager::findPlayer(const void* data)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	for (auto slot : m_player_slots.truebits())
	{
		if (m_players[slot].data == data)
			return &m_players[slot];
	}

	return nullptr;
}

bool PlayerManager::switchPlayer(uint16_t player_id, uint16_t new_player_id)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	if (player_id >= MAX_PLAYERS || new_player_id >= MAX_PLAYERS)
		return false;

	if (!m_player_slots.isset(player_id) || m_player_slots.isset(new_player_id))
		return false;

	m_player_slots.set(new_player_id);
	m_player_slots.unset(player_id);
	std::swap(m_players[player_id].data, m_players[new_player_id].data);

	if (m_local_player && m_local_player->player_id == player_id)
		m_local_player = &m_players[new_player_id];

	return true;
}

void PlayerManager::removePlayer(uint16_t player_id)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	if (player_id >= MAX_PLAYERS)
		return;

	m_player_slots.unset(player_id);
	m_players[player_id].data = nullptr;
}

void PlayerManager::reset()
{
	std::lock_guard<std::mutex> guard(m_mutex);

	m_player_slots.reset();
	m_local_player = nullptr;

	for (uint16_t i = 0; i < MAX_PLAYERS; ++i)
	{
		m_players[i].player_id = i;
		m_players[i].data = nullptr;
	}
}
