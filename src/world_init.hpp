#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture

// the player
Entity createPlayer(RenderSystem* renderer, int side, vec2 position, bool direction);

// stage blocks
Entity createBlock1(RenderSystem* renderer, int x, int y, int width, int height);
Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height);

// the bullet
Entity createBullet(RenderSystem* renderer, int side, vec2 position, int direction);
Entity createLaser(RenderSystem* renderer);
Entity createLaserBeam(vec2 start, vec2 end);