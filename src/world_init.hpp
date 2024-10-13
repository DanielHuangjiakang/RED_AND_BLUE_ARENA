#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// These are hardcoded to the dimensions of the entity texture
// BB = bounding box
const float FISH_BB_WIDTH  = 0.6f * 165.f;
const float FISH_BB_HEIGHT = 0.6f * 165.f;
const float EEL_BB_WIDTH   = 0.6f * 300.f;	// 1001
const float EEL_BB_HEIGHT  = 0.6f * 202.f;	// 870
const float PLAYER_HEIGHT  = 0.6f * 165.f;
const float PLAYER_WIDTH   = 0.6f * 165.f;

// the player
Entity createPlayer(RenderSystem* renderer, int side, vec2 position, bool direction);

// Bullet
Entity  createBullet(RenderSystem* renderer, vec2 position, bool direction);

// stage blocks
Entity createBlock1(RenderSystem* renderer, int x, int y, int width, int height);
Entity createBlock2(RenderSystem* renderer, vec2 position, int width, int height);
Entity createLaser(RenderSystem* renderer, vec2 target);