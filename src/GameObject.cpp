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

#include <Box2D/Box2D.h>
#include "GameObject.hpp"

void GameObjectState::init(const b2Body* body)
{
	GameObject* obj = static_cast<GameObject*>(body->GetUserData());

	player_id = obj->player_id;
	object_id = obj->object_id;
	position_x = body->GetPosition().x;
	position_y = body->GetPosition().y;
	velocity_x = body->GetLinearVelocity().x;
	velocity_y = body->GetLinearVelocity().y;
}

void GameObjectState::apply(b2Body* body)
{
#ifdef _DEBUG
	GameObject* obj = static_cast<GameObject*>(body->GetUserData());
	if (obj->player_id != player_id || obj->object_id != object_id)
		return;
#endif

	body->SetTransform(b2Vec2(position_x, position_y), 0.f);
	body->SetLinearVelocity(b2Vec2(velocity_x, velocity_y));
}
