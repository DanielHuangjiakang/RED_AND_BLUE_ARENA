#include <random>
#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

// Entity createFish(RenderSystem* renderer, vec2 position)
// {
// 	// Reserve en entity
// 	auto entity = Entity();

// 	// Store a reference to the potentially re-used mesh object
// 	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
// 	registry.meshPtrs.emplace(entity, &mesh);

// 	// Initialize the position, scale, and physics components
// 	auto& motion = registry.motions.emplace(entity);
// 	motion.angle = 0.f;
// 	motion.velocity = { 0, 50 };
// 	motion.position = position;

// 	// Setting initial values, scale is negative to make it face the opposite way
// 	motion.scale = vec2({ -FISH_BB_WIDTH, FISH_BB_HEIGHT });

// 	// Create an (empty) Bug component to be able to refer to all bug
// 	registry.eatables.emplace(entity);
// 	registry.renderRequests.insert(
// 		entity,
// 		{
// 			TEXTURE_ASSET_ID::FISH,
// 			EFFECT_ASSET_ID::TEXTURED,
// 			GEOMETRY_BUFFER_ID::SPRITE
// 		});

// 	return entity;
// }

Entity createLaser(RenderSystem* renderer, vec2 target) {
    auto entity = Entity();
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
    registry.meshPtrs.emplace(entity, &mesh);
    registry.lasers.emplace(entity);

    std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_real_distribution<float> distX(0.f, static_cast<float>(window_width_px));
    std::uniform_real_distribution<float> distY(0.f, static_cast<float>(window_height_px));
    vec2 position = {distX(rng), distY(rng)};

    auto& motion = registry.motions.emplace(entity);
    motion.position = position;
    motion.scale = { 10, 10 };

    vec2 direction = normalize(target - position);
    motion.velocity = direction * 100.f;

    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::TEXTURE_COUNT,
              EFFECT_ASSET_ID::SALMON,
              GEOMETRY_BUFFER_ID::SQUARE });
    registry.colors.insert(entity, {0.0f, 1.0f, 0.0f});

    return entity;
}


Entity createPlayer(RenderSystem* renderer, int side, vec2 position, bool direction) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& player = registry.players.emplace(entity);
	player.side = side;
	player.direction = direction; // Default to facing right initially

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 }; 
 	motion.position = position;
	motion.scale = { 50, 50 }; // width * height

	auto& gravity = registry.gravities.emplace(entity);

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });

 	return entity;
}

// Function to create bullets: bullets should contain component speed, position, size should be fixed;
Entity createBullet(RenderSystem* renderer, vec2 position, bool direction) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& bullet = registry.bullet.emplace(entity);
	bullet.direction = direction;

	// For getting the right velocity
	int dir = 0;

	if (direction == 0)
	{
		// facing left
		dir = -1;
	}
	else {
		dir = 1;
	}
	

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 100 * dir, 0 }; 
 	motion.position = position;
	motion.scale = { 5, 5 }; // width * height

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });
	
	return entity;

}

Entity createBlock1(RenderSystem* renderer, int x, int y, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& block = registry.blocks.emplace(entity);
	block.x = x;
	block.y = y;
	block.width = width;
	block.height = height;

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 };
 	motion.position = {x + (width / 2), y + (height / 2)};
	motion.scale = {width, height};

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });

 	return entity;
}

Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 };
 	motion.position = position;
	motion.scale = {width, height};

	auto& block = registry.blocks.emplace(entity);
	block.x = position[0] - (width / 2);
	block.y = position[1] - (height / 2);
	block.width = width;
	block.height = height;

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });

 	return entity;
}
