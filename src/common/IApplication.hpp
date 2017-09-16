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

#include "common/Settings.hpp"
#include "common/Events.hpp"

struct Player;
class PlayerManager;

enum class GameMode
{
	SingplePlay,
	Host,
	Client
};

class IApplication
{
public:
	// I don't need virtual destructor
	virtual GameMode getGameMode() const = 0;
	virtual PlayerManager* getPlayerManager() = 0;
	virtual void exit(int exit_code, const char* msg = nullptr) = 0;
	virtual void handle(const Connected& e, EventSource src) = 0;
	virtual void handle(const Disconnected& e, EventSource src) = 0;
	virtual void handle(const SwitchPlayer& e, EventSource src) = 0;
	virtual void handle(const Message& e, EventSource src) = 0;
	virtual void handle(const AddGameObject& e, EventSource src) = 0;
	virtual void handle(const MergeGameObjects& e, EventSource src) = 0;
	virtual void handle(const RemoveGameObjectsNearMouse& e, EventSource src) = 0;
	virtual void handle(const RemoveGameObject& e, EventSource src) = 0;
	virtual void handle(const RemovePlayerGameObjects& e, EventSource src) = 0;
	virtual void handle(const GameObjectSync& e, EventSource src) = 0;
	virtual void handle(const GameObjectSyncRequest& e, EventSource src) = 0;
};
