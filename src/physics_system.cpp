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

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	// vec2 dp = motion1.position - motion2.position;
	// float dist_squared = dot(dp,dp);
	// const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	// const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	// const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	// const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	// const float r_squared = max(other_r_squared, my_r_squared);
	// if (dist_squared < r_squared)
	// 	return true;
	// return false;
	float x1_left = motion1.position[0] - (motion2.scale[0] / 2);
	float x1_right = motion1.position[0] + (motion2.scale[0] / 2);
	float y1_up = motion1.position[1] - (motion2.scale[1] / 2);
	float y1_down = motion1.position[1] + (motion2.scale[1] / 2);

	float x2_left = motion2.position[0] - (motion2.scale[0] / 2);
	float x2_right = motion2.position[0] + (motion2.scale[0] / 2);
	float y2_up = motion2.position[1] - (motion2.scale[1] / 2);
	float y2_down = motion2.position[1] + (motion2.scale[1] / 2);

	if (x1_left >= x2_right || x2_left >= x1_right) return false;
    if (y1_up >= y2_down || y2_up >= y1_down) return false;

    return true;
	
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

		motion.velocity[1] += gravity.a;
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
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}