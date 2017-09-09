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

// window settings
#define APP_NAME "RazzGravitas"
#define RESOLUTION_WIDTH 800
#define RESOLUTION_HEIGHT 600

// world settings
#define WORLD_WIDTH 80
#define WORLD_HEIGHT 60
#define WORLD_SCALE (1.f / 10.f)
#define WORLD_STEP (1.f / 60.f)
#define GRAVITY 1000.f
#define MIN_GAME_OBJECT_SIZE 0.4f
#define MAX_GAME_OBJECT_SIZE 3.2f
#define GAME_OBJECT_EXPIRY 60
#define MAX_PLAYERS 12
#define MAX_GAME_OBJECTS_PER_PLAYER 32

// chat settings
#define MESSAGE_TIMEOUT 2000
#define MESSAGE_CHAR_SIZE 16

// network settings
#define GAME_PORT 12345
#define MAX_PACKET_SIZE 512
#define MAX_GAME_OBJECTS_PER_SYNC 16
#define GAME_SYNC_RATE 100
#define PING_RATE 500
#define CONNECTION_TIMEOUT 1500
