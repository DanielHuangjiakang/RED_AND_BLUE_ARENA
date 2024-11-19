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

Entity createBackground(RenderSystem* renderer, int width, int height) {
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
 	motion.velocity = { 0, 0 };
 	motion.position = {width / 2, height / 2};
	motion.scale = {width, height};

	registry.backgrounds.emplace(entity);
	
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
 	motion.velocity = { 500 * dir, -120 }; 
 	motion.position = {position.x, position.y};
	motion.scale = { 15, 6 }; // width * height

	auto& motion2 = registry.motions.emplace(entity2);
 	motion2.velocity = { 500 * dir, -40 }; 
 	motion2.position = {position.x, position.y};
	motion2.scale = { 15, 6 }; // width * height

	auto& motion3 = registry.motions.emplace(entity3);
 	motion3.velocity = { 500 * dir, 40 }; 
 	motion3.position = {position.x, position.y};
	motion3.scale = { 15, 6 }; // width * height

	auto& motion4 = registry.motions.emplace(entity4);
 	motion4.velocity = { 500 * dir, 120 }; 
 	motion4.position = {position.x, position.y};
	motion4.scale = { 15, 6 }; // width * height

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
    std::uniform_real_distribution<float> distY(0.f, static_cast<float>(window_height_px / 2));
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
    motion.scale = {10.f, beamLength};
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

Entity createLaserBeam2(vec2 start, int direction, int side) {
	auto beam = Entity();
	int dir = 0;
	if (direction == 0) dir = -1;
	else dir = 1;

	vec2 target = {start.x + dir * 1210, start.y};
	vec2 midpoint = (start + target) * 0.5f;
	Motion& motion = registry.motions.emplace(beam);
    motion.position = midpoint;

	Laser2& laser2 = registry.lasers2.emplace(beam);
	laser2.side = side;

    motion.scale = {1210.0f, 44.0f};
    registry.renderRequests.insert(
        beam,
        {TEXTURE_ASSET_ID::LONG_LASER, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    );
	
	auto& lifetime = registry.lifetimes.emplace(beam);
    lifetime.counter_ms = 150; // Laser beam lasts for 1000 milliseconds (1 second)
    return beam;
}

Entity createRandomItem(RenderSystem* renderer, Motion motion) {
	auto entity = Entity();
	Item& item = registry.items.emplace(entity);
	std::random_device rd;
    std::default_random_engine rng(rd());
    std::uniform_int_distribution<int> dist(0, 2);
    item.id = dist(rng);

	Motion& item_motion = registry.motions.emplace(entity);
	item_motion = motion;

	if (item.id == 0) {
		registry.renderRequests.insert(
        	entity,
        	{TEXTURE_ASSET_ID::POTION, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    	);	
	}
	else if (item.id == 1) {
		registry.renderRequests.insert(
        	entity,
        	{TEXTURE_ASSET_ID::GRENADE, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    	);
	}
	
	else  {
		item_motion.scale = {45, 20};
		item_motion.angle = 3 * M_PI / 4;
		registry.renderRequests.insert(
        	entity,
        	{TEXTURE_ASSET_ID::LASER, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    	);
	}

	return entity;
}

Entity createGrenade(RenderSystem* renderer, vec2 position, int direction, int side) {
	auto entity = Entity();
	Grenade& grenade = registry.grenades.emplace(entity);
	grenade.side = side;

	auto& motion = registry.motions.emplace(entity);
	int dir = 0;
	if (direction == 0) dir = -1;
	else dir = 1;
 	motion.velocity = { 500 * dir, -100 }; 
	motion.scale = { 30, 45 }; // width * height
	motion.position = {position.x + dir * (motion.scale.x), position.y - motion.scale.y};

	Gravity& gravity = registry.gravities.emplace(entity);
	gravity.g.y = 300.f;

	registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::GRENADE, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    );
	
	return entity;
}

Entity createExplosion(vec2 position) {
	auto entity = Entity();
	registry.explosions.emplace(entity);

	auto& motion = registry.motions.emplace(entity);
 	motion.position = position;
	motion.scale = { 200, 165 }; // width * height

	registry.renderRequests.insert(
        entity,
        {TEXTURE_ASSET_ID::EXPLOSION, EFFECT_ASSET_ID::TEXTURED, GEOMETRY_BUFFER_ID::SPRITE}
    );

	auto& lifetime = registry.lifetimes.emplace(entity);
    lifetime.counter_ms = 150; 
	return entity;
}