#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"


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

	registry.gravities.emplace(entity);

	
	auto& animation = registry.animations.emplace(entity);
	if (side == 2) { // red player
		animation.frames = {
			TEXTURE_ASSET_ID::RED_RUN_1,
			TEXTURE_ASSET_ID::RED_RUN_2,
			TEXTURE_ASSET_ID::RED_RUN_3,
		};
		motion.scale = { PLAYER_WIDTH, PLAYER_HEIGHT };
		motion.scale.x *= -1;
	} else { // blue player
		animation.frames = {
			TEXTURE_ASSET_ID::BLUE_RUN_1,
			TEXTURE_ASSET_ID::BLUE_RUN_2,
			TEXTURE_ASSET_ID::BLUE_RUN_3,
		};
		motion.scale = { PLAYER_WIDTH, PLAYER_HEIGHT };
	}

	// Initial render request uses first frame
	registry.renderRequests.insert(
		entity,
		{ animation.frames[0], 
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createGun(RenderSystem* renderer, int side, vec2 position) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = { 0.2, 20 };

	if (side == 2) {
		registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::RED_GUN,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });
	} else {
		registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BLUE_GUN,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createBackground(RenderSystem* renderer, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 };
 	motion.position = {width / 2, height / 2};
	motion.scale = {width, height};
	
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CITY,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });

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
 		{ TEXTURE_ASSET_ID::PAD, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::TEXTURED,
 			GEOMETRY_BUFFER_ID::SPRITE });

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
 		{ TEXTURE_ASSET_ID::BLOCK, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::TEXTURED,
 			GEOMETRY_BUFFER_ID::SPRITE });

 	return entity;
}

Entity createHelpPanel(RenderSystem* renderer, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = {width / 2, height / 2};
	motion.scale = {width, height};

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HELP,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE });
	return entity;
}

Entity createText(RenderSystem* renderer, std::string text_content, vec2 position, bool is_visible) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& text = registry.texts.emplace(entity);
	text.text = text_content;
	text.position = position;
	text.is_visible = is_visible;

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
	motion.scale = { 15, 6 }; // width * height

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::BULLET, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::TEXTURED,
 			GEOMETRY_BUFFER_ID::SPRITE });
	
	return entity;

}