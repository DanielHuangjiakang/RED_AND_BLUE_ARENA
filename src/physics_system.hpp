#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

vec2 get_bounding_box(const Motion& motion);
int collides(const Motion& motion1, const Motion& motion2);

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);

	// int collides(const Motion& motion1, const Motion& motion2);

	PhysicsSystem()
	{
	}
};