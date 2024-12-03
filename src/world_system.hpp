#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>
#include <deque>
#include <string>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

#include "animation_system.hpp"
#include "DecisionTree.hpp"

#include <random>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
struct ItemSpawnInfo {
    vec2 position;
    int itemType;
    Entity entity; // The item entity
    float respawnTimer;
};


class WorldSystem
{
public:
	WorldSystem();

	// keeping track of printing text for toogling +3 lives
	int toogle_life;
	int toogle_life_timer;

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// for keeping track of remaining shots
	int remaining_bullet_shots_p1 = 10;
	int remaining_bullet_shots_p2 = 10;
	int remaining_buck_p1 = 3;
	int remaining_buck_p2 = 3;

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;

	void loadMatchRecords();

	void recordMatchResult();

	void createStage(int currentStage);

	float fps;

	bool showMatchRecords = false; 

	int num_p1_wins = 0; // keeping track of p1 wins

	int num_p2_wins = 0; // keeping track of p2 wins

	int rounds = 9; // changing the maps will reset rounds

	std::deque<std::string> match_records;
    std::vector<std::pair<vec2, int>> itemSpawnPositions; // Stores the positions and types of items for respawning
    std::vector<float> itemRespawnTimers;                 // Timers for each item respawn
	std::vector<ItemSpawnInfo> itemSpawnInfos;
    const float ITEM_RESPAWN_DELAY_MS = 1000.0f;          // 5 seconds delay
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_shoot();

	// For handling reload
	bool on_reload_p1 = 0;
	bool on_reload_p2 = 0;
	float reloading_time_p1 = 0.0f;
	float reloading_time_p2 = 0.0f;

	// For counting rounds and introducing new features
	bool laser_toogle = 0; // start having lasers on 4th round
	bool item_toogle = 0; // start having items on 7th round

	// restart level
	void restart_game();

	// 
	bool movable = true;

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	RenderSystem* renderer;
	AnimationSystem animation_system;
	float current_speed;
	Entity player1;
	Entity player2;
	Entity gun1;
	Entity gun2;

	bool player1_right_button = false;
	bool player1_left_button = false;
	int player1_shooting = 0;
	bool player1_item = true;
	bool player2_right_button = false;
	bool player2_left_button = false;
	int player2_shooting = 0;
	bool player2_item = true;
	bool player1_fall = false;
	bool player2_fall = false;

	float next_item_spawn;

	// Stage atrributes
	Entity helpPanel;
	Entity helpText;
	Entity background;
	Entity ground;
	Entity platform1;
	Entity platform2;
	Entity platform3;

	//Portals
	Entity portal1;
	Entity portal2;

	// music references
	Mix_Music* snow_music;
	Mix_Music* city_music;
	Mix_Music* desert_music;
	Mix_Music* mapselections_music;
	Mix_Music* tutorial_music;
	

	Mix_Chunk* end_music;
	Mix_Chunk* hit_sound;
	Mix_Chunk* shoot_sound;
	Mix_Chunk* laser_sound;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;
	Mix_Chunk* portal_sound;
	Mix_Chunk* buck_shot_sound;
	Mix_Chunk* select_music;
	
	Mix_Chunk* laser2_sound;
	Mix_Chunk* healthpickup_sound;
	Mix_Chunk* explosion_sound;
	Mix_Chunk* reload_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

  	float calculateDistance(vec2 pos1, vec2 pos2);
	void updateLaserVelocity(Entity laserEntity, Motion& player1Motion, Motion& player2Motion);
	DecisionTreeNode* rootNode;
  	float laserRange = 10.0f;
  	float laserCoolDownTime = 2000.0f;  // CoolDown time in milliseconds
  	float laserCoolDownTimer = 0.0f;
	void initializeLaserAI();
	bool isPlayerInRange();
	void handleLaserCollisions();
	bool isLaserInRange(vec2 laserPosition, vec2 playerPosition);
	bool isMouseOverEntity(vec2 mouse_position, Entity entity);
	void handleEntityClick(Entity entity);
	void on_mouse_button(int button, int action, int mods);
	int time_since_last_frame = 0;
	int currentDelay = 0;
	bool isLaserFiring = false;
	float laserFireCounter = 0.0f; 
	vec2 target;

};
