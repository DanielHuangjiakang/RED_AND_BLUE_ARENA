#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
const float PLAYER_WIDTH = 75.0f;
const float PLAYER_HEIGHT = 90.0f;

Entity createBackground(RenderSystem* renderer, int width, int height);
Entity createIntro(RenderSystem* renderer, int width, int height);
Entity createHelpPanel(RenderSystem* renderer, int width, int height);
Entity createStageChoice(RenderSystem* renderer, int x, int y, int width, int height, int stage);
Entity createWinnerScreen(RenderSystem* renderer, int width, int height, int winner);
Entity createWinner(RenderSystem* renderer, int x, int y, int s);

// the player
Entity createPlayer(RenderSystem* renderer, int side, vec2 position, bool direction);
Entity createGun(RenderSystem* renderer, int side, vec2 position);

// stage blocks
Entity createBlock1(RenderSystem* renderer, int x, int y, int width, int height);
Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height);


// Portals
Entity createPortal(RenderSystem* renderer, vec2 position, int width, int height);

// the bullet
Entity createBullet(RenderSystem* renderer, int side, vec2 position, int direction);

// the arrow
std::vector<Entity> createBuckshot(RenderSystem* renderer, int side, vec2 position, int direction);
Entity createLaser(RenderSystem* renderer);
Entity createLaserBeam(vec2 start, vec2 end);
