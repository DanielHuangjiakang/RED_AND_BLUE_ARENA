#pragma once
#include "common.hpp"
#include <vector>
#include <queue>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

struct Item
{
    int id; // 0 = health potion, 1 = grenade, 2 = laser gun        
};

// Player component
struct Player
{
	int side; // side = 1 for blue, side = 2 for red
	bool jumpable = false;
	bool direction;  // 0 for left, 1 for right
	int health = 10;
	bool is_moving = false;
	float jump_accel = -600.f;
	float lr_accel = 1200.f;
	
	std::queue<Item> items;
};

struct GunTimer {
	float counter_ms = 600;
};

struct Bullet {
	int side; // side = 1 for blue, side = 2 for red
};

struct Background {

};

struct Grenade {
	int side; // side = 1 for blue, side = 2 for red
};

struct Explosion {
	bool damagable1 = true;
	bool damagable2 = true;
};

// Weapon component
struct Weapon
{
    std::string type;
	int damage;
};

struct Text {
	std::string text;
	vec2 position;
	bool is_visible;
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 1, 1 };
	float angle = 0.f;
};

struct Block {
	int x;
	int y;
	int width;
	int height;
	int moving; // 0 = no moving, 1 = horizontal movement, 2 = vertical movement
	vec2 travelled_dist = {0, 0};
};

struct StageChoice {
	int stage;
	int x;
	int y;
};

// A sturct for portals, similar to how blocks work but with different collision.
struct Portal {
	int x;
	int y;
	int width;
	int height;
};

struct Gravity {
	vec2 g = {0.f, 750.f};
	bool drag = false;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
	int direction = 0; // 1 for top, 2 for bottom, 3 for left, 4 for right
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Stage {
    vec2 groundPosition;
    vec2 groundSize;
    std::vector<vec2> platformPositions;
    std::vector<vec2> platformSizes;
	std::vector<int> moving;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	FISH = 0,
	EEL = FISH + 1,

	CITY = EEL + 1,
	RED_RUN_1 = CITY + 1,
	RED_RUN_2 = RED_RUN_1 + 1,
	RED_RUN_3 = RED_RUN_2 + 1,
	BLUE_RUN_1 = RED_RUN_3 + 1,
	BLUE_RUN_2 = BLUE_RUN_1 + 1,
	BLUE_RUN_3 = BLUE_RUN_2 + 1,
	BULLET = BLUE_RUN_3 + 1,
	BLOCK = BULLET + 1,
	PAD = BLOCK + 1,
	RED_GUN = PAD + 1,
	BLUE_GUN = RED_GUN + 1,
	HELP = BLUE_GUN + 1,
	DESERT = HELP +1,
	INTRO = DESERT +1,
	INTRO1 = INTRO+1,
	GRENADE = INTRO1 + 1,
	POTION = GRENADE + 1,
	LASER = POTION + 1,
	LONG_LASER = LASER + 1,
	EXPLOSION = LONG_LASER + 1,
	ICEMOUNTAIN = EXPLOSION + 1,
	ICEPAD = ICEMOUNTAIN +1,
	SCIFI = ICEPAD + 1,
	LASER2 = SCIFI + 1,
	TEXTURE_COUNT = LASER2 + 1

};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	FONT = EGG + 1,
	SALMON = FONT + 1,
	TEXTURED = SALMON + 1,
	WATER = TEXTURED + 1,
	EFFECT_COUNT = WATER + 1
};

const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
    SALMON = 0,
    SPRITE = SALMON + 1,
    EGG = SPRITE + 1,
    SQUARE = EGG + 1,
    DEBUG_LINE = SQUARE + 1,
    SCREEN_TRIANGLE = DEBUG_LINE + 1,
    PORTAL = SCREEN_TRIANGLE + 1,
    HEALTH_BAR = PORTAL + 1,    
    GEOMETRY_COUNT = HEALTH_BAR + 1 
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;




struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};


// animation
struct AnimationFrame {
    std::vector<TEXTURE_ASSET_ID> frames;
    int current_frame = 0;
    float frame_time = 0.f;
};
  
struct Laser {};

struct Laser2 {
	int side;
	bool damagable = true;
};

struct Lifetime {
    float counter_ms;
};