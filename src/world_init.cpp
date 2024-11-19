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

   	auto& gravity = registry.gravities.emplace(entity);
    gravity.drag = true;

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

Entity createWinner(RenderSystem* renderer, int x, int y, int s) {
	auto entity = Entity();
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
    registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
     motion.velocity = { 0, 0 }; 
	 motion.position = {x, y};

	auto& winner = registry.winner.emplace(entity);
	auto& animation = registry.animations.emplace(entity);
    if (s == 1) { // red player
        animation.frames = {
            TEXTURE_ASSET_ID::RW1,
            TEXTURE_ASSET_ID::RW2,
            TEXTURE_ASSET_ID::RW3,
			TEXTURE_ASSET_ID::RW4,
			TEXTURE_ASSET_ID::RW5,
			TEXTURE_ASSET_ID::RW6,
			TEXTURE_ASSET_ID::RW7,
			TEXTURE_ASSET_ID::RW8,
			TEXTURE_ASSET_ID::RW9,
			TEXTURE_ASSET_ID::RW10,
			TEXTURE_ASSET_ID::RW11,
			TEXTURE_ASSET_ID::RW12,
			TEXTURE_ASSET_ID::RW13,
			TEXTURE_ASSET_ID::RW14,

        };
        motion.scale = { PLAYER_WIDTH, PLAYER_HEIGHT };
    } else { // blue player
        animation.frames = {
            TEXTURE_ASSET_ID::BW1,
            TEXTURE_ASSET_ID::BW2,
            TEXTURE_ASSET_ID::BW3,
			TEXTURE_ASSET_ID::BW4,
			TEXTURE_ASSET_ID::BW5,
			TEXTURE_ASSET_ID::BW6,
			TEXTURE_ASSET_ID::BW7,
			TEXTURE_ASSET_ID::BW8,
			TEXTURE_ASSET_ID::BW9,
			TEXTURE_ASSET_ID::BW10,
			TEXTURE_ASSET_ID::BW11,
			TEXTURE_ASSET_ID::BW12,
			TEXTURE_ASSET_ID::BW13,
			TEXTURE_ASSET_ID::BW14
        };
        motion.scale = { PLAYER_WIDTH, PLAYER_HEIGHT };

		registry.renderRequests.insert(
        entity,
        { animation.frames[0], 
          EFFECT_ASSET_ID::TEXTURED,
          GEOMETRY_BUFFER_ID::SPRITE });
	}

    return entity;
}


// Create a portal at given pos
// create a block based on its center (position), and its width and height
Entity createPortal(RenderSystem* renderer, vec2 position, int width, int height) { 
    auto entity = Entity();
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PORTAL);
    registry.meshPtrs.emplace(entity, &mesh);

    auto& portal = registry.portals.emplace(entity);
    portal.x = int(position[0] - (width / 2));
    portal.y = int(position[1] - (height / 2));
    portal.width = width;
    portal.height = height;

    auto& motion = registry.motions.emplace(entity);
     motion.velocity = { 0, 0 };
     motion.position = {position[0] - (width / 2), position[1] - (height / 2)};
    motion.scale = {width, height};


    registry.renderRequests.insert(
        entity,
         { TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
             EFFECT_ASSET_ID::SALMON,
             GEOMETRY_BUFFER_ID::PORTAL });

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

Entity createIntro(RenderSystem* renderer, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = {0,0};
	motion.position = {width / 2, height / 2};
	motion.scale = {width, height};
	
	registry.renderRequests.insert(
		entity,
		{TEXTURE_ASSET_ID::INTRO1,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE}
	);
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

	if (!registry.stageSelection) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::INTRO,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	} else if (registry.stageSelection ==1 ) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::CITY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });
	} else if (registry.stageSelection == 2) {
		registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::DESERT,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE}
		);
	} else {
		registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::ICEMOUNTAIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE}
		);
	}

	return entity;
}

Entity createWinnerScreen(RenderSystem* renderer, int width, int height, int winner) {
	auto entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 };
 	motion.position = {width / 2, height / 2};
	motion.scale = {width, height};

	if (registry.gamewinner ==1 ){
		registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::REDWIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
			});
	} else {
		registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::BLUEWIN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
			}
		);
	}
	return entity;
}

Entity createStageChoice(RenderSystem* renderer, int x, int y, int width, int height, int stage) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	auto& stageChoice = registry.stages.emplace(entity);

	stageChoice.x = x;
	stageChoice.y = y;
	stageChoice.stage = stage;

	motion.velocity = { 0.f, 0.f };
 	motion.position = {x + (width / 2), y + (height / 2)};
	motion.scale = {width, height};

	if (stage ==1 ) {
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::CITY,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE });
	} else if (stage == 2) {
			registry.renderRequests.insert(
				entity,
				{TEXTURE_ASSET_ID::DESERT,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE}
			);
	} else {
		registry.renderRequests.insert(
				entity,
				{TEXTURE_ASSET_ID::ICEMOUNTAIN,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE}
		);
	}
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

std::vector<Entity> createBuckshot(RenderSystem* renderer, int side, vec2 position, int direction) {
	auto entity = Entity();
	auto entity2 = Entity();
	auto entity3 = Entity();
	auto entity4 = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.meshPtrs.emplace(entity2, &mesh);
	registry.meshPtrs.emplace(entity3, &mesh);
	registry.meshPtrs.emplace(entity4, &mesh);

	auto& bullet1 = registry.bullets.emplace(entity);
	bullet1.side = side;

	auto& bullet2 = registry.bullets.emplace(entity2);
	bullet2.side = side;

	auto& bullet3 = registry.bullets.emplace(entity3);
	bullet3.side = side;

	auto& bullet4 = registry.bullets.emplace(entity4);
	bullet4.side = side;


	auto& motion = registry.motions.emplace(entity);
	int dir = 0;
	if (direction == 0) dir = -1;
	else dir = 1;
 	motion.velocity = { 500 * dir, -30 }; 
 	motion.position = {position.x, position.y - 30.0f};;;
	motion.scale = { 8, 8 }; // width * height

	auto& motion2 = registry.motions.emplace(entity2);
 	motion2.velocity = { 500 * dir, -20 }; 
 	motion2.position = {position.x, position.y - 20.0f};;
	motion2.scale = { 8, 8 }; // width * height

	auto& motion3 = registry.motions.emplace(entity3);
 	motion3.velocity = { 500 * dir, 20 }; 
 	motion3.position = {position.x, position.y + 20.0f};;
	motion3.scale = { 8, 8 }; // width * height

	auto& motion4 = registry.motions.emplace(entity4);
 	motion4.velocity = { 500 * dir, 30 }; 
 	motion4.position = {position.x, position.y + 30.0f};;
	motion4.scale = { 8, 8 }; // width * height

	registry.renderRequests.insert(
		entity,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });
	
	registry.renderRequests.insert(
		entity2,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });
	
	registry.renderRequests.insert(
		entity3,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });
	
	registry.renderRequests.insert(
		entity4,
 		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no texture is needed
 			EFFECT_ASSET_ID::SALMON,
 			GEOMETRY_BUFFER_ID::SQUARE });
	
	return {entity, entity2, entity3, entity4};
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
    motion.scale = {8,8};

    registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::SALMON, GEOMETRY_BUFFER_ID::SQUARE});
    registry.colors.insert(entity, {0.0f, 1.0f, 0.0f});

    return entity;
}

Entity createLaserBeam(vec2 start, vec2 target) {
    auto beam = Entity();

    // Calculate the midpoint and angle between start and target
    vec2 midpoint = (start + target) * 0.5f;
    vec2 direction = normalize(target - start);
    float beamLength = length(target - start);

    // Set up the beam's motion properties
    Motion& motion = registry.motions.emplace(beam);
    motion.position = midpoint;
    motion.scale = {25.f, beamLength};
    motion.angle = -atan2(direction.x, direction.y);
    registry.colors.insert(beam, {1.0f, 1.0f, 0.0f});
    registry.renderRequests.insert(
        beam,
        {TEXTURE_ASSET_ID::TEXTURE_COUNT, EFFECT_ASSET_ID::SALMON, GEOMETRY_BUFFER_ID::SQUARE}
    );
	
	auto& lifetime = registry.lifetimes.emplace(beam);
    lifetime.counter_ms = 1000; // Laser beam lasts for 1000 milliseconds (1 second)
    return beam;
}
