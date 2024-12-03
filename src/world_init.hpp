#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
const float PLAYER_WIDTH = 75.0f;
const float PLAYER_HEIGHT = 80.0f;

Entity createBackground(RenderSystem* renderer, int width, int height);
Entity createIntro(RenderSystem* renderer, int width, int height);
Entity createHelpPanel(RenderSystem* renderer, int width, int height);
Entity createStageChoice(RenderSystem* renderer, int x, int y, int width, int height, int stage);

// the player
Entity createPlayer(RenderSystem* renderer, int side, vec2 position, bool direction);
Entity createGun(RenderSystem* renderer, int side, vec2 position);

// stage blocks
Entity createBlock1(RenderSystem* renderer, int x, int y, int width, int height);
Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height, int moving);


// Portals
Entity createPortal(RenderSystem* renderer, vec2 position, int width, int height);

// the bullet
Entity createBullet(RenderSystem* renderer, int side, vec2 position, int direction);

Entity createRandomItem(RenderSystem* renderer, Motion motion);
Entity createGrenade(RenderSystem* renderer, vec2 position, int direction, int side);
Entity createExplosion(vec2 position);
Entity createSpecificItem(RenderSystem* renderer, Motion motion, int itemID);


// the arrow
std::vector<Entity> createBuckshot(RenderSystem* renderer, int side, vec2 position, int direction);
Entity createLaser(RenderSystem* renderer);
Entity createLaserBeam(vec2 start, vec2 end);
Entity createLaserBeam2(vec2 start, int direction, int side);
