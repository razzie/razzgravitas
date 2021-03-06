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

// window config
#define APP_NAME "RazzGravitas"
#define RESOLUTION_WIDTH 800
#define RESOLUTION_HEIGHT 600
#define ANTIALIASING_LEVEL 4
#define MESSAGE_TIMEOUT 3000
#define MESSAGE_CHAR_SIZE 16
#define MOUSE_IDLE_TIMEOUT 3000

// world config
#define WORLD_WIDTH 80
#define WORLD_HEIGHT 60
#define WORLD_SCALE (1.f / 10.f)
#define WORLD_STEP (1.f / 60.f)
#define GRAVITY 1800.f
#define MAX_PLAYERS 13 // player0 + player1..12
#define MAX_GAME_OBJECTS_PER_PLAYER 32
#define MAX_GAME_OBJECTS (MAX_PLAYERS * MAX_GAME_OBJECTS_PER_PLAYER)

// gameplay config
#define MIN_GAME_OBJECT_SIZE 0.4f
#define MAX_GAME_OBJECT_CREATION_SIZE 2.0f
#define MAX_GAME_OBJECT_SIZE 6.0f
#define MIN_GAME_OBJECT_DURATION 10
#define MAX_GAME_OBJECT_DURATION 30
#define MIN_GAME_OBJECT_VALUE 10
#define MAX_GAME_OBJECT_VALUE 100
#define GAME_OBJECT_MERGE_VELOCITY_THRESHOLD 50.f
#define GAME_OBJECT_MERGE_SCALE_THRESHOLD 1.2f
#define GAME_OBJECT_MERGE_BONUS 5
#define GAME_OBJECT_EXPIRATION_BONUS 5
#define HIGHSCORE_SYNC_RATE 500

// network config
#define GAME_PORT 12345
#define MAX_PACKET_SIZE 512
#define MAX_GAME_OBJECTS_PER_SYNC 16
#define GAME_SYNC_RATE 50
#define PING_RATE 250
#define CONNECTION_TIMEOUT 3000
