#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include <random>
#include "decisionTree.hpp"

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

	registry.gravities.emplace(entity);

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });

 	return entity;
}


// create a block based on its top left corner (x, y), and its width and height
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
 	motion.velocity = { 0.f, 0.f };
 	motion.position = {x + (width / 2), y + (height / 2)};
	motion.scale = {width, height};

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });

 	return entity;
}


// create a block based on its center (position), and its width and height
Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 };
 	motion.position = position;
	motion.scale = {width, height};

	auto& block = registry.blocks.emplace(entity);
	block.x = int(position[0] - (width / 2));
	block.y = int(position[1] - (height / 2));
	block.width = width;
	block.height = height;

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });

 	return entity;
}

Entity createBullet(RenderSystem* renderer, int side, vec2 position, int direction) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& bullet = registry.bullets.emplace(entity);
	bullet.side = side;

	auto& motion = registry.motions.emplace(entity);
	int dir = 0;
	if (direction == 0) dir = -1;
	else dir = 1;
 	motion.velocity = { 500 * dir, 0 }; 
 	motion.position = position;
	motion.scale = { 8, 8 }; // width * height

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });
	
	return entity;

}

Entity createLaser(RenderSystem* renderer) {
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
    motion.scale = {10, 10};

    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::SALMON, GEOMETRY_BUFFER_ID::SQUARE});
    registry.colors.insert(entity, {0.0f, 1.0f, 0.0f});

    return entity;
}

