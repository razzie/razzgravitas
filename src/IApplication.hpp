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

#include "Settings.hpp"
#include "Events.hpp"

class IApplication
{
public:
	// I don't need virtual desctructor
	virtual void exit(int exit_code, const char* msg = nullptr) = 0;
	virtual void handle(Connected e, EventSource src) = 0;
	virtual void handle(Disconnected e, EventSource src) = 0;
	virtual void handle(Message e, EventSource src) = 0;
	virtual void handle(AddGameObject e, EventSource src) = 0;
	virtual void handle(RemoveGameObjectsNearMouse e, EventSource src) = 0;
	virtual void handle(RemoveGameObject e, EventSource src) = 0;
	virtual void handle(RemovePlayerGameObjects e, EventSource src) = 0;
	virtual void handle(GameObjectSync e, EventSource src) = 0;
	virtual void handle(GameObjectSyncRequest e, EventSource src) = 0;
	virtual void handle(IGameObjectRenderInvoker*) = 0;
};
