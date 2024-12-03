// internal
#include "physics_system.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
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
	float x1_left = motion1.position[0] - (abs(motion1.scale[0]) / 2);
    float x1_right = motion1.position[0] + (abs(motion1.scale[0]) / 2);
    float y1_top = motion1.position[1] - (abs(motion1.scale[1]) / 2);
    float y1_bot = motion1.position[1] + (abs(motion1.scale[1]) / 2);
    float x2_left = motion2.position[0] - (abs(motion2.scale[0]) / 2);
    float x2_right = motion2.position[0] + (abs(motion2.scale[0]) / 2);
    float y2_top = motion2.position[1] - (abs(motion2.scale[1]) / 2);
    float y2_bot = motion2.position[1] + (abs(motion2.scale[1]) / 2);

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
}

void compute_transformed_vertices(const Mesh& mesh, const Motion& motion, std::vector<vec2>& out_vertices)
{
    out_vertices.clear();
    out_vertices.reserve(mesh.vertices.size());

    float cos_theta = cos(motion.angle);
    float sin_theta = sin(motion.angle);

    for (const ColoredVertex& vertex : mesh.vertices)
    {
        vec2 v = { vertex.position.x * motion.scale.x, vertex.position.y * motion.scale.y };
        vec2 v_rotated = { v.x * cos_theta - v.y * sin_theta, v.x * sin_theta + v.y * cos_theta };
        vec2 v_transformed = v_rotated + motion.position;
        out_vertices.push_back(v_transformed);
    }
}



// bool mesh_collides(Entity entity_i, Entity entity_j)
// {
//     Motion& motion_j = registry.motions.get(entity_j);
// 	Motion& motion_i = registry.motions.get(entity_i);
// 	Mesh* mesh1 = registry.meshPtrs.get(entity_j);

//     std::vector<vec2> transformed_vertices;
//     compute_transformed_vertices(*mesh1, motion_j, transformed_vertices);

//     for (size_t i = 0; i < mesh1->vertex_indices.size(); i += 1)
//     {
//         vec2& v1 = transformed_vertices[mesh1->vertex_indices[i]];
// 		if()
//     }

//     return false; // no collision
// }

bool mesh_collides(Entity entity_i, Entity entity_j)
{
    Motion& motion_j = registry.motions.get(entity_j);
    Motion& motion_i = registry.motions.get(entity_i);
    Mesh* mesh1 = registry.meshPtrs.get(entity_j);

    std::vector<vec2> transformed_vertices;
    compute_transformed_vertices(*mesh1, motion_j, transformed_vertices);

    // Get the bounding box for entity_i (portal)
    vec2 portal_min = motion_i.position - abs(motion_i.scale / 2.f);
    vec2 portal_max = motion_i.position + abs(motion_i.scale / 2.f);

    for (size_t i = 0; i < mesh1->vertex_indices.size(); i += 1)
    {
        vec2& v1 = transformed_vertices[mesh1->vertex_indices[i]];

        // Check if the transformed vertex is within the portal's bounding box
        if (v1.x >= portal_min.x && v1.x <= portal_max.x &&
            v1.y >= portal_min.y && v1.y <= portal_max.y)
        {
            return true; // Collision detected
        }
    }

    return false; // No collision
}


void PhysicsSystem::step(float elapsed_ms)
{
	float step_seconds = elapsed_ms / 1000.f;
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		if (registry.blocks.has(entity)) {
			Block& block = registry.blocks.get(entity);
			block.travelled_dist = motion.velocity * step_seconds;
		}
		motion.position += motion.velocity * step_seconds;
	}

	auto& gravity_registry = registry.gravities;
	for(uint i = 0; i< gravity_registry.size(); i++) 
	{
		Gravity& gravity = gravity_registry.components[i];
		Entity entity = gravity_registry.entities[i];
		Motion& motion = registry.motions.get(entity);
		motion.velocity += gravity.g * step_seconds;

		float signx = (motion.velocity[0] > 0) - (motion.velocity[0] < 0);
		if (gravity.drag) {
			motion.velocity[0] += -1 * signx * step_seconds * 800.f;
			if ((motion.velocity[0] > 0) - (motion.velocity[0] < 0) != signx) {
				motion.velocity[0] = 0.f;
			}
		}

		float signy = (motion.velocity[1] > 0) - (motion.velocity[1] < 0);
		if (registry.players.has(entity)) {
			if (abs(motion.velocity[0]) > 350) motion.velocity[0] = signx * 350;
			if (abs(motion.velocity[1]) > 700) motion.velocity[1] = signy * 700;
		} 
	}	

	auto& block_registry = registry.blocks;
	for(uint i = 0; i< block_registry.size(); i++)
	{
		Block& block = block_registry.components[i];
		Entity entity = block_registry.entities[i];
		Motion& motion = registry.motions.get(entity);
		if (block.moving == 1) {
			if (motion.position.x > window_width_px - 200) {
				motion.velocity.x = -abs(motion.velocity.x);
			} else if (motion.position.x < 200) {
				motion.velocity.x = abs(motion.velocity.x);
			}
		}
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
			
				if (registry.players.has(entity_i) && registry.portals.has(entity_j)) {
					// mesh collision code
					if (mesh_collides(entity_i, entity_j)) {
						auto& collision1 = registry.collisions.emplace_with_duplicates(entity_i, entity_j);
						collision1.direction = collision;
						auto& collision2 = registry.collisions.emplace_with_duplicates(entity_j, entity_i);
						collision2.direction = collision;
					}
				} else if (registry.players.has(entity_j) && registry.portals.has(entity_i)) {
					// mesh collision code
					if (mesh_collides(entity_j, entity_i)) {
						auto& collision1 = registry.collisions.emplace_with_duplicates(entity_i, entity_j);
						collision1.direction = collision;
						auto& collision2 = registry.collisions.emplace_with_duplicates(entity_j, entity_i);
						collision2.direction = collision;
					}
				} else {
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
}


