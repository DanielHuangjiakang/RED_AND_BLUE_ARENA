#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
const float PLAYER_WIDTH = 75.0f;
const float PLAYER_HEIGHT = 90.0f;

Entity createBackground(RenderSystem* renderer, int width, int height);
Entity createHelpPanel(RenderSystem* renderer, int width, int height);

// the player
Entity createPlayer(RenderSystem* renderer, int side, vec2 position, bool direction);
Entity createGun(RenderSystem* renderer, int side, vec2 position);

// stage blocks
Entity createBlock1(RenderSystem* renderer, int x, int y, int width, int height);
Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height);

// the bullet
Entity createBullet(RenderSystem* renderer, int side, vec2 position, int direction);

// text
Entity createText(RenderSystem* renderer, const std::string& text, vec2 position, bool is_visible);