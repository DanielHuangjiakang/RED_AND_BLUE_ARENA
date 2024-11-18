#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"

#include "animation_system.hpp"
#include "DecisionTree.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;

	float fps;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_shoot();

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
	bool player2_right_button = false;
	bool player2_left_button = false;
	int player2_shooting = 0;

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
	Mix_Music* background_music;

	Mix_Chunk* end_music;
	Mix_Chunk* hit_sound;
	Mix_Chunk* shoot_sound;
	Mix_Chunk* laser_sound;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;
	Mix_Chunk* portal_sound;
	Mix_Chunk* buck_shot_sound;
	

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
};
