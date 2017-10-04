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
#include "common/GameObjectState.hpp"
#include "gameworld/GameObject.hpp"

void GameObject::fill(GameObjectState& state) const
{
	state.player_id = player_id;
	state.object_id = object_id;
	state.position_x = body->GetPosition().x;
	state.position_y = body->GetPosition().y;
	state.radius = radius;
	state.velocity_x = body->GetLinearVelocity().x;
	state.velocity_y = body->GetLinearVelocity().y;
}

void GameObject::apply(const GameObjectState& state)
{
	b2Vec2 position(state.position_x, state.position_y);
	b2Vec2 velocity(state.velocity_x, state.velocity_y);

	if ((position - body->GetPosition()).LengthSquared() > velocity.LengthSquared() * 0.25f)
		body->SetTransform(position, 0.f);

	body->SetLinearVelocity(velocity);
}

void GameObject::remove()
{
	value = 0;
	expiry = std::chrono::time_point<std::chrono::steady_clock>();
}

bool GameObject::isExpired() const
{
	return (std::chrono::steady_clock::now() > expiry);
}
