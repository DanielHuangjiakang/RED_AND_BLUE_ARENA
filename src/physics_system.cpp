
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
    if (y1_top >= y2_bot || y2_top >= y1_bot) return 0; // no collision
    float x_overlap = std::min(x1_right, x2_right) - std::max(x1_left, x2_left);
    float y_overlap = std::min(y1_bot, y2_bot) - std::max(y1_top, y2_top);

    if (x_overlap < y_overlap) {
        if (motion1.position[0] < motion2.position[0]) return 3; // left collision
        else return 4; // right collision
    } else {
        if (motion1.position[1] < motion2.position[1]) return 1; // top collision
        else return 2; // bot collision
    }

    return 0; // no collision
}

void PhysicsSystem::step(float elapsed_ms)
{
	float step_seconds = elapsed_ms / 1000.f;
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		motion.position += motion.velocity * step_seconds;
	}

	auto& gravity_registry = registry.gravities;
	for(uint i = 0; i< gravity_registry.size(); i++) 
	{
		Gravity& gravity = gravity_registry.components[i];
		Entity entity = gravity_registry.entities[i];
		Motion& motion = registry.motions.get(entity);
		Player player = registry.players.get(entity);
		motion.velocity += gravity.g * step_seconds;
		if (abs(motion.velocity[0]) > 700) motion.velocity[0] = ((motion.velocity[0] > 0) - (motion.velocity[0] < 0)) * 700;
		if (abs(motion.velocity[1]) > 700) motion.velocity[1] = ((motion.velocity[1] > 0) - (motion.velocity[1] < 0)) * 700;
		motion.velocity[0] = 0.95 * motion.velocity[0];	
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