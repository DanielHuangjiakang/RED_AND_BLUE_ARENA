// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}


int collides(const Motion& motion1, const Motion& motion2)
{
	float x1_left = motion1.position[0] - (motion1.scale[0] / 2);
	float x1_right = motion1.position[0] + (motion1.scale[0] / 2);
	float y1_top = motion1.position[1] - (motion1.scale[1] / 2);
	float y1_bot = motion1.position[1] + (motion1.scale[1] / 2);

	float x2_left = motion2.position[0] - (motion2.scale[0] / 2);
	float x2_right = motion2.position[0] + (motion2.scale[0] / 2);
	float y2_top = motion2.position[1] - (motion2.scale[1] / 2);
	float y2_bot = motion2.position[1] + (motion2.scale[1] / 2);

	if (x1_left >= x2_right || x2_left >= x1_right) return 0; // no collision
    if (y1_top >= y2_bot || y2_top >= y1_bot) return 0; // no collision;

	if (y1_bot > y2_top && y1_bot < y2_bot) {
		return 1; // top collision
	} else if (y1_top > y2_top && y1_top < y2_bot) {
		return 2; // bot collision
	} else if (x1_right > x2_left && x1_right < x2_right) {
		return 3; // left collision
	} else if (x1_left > x2_left && x1_left < x2_right) {
		return 4; // right collision
	}

    return 0;
	
}

void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		motion.position += motion.velocity * step_seconds;
	}

	auto& gravity_registry = registry.gravities;
	for(uint i = 0; i< gravity_registry.size(); i++) 
	{
		Gravity gravity = gravity_registry.components[i];
		Entity entity = gravity_registry.entities[i];
		Motion& motion = registry.motions.get(entity);
		float step_seconds = elapsed_ms / 1000.f;
		motion.velocity[1] += gravity.a * step_seconds;
		// motion.velocity[1] += gravity.a;
	}

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			int collision = collides(motion_i, motion_j);
			if (collision)
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				auto& collision1 = registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				collision1.direction = collision;
				auto& collision2 = registry.collisions.emplace_with_duplicates(entity_j, entity_i);
				collision2.direction = collision;
			}
		}
	}
}