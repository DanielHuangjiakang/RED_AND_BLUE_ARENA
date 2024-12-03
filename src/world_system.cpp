// Header
#include "world_system.hpp"
#include "GLFW/glfw3.h"
#include "SDL_mixer.h"
#include "common.hpp"
#include "components.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <deque>
#include <fstream>
#include <algorithm>   

#include "physics_system.hpp"

#include "animation_system.hpp"

#include "tiny_ecs_registry.hpp"
#include "decisionTree.hpp"

// for portal randomization
#include <random>

using namespace std;

const size_t MAX_NUM_ITEMS = 2;
size_t ITEM_SPAWN_DELAY_MS = 8000;

std::vector<Stage> stagesArray = {
    // Stage 1
    {
        {{0, window_height_px - 50}}, 
		{{window_width_px, 50}}, 
        {{window_width_px / 4, window_height_px - 250}, {window_width_px / 2, window_height_px - 450}, {3 * window_width_px / 4, window_height_px - 250}}, // Platform positions
        {{250, 10}, {250, 10}, {250, 10}},  // Platform sizes
		{0, 0, 0}  // No moving
    },
    // Stage 2
    {
		{{0, window_height_px - 50}}, 
		{{window_width_px, 50}},
        {{window_width_px / 4, window_height_px - 450}, {window_width_px / 2, window_height_px - 250}, {3 * window_width_px / 4, window_height_px - 450}}, // Platform positions
        {{250, 10}, {250, 10}, {250, 10}},  // Platform sizes
		{0, 1, 0}  // Bottom moves
    },
    // Stage 3
    {
        {{0, window_height_px - 50}, {window_width_px / 2, window_height_px - 50}}, 
		{{window_width_px / 2, 50}, {window_width_px / 2, 50}}, 
        {{window_width_px / 4, window_height_px - 250}, {3 * window_width_px / 4, window_height_px - 250}, {window_width_px / 4, window_height_px - 450}, {3 * window_width_px / 4, window_height_px - 450}}, // Platform positions
        {{300, 10}, {300, 10}, {200, 10}, {200, 10}},  // Platform sizes
		{0, 0, 0, 0}  // No moving
    },
	// Stage 4
    {
        {{0, window_height_px - 50}, {window_width_px / 3 + 250, window_height_px - 50}}, 
		{{window_width_px / 3, 50}, {2 * window_width_px / 3 - 200, 50}}, 
        {{window_width_px / 3, window_height_px / 2 + 100}, {3 * window_width_px / 4, window_height_px - 450}}, // Platform positions
        {{200, 10}, {480, 10}},  // Platform sizes
		{2, 0}  // Smaller platform moves
    },
	// Stage 5
    {
        {}, 
		{}, 
        {{200, window_height_px / 2 + 100}, {window_width_px - 200, window_height_px / 2 + 100}, {window_width_px / 2, window_height_px - 450}, {window_width_px / 2, window_height_px - 50}}, // Platform positions
        {{150, 10}, {150, 10}, {150, 10}, {150, 10}},  // Platform sizes
		{2, 2, 1, 3}  // All platforms move
    },
	// Tutorial Stage 6
	{
    {{0, window_height_px - 50}}, {{window_width_px, 50}},
    {
        {window_width_px / 4, window_height_px - 200},
        {window_width_px / 2, window_height_px - 300},
        {3 * window_width_px / 4, window_height_px - 200}
    }, // Platform positions
    {{200, 10}, {200, 10}, {200, 10}},  // Platform sizes
    {0, 0, 0}
	}
};

// create the underwater world
WorldSystem::WorldSystem()
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{

	// destroy music components
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);

	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace
{
	void glfw_err_cb(int error, const char *desc)
	{
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow *WorldSystem::create_window()
{
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Project", nullptr, nullptr);
	if (window == nullptr)
	{
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow *wnd, double _0, double _1)
	{ ((WorldSystem *)glfwGetWindowUserPointer(wnd))->on_mouse_move({_0, _1}); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
	{
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	snow_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	city_music = Mix_LoadMUS(audio_path("city.wav").c_str());
	desert_music = Mix_LoadMUS(audio_path("desert.wav").c_str());
	mapselections_music = Mix_LoadMUS(audio_path("mapselection.wav").c_str());
	tutorial_music = Mix_LoadMUS(audio_path("tutorial.wav").c_str());
	end_music = Mix_LoadWAV(audio_path("end_music.wav").c_str());
	hit_sound = Mix_LoadWAV(audio_path("hit_sound.wav").c_str());
	shoot_sound = Mix_LoadWAV(audio_path("shoot.wav").c_str());
	laser_sound = Mix_LoadWAV(audio_path("laser.wav").c_str());
	portal_sound = Mix_LoadWAV(audio_path("portal.wav").c_str());
	buck_shot_sound = Mix_LoadWAV(audio_path("buck_shot.wav").c_str());


	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());
	reload_sound = Mix_LoadWAV(audio_path("reload.wav").c_str());
	laser2_sound = Mix_LoadWAV(audio_path("laser2.wav").c_str());
	healthpickup_sound = Mix_LoadWAV(audio_path("healthpickup.wav").c_str());
	explosion_sound = Mix_LoadWAV(audio_path("explosion.wav").c_str());
	select_music = Mix_LoadWAV(audio_path("select.wav").c_str());

	if (salmon_dead_sound == nullptr || salmon_eat_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("city.wav").c_str(),
				audio_path("desert.wav").c_str(),
				audio_path("mapselection.wav").c_str(),
				audio_path("death_sound.wav").c_str(),
				audio_path("eat_sound.wav").c_str(),
				audio_path("shoot.wav").c_str(),
				audio_path("hit_sound.wav").c_str(),
				audio_path("end_music.wav").c_str(),
				audio_path("portal.wav").c_str(),
				audio_path("buck_shot.wav").c_str(),
				audio_path("laser2.wav").c_str(),
				audio_path("healthpickup.wav").c_str(),
				audio_path("explosion.wav").c_str(),
				audio_path("select.wav").c_str(),
				audio_path("tutorial.wav").c_str(),
				audio_path("laser.wav").c_str());
		return nullptr;
	}


	// Adjust the volume for the sound effects
	Mix_VolumeChunk(hit_sound, 10);
	Mix_VolumeChunk(shoot_sound, 10);
	Mix_VolumeChunk(buck_shot_sound, 10);
	Mix_VolumeChunk(laser_sound, 10);
	Mix_VolumeChunk(select_music, 20);
	Mix_VolumeChunk(end_music, 10);
	Mix_VolumeChunk(portal_sound, 10);

	glfwSetMouseButtonCallback(window, [](GLFWwindow* wnd, int button, int action, int mods) {
		((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(button, action, mods);
	});

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;

	// Load high score from file
	loadMatchRecords();
	// Playing background music indefinitely
	// Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update)
{
	static float total_time = 0.0f;
    static int frame_count = 0;

    total_time += elapsed_ms_since_last_update;
    frame_count++;

	// updating the timer for printing text.
	toogle_life_timer -= elapsed_ms_since_last_update;
	time_since_last_frame = elapsed_ms_since_last_update;
	// std::cout << std::to_string(rounds) << std::endl;
	// std::cout << std::to_string(num_p1_wins) << std::endl;

    if (total_time > 1000.0f) {
        fps = frame_count / (total_time / 1000.0f);
		std::stringstream title_ss;
        title_ss << "Game Screen - FPS: " << static_cast<int>(fps);
        glfwSetWindowTitle(window, title_ss.str().c_str());

        total_time = 0.0f;
        frame_count = 0;
    }

	// Only execute this logic in Stage 6 (Tutorial Mode)
	if (registry.stageSelection == 6) {
		for (size_t i = 0; i < itemSpawnInfos.size(); ++i) {
			// Check if the item entity no longer has an Item component
			if (!registry.items.has(itemSpawnInfos[i].entity)) {
				if (itemSpawnInfos[i].respawnTimer > 0.0f) {
					// Decrease the respawn timer
					itemSpawnInfos[i].respawnTimer -= elapsed_ms_since_last_update;
					if (itemSpawnInfos[i].respawnTimer <= 0.0f) {
						// Respawn the item
						Motion item_motion;
						item_motion.position = itemSpawnInfos[i].position;
						item_motion.scale = {30, 45};
						Entity itemEntity = createSpecificItem(renderer, item_motion, itemSpawnInfos[i].itemType);
						itemSpawnInfos[i].entity = itemEntity;
					}
				}
			}
		}
	}

	if (isLaserFiring) 
	{
		laserFireCounter += elapsed_ms_since_last_update;

		// Check if 0.25 seconds have passed
		if (laserFireCounter >= 500.0f)
		{
			// Create the laser beam
			createLaserBeam({window_width_px / 2, 0}, target);
			handleLaserCollisions();

			// Reset the counter and the firing flag
			laserFireCounter = 0.0f;
			isLaserFiring = false;
		}
	}

	// handling reload
	if (reloading_time_p1 > 0)
	{
		reloading_time_p1 = std::max(reloading_time_p1 - elapsed_ms_since_last_update, 0.0f);

		if (reloading_time_p1 == 0)
		{
			// refill the shots
			remaining_buck_p1 = 3;
			remaining_bullet_shots_p1 = 10;
		}
		
	}

	if (reloading_time_p2 > 0)
	{
		reloading_time_p2 = std::max(reloading_time_p2 - elapsed_ms_since_last_update, 0.0f);
		if (reloading_time_p2 == 0)
		{
			// refill the shots
			remaining_buck_p2 = 3;
			remaining_bullet_shots_p2 = 10;
		}
	}
	
	// Check if 0.25 seconds have passed
	if (laserFireCounter >= 500.0f) {
		// Create the laser beam
		createLaserBeam({window_width_px / 2, 0}, target);
		handleLaserCollisions();

		// Reset the counter and the firing flag
		laserFireCounter = 0.0f;
		isLaserFiring = false;
	}

	if (!registry.intro && registry.stageSelection) {
	
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0) registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto &motions_registry = registry.motions;
  
   	// Decrease cooldown timer each frame
    if (laserCoolDownTimer > 0) {
        laserCoolDownTimer -= elapsed_ms_since_last_update;
    }
	if (rootNode) {
        rootNode->execute();
    }

    // Laser updates and other game mechanics (e.g., enforcing boundaries, handling collisions)
    for (Entity entity : registry.lasers.entities) {

        // Reset coolDown timer after an attack
        if (laserCoolDownTimer <= 0 && isPlayerInRange()) {
            laserCoolDownTimer = 3000;  // 3-second coolDown after attacking
        }
		for (Entity entity : registry.lifetimes.entities) {
        Lifetime& lifetime = registry.lifetimes.get(entity);
        lifetime.counter_ms -= elapsed_ms_since_last_update;

        // Remove the entity when its lifetime expires
        if (lifetime.counter_ms <= 0) {
            registry.remove_all_components_of(entity);
        }
    }
    }

	for (Entity entity : registry.lightUps.entities) {
		// progress timer
		if (registry.lightUps.has(entity)) {
			LightUp& counter = registry.lightUps.get(entity);
			counter.counter_ms -= elapsed_ms_since_last_update;

			// remove the light up effect once the timer expired
			if (counter.counter_ms < 0) {
				registry.lightUps.remove(entity);
			}
		} 
	}

	// Update player1's position and enforce boundaries
    Motion& motion1 = registry.motions.get(player1);
	Motion& gunMotion1 = registry.motions.get(gun1);
    // Make gun follow player1
	int dir1 = motion1.scale.x > 0 ? 1 : -1;		
    gunMotion1.position = motion1.position + vec2(35 * dir1, 0);
    gunMotion1.scale.x = motion1.scale.x; // Match player direction
    if (motion1.position.x < abs(motion1.scale[0]/2)) {
        motion1.position.x = abs(motion1.scale.x/2); // Stop at left boundary
		motion1.velocity.x = 0;
		gunMotion1.velocity[0] = 0;
    } else if (motion1.position.x + motion1.scale.x > window_width_px + motion1.scale[0]/2) {
        motion1.position.x = window_width_px - motion1.scale[0]/2; // Stop at right boundary
		motion1.velocity[0] = 0;
		gunMotion1.velocity[0] = 0;
    }
    if (motion1.position.y < motion1.scale[1]/2) {
        motion1.position.y = motion1.scale[1]/2; // Stop at the top boundary
		motion1.velocity[1] = 0;
		gunMotion1.velocity[1] = 0;
    }

    // Update player2's position and enforce boundaries
    Motion& motion2 = registry.motions.get(player2);
	Motion& gunMotion2 = registry.motions.get(gun2);
    // Make gun follow player2
	int dir2 = motion2.scale.x > 0 ? 1 : -1;		
    gunMotion2.position = motion2.position + vec2(35 * dir2, 0);
    gunMotion2.scale.x = motion2.scale.x; // Match player direction
    if (motion2.position.x < abs(motion2.scale[0]/2)) {
        motion2.position.x = abs(motion2.scale[0]/2); // Stop at left boundary
		motion2.velocity.x = 0;
		gunMotion2.velocity[0] = 0;
    } else if (motion2.position.x + motion2.scale.x > window_width_px + motion2.scale[0]/2) {
        motion2.position.x =  window_width_px - motion2.scale[0]/2; // Stop at right boundary
		motion2.velocity[0] = 0;
		gunMotion2.velocity[0] = 0;
    }
    if (motion2.position.y < motion2.scale[1]/2) {
        motion2.position.y = motion2.scale[1]/2; // Stop at the top boundary
		motion2.velocity[1] = 0;
		gunMotion2.velocity[1] = 0;
    }


	// Remove entities that leave the screen on the left/right side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motions_registry.components[i];
		Entity entity = motions_registry.entities[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f || motion.position.x - abs(motion.scale.x) > window_width_px)
		{
			if (!registry.players.has(entity)) {
				if (registry.grenades.has(entity)) {
					createExplosion(motion.position);
					Mix_PlayChannel(-1, explosion_sound, 0);
				}
				registry.remove_all_components_of(entity);
			}
		}

		if (motion.position.y - abs(motion.scale.y) > window_height_px)
		{
			if (registry.players.has(entity)) {
				Player& player = registry.players.get(entity);
				if (player.side == 1 && !player1_fall) {
					player.health = 0;
					if (!registry.deathTimers.has(entity)) registry.deathTimers.emplace(entity);
					// end music
					Mix_PlayChannel(-1, end_music, 0);
					movable = false;
					rounds--;
					num_p2_wins++;
					player1_fall = true;
				}

				if (player.side == 2 && !player2_fall) {
					player.health = 0;
					if (!registry.deathTimers.has(entity)) registry.deathTimers.emplace(entity);
					// end music
					Mix_PlayChannel(-1, end_music, 0);
					movable = false;
					rounds--;
					num_p1_wins++;
					player2_fall = true;
				}
				// registry.winner = player.side == 1 ? 2 : 1;
				// createBackground(renderer, window_width_px, window_height_px);

				if (rounds ==0) {
            		registry.winner = (num_p1_wins > num_p2_wins) ?  1 : 2;
              		createBackground(renderer, 	window_width_px, window_height_px);
					if (registry.deathTimers.size() == 0) {
						int w, h;
						glfwGetWindowSize(window, &w, &h);
						registry.stageSelection = 0;
						registry.winner = 0;
						registry.stages.clear();
						rounds = 9;
						ScreenState &screen = registry.screenStates.components[0];
						screen.darken_screen_factor = 0;
						num_p1_wins = 0;
						num_p2_wins = 0;
						restart_game();
					}
            	}
				// registry.remove_all_components_of(motions_registry.entities[i]);	
			}

		}
	}

	

	next_item_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.items.components.size() < MAX_NUM_ITEMS && next_item_spawn < 0.f && registry.stageSelection != 6 && item_toogle == true) {
		next_item_spawn = (3 * ITEM_SPAWN_DELAY_MS / 4) + uniform_dist(rng) * (ITEM_SPAWN_DELAY_MS / 4);

		// do rejection sampling on a circle with a hole in the center
		bool restart_flag = true;
		while (restart_flag) {
			float r = (1 + 3 * uniform_dist(rng)) / 4 * (std::min(window_height_px, window_width_px) / 2);
			float theta = uniform_dist(rng) * 2 * M_PI;

			Motion item_motion;
			item_motion.position = {(r * cos(theta)) + (window_width_px / 2), (r * sin(theta)) + (window_height_px / 2)};
			item_motion.scale = {30, 45};

			restart_flag = false;
			for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
				Motion &motion = motions_registry.components[i];
				if (collides(item_motion, motion) && !registry.bullets.has(motions_registry.entities[i]) && !registry.backgrounds.has(motions_registry.entities[i])) {
					restart_flag = true;
					break;
				}
			}

			if (!restart_flag) createRandomItem(renderer, item_motion);
		}
	}

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities)
	{
		// progress timer
		DeathTimer &counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms)
		{
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0)
		{
			int side = registry.players.get(entity).side;
			if (side == 2)
				std::cout << "Red Player Wins" << std::endl; // Red Wins
			else
				std::cout << "Blue Player Wins" << std::endl; // Blue Wins

			recordMatchResult();

			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			registry.winner =0;
			restart_game();
			return true;
		}
	}
	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	for (Entity entity : registry.gunTimers.entities)
	{
		GunTimer &counter = registry.gunTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0)
		{
			registry.gunTimers.remove(entity);
		}
	}

	on_shoot();

	// Update animations
	animation_system.step(elapsed_ms_since_last_update);
	}

	return true;
}

// Reset the world state to its initial state

void WorldSystem::restart_game() {
	remaining_bullet_shots_p1 = 10;
	remaining_bullet_shots_p2 = 10;
	remaining_buck_p1 = 3;
	remaining_buck_p2 = 3;
	player1_fall = false;
	player2_fall = false;
	if (rounds <= 7)
	{
		laser_toogle = true;
	}

	if (rounds <= 5)
	{
		item_toogle = true;
	}
	
	// resetting rounds per map

	// Create an intro screen
	if (registry.intro) {
		// Create intro entities
		Entity introBackground = createIntro(renderer, window_width_px, window_height_px);
	}

	// Create a Stage Selection screen when a key is pressed
	if (!registry.intro && !registry.stageSelection && !registry.winner) {
		// Create stage selection entities
		Entity stageSelectionBackground = createBackground(renderer, window_width_px, window_height_px);

		// Create stage button entities
		Entity stageButton1 = createStageChoice(renderer, 10, window_height_px / 2-100, 400, 200, 1);
		Entity stageButton2 = createStageChoice(renderer, window_width_px / 2-200, window_height_px / 2-100, 400, 200, 2);
		Entity stageButton3 = createStageChoice(renderer, 3 * window_width_px/4-100, window_height_px / 2-100, 400, 200, 3);
		Entity stageButton4 = createStageChoice(renderer, 10, window_height_px / 2+120, 400, 200, 4);
		Entity stageButton5 = createStageChoice(renderer, window_width_px / 2-200, window_height_px / 2+120, 400, 200, 5);
		// Tutorial Stage Button
    	Entity tutorialButton = createStageChoice(renderer, 3 * window_width_px/4-100, window_height_px / 2+120, 400, 200, 6);
	}

	if (registry.stageSelection == 1) {
		Mix_PlayMusic(city_music, -1); // Play city music
		Mix_VolumeMusic(4);
	} else if (registry.stageSelection == 2) {
		Mix_PlayMusic(desert_music, -1); // Play desert music
		Mix_VolumeMusic(4);
	} else if (registry.stageSelection == 3) {
		Mix_PlayMusic(snow_music, -1); // Play snow music
			Mix_VolumeMusic(4);
	} else if (registry.stageSelection == 0 && !registry.intro) {
		Mix_PlayMusic(mapselections_music, -1);
		Mix_VolumeMusic(4);
	} else if (registry.stageSelection == 6) { // Tutorial Stage
    	Mix_PlayMusic(tutorial_music, -1); 
    	Mix_VolumeMusic(4);
	} else if (registry.stageSelection == 5 || registry.stageSelection == 4) {
		Mix_PlayMusic(city_music, -1); // Play city music
		Mix_VolumeMusic(4);
	}

	if (!registry.intro && registry.stageSelection) {
	movable = true;
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

    background = createBackground(renderer, window_width_px, window_height_px);

	createStage(registry.stageSelection - 1);
	}
}

// Compute collisions between entities
void WorldSystem::handle_collisions()
{
	// Loop over all collisions detected by the physics system
	auto &collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++)
	{
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;
		int direction = collisionsRegistry.components[i].direction;

		if (registry.players.has(entity) && registry.blocks.has(entity_other)) {
			Motion& motion = registry.motions.get(entity);
			Block& block = registry.blocks.get(entity_other);
			Motion& motion_block = registry.motions.get(entity_other);
			Player& player = registry.players.get(entity);
			if (direction == 1) { // top collision
				if (motion.velocity[1] >= 0.0f) {
					motion.velocity[1] = 0.0f;
					motion.position[1] = motion_block.position.y - (motion_block.scale.y / 2) +  - abs(motion.scale[1] / 2) + 1;
					motion.position += block.travelled_dist;
					player.jumpable = true;
				}
			} // else if (direction == 2) { // bot collision

			// } else if (direction == 3) { // left collision

			// } else if (direction == 4) { // right collision

			// }
		}

		// add collision between blocks & bullets such that bullets should disappear when colliding with the block
		if (registry.blocks.has(entity) && registry.bullets.has(entity_other)) registry.remove_all_components_of(entity_other);

		if (registry.players.has(entity) && registry.bullets.has(entity_other))
		{	
			Player &player = registry.players.get(entity);
			if (player.side != registry.bullets.get(entity_other).side)
			{
				if (player.health != 0) {
					if (registry.stageSelection != 6) {
						player.health -= 1;
					}
			
					// hit sound
					Mix_PlayChannel(-1, hit_sound, 0);
					if (player.health <= 0)
					{
						player.health = 0;
						if (!registry.deathTimers.has(entity))
							registry.deathTimers.emplace(entity);
						// end music
						Mix_PlayChannel(-1, end_music, 0);
						Motion &motion = registry.motions.get(entity);
						motion.angle = M_PI / 2;
						motion.scale.y = motion.scale.y / 2;
						movable = false;
						
						rounds -= 1;

						if (entity != player1) 
						{
							// player 1 wins
							num_p1_wins += 1;
						}
						else 
						{
							num_p2_wins += 1;
						}
            
            if (rounds ==0) {
            	registry.winner = (num_p1_wins > num_p2_wins) ?  1 : 2;
              createBackground(renderer, 	window_width_px, window_height_px);
          
            }
						// write something to handle the events where rounds == 0 & display the victory screen, victory conditions: over 9 rounds (has to play 9 rounds),
						// the player won wins more rounds will be the victor: num_p2_wins = rounds - num_p1_wins;

					}
					registry.remove_all_components_of(entity_other);
				}
			}
		}


		if (registry.portals.has(entity) && (registry.bullets.has(entity_other) || registry.grenades.has(entity_other) || registry.players.has(entity_other)))
		{
			// updated behaviour such that bullets can be teleported too
			Portal &portal = registry.portals.get(entity);
			Motion &motion_portal1 = registry.motions.get(portal1);
			Motion &motion_bullet = registry.motions.get(entity_other);
			Motion &motion_portal2 = registry.motions.get(portal2);
			float offset;
			if (registry.bullets.has(entity_other)) offset = 35;
			else if (registry.grenades.has(entity_other)) offset = 50;
			else offset = 65;
			Mix_PlayChannel(-1, portal_sound, 0);
			// since there are just 2 portals
			if (portal.x ==  registry.portals.get(portal1).x && portal.y == registry.portals.get(portal1).y)
			{
				// teleport player to the pos of portal2
				if (motion_bullet.velocity.x >= 0) motion_bullet.position = {motion_portal2.position.x + offset, motion_portal2.position.y + (motion_bullet.position.y - motion_portal1.position.y)};
				else motion_bullet.position =  {motion_portal2.position.x - offset, motion_portal2.position.y + (motion_bullet.position.y - motion_portal1.position.y)};
			}
			else
			{
				if (motion_bullet.velocity.x >= 0) motion_bullet.position = {motion_portal1.position.x + offset, motion_portal1.position.y + (motion_bullet.position.y - motion_portal2.position.y)};
				else motion_bullet.position = {motion_portal1.position.x - offset, motion_portal1.position.y + (motion_bullet.position.y - motion_portal2.position.y)};
			}

			for(uint j = 0; j < registry.portals.size(); j++)
			{
				Entity entity_portal = registry.portals.entities[j];
				registry.lightUps.emplace_with_duplicates(entity_portal);
			}
		}

		if (registry.bullets.has(entity) && registry.bullets.has(entity_other))
		{
			if (registry.bullets.get(entity).side != registry.bullets.get(entity_other).side) {
				registry.remove_all_components_of(entity);
				registry.remove_all_components_of(entity_other);
			}
		}

		
		if (registry.players.has(entity) && registry.items.has(entity_other))
		{
			Player &player = registry.players.get(entity);
			Item item = registry.items.get(entity_other);

			// Allow the player to pick up the item
			if (player.items.size() == 3) {
				player.items.pop();
			}
			player.items.push(item);

			// Remove the item from the registry
			registry.items.remove(entity_other);
			registry.remove_all_components_of(entity_other);

			// If we're in Stage 4, update the ItemSpawnInfo
			if (registry.stageSelection == 6) {
				for (size_t i = 0; i < itemSpawnInfos.size(); ++i) {
					if (itemSpawnInfos[i].entity == entity_other) {
						itemSpawnInfos[i].entity = Entity(); // Invalidate the entity
						itemSpawnInfos[i].respawnTimer = ITEM_RESPAWN_DELAY_MS; // Set the respawn timer
						break;
					}
				}
			} else {
				// For other stages, you might have different logic
				if (next_item_spawn < 5000.0f) {
					next_item_spawn = 5000.0f;
				}
			}
		}

		if ((registry.players.has(entity) || registry.blocks.has(entity)) && registry.grenades.has(entity_other))
        {
            if ((registry.players.has(entity) && registry.players.get(entity).side != registry.grenades.get(entity_other).side) || registry.blocks.has(entity)) {
                Motion& motion = registry.motions.get(entity_other);
                createExplosion(motion.position);
				Mix_PlayChannel(-1, explosion_sound, 0);
                registry.remove_all_components_of(entity_other);
            } 
        }

        if (registry.players.has(entity) && registry.explosions.has(entity_other))
        {	
            Explosion& explosion = registry.explosions.get(entity_other);
			Player& player = registry.players.get(entity);
			int side = player.side;
			bool player_damagable;
			if (side == 1) player_damagable = explosion.damagable1;
			else player_damagable = explosion.damagable2;

            if (player_damagable) {
                if (registry.stageSelection != 6) {
					 player.health -= 3;
				}

				if (player.health <= 0)
                {
          
// 					registry.winner = player.side == 1 ? 2 : 1;
// 					createBackground(renderer, 	window_width_px, window_height_px);

					player.health = 0;
                    if (!registry.deathTimers.has(entity)) registry.deathTimers.emplace(entity);
                    // end music
                    Mix_PlayChannel(-1, end_music, 0);
                    Motion &motion = registry.motions.get(entity);
                    motion.angle = M_PI / 2;
                    motion.scale.y = motion.scale.y / 2;
                    movable = false;

                }
				if (side == 1) explosion.damagable1 = false;
				else explosion.damagable2 = false;
            }
        }

        if (registry.players.has(entity) && registry.lasers2.has(entity_other))
        {
            Laser2& laser2 = registry.lasers2.get(entity_other);
            if (registry.players.get(entity).side != registry.lasers2.get(entity_other).side && laser2.damagable) {
                Player& player = registry.players.get(entity);
                if (registry.stageSelection != 6) {
					 player.health -= 3;
				}
		
				if (player.health <= 0)
                {
// 					registry.winner = player.side == 1 ? 2 : 1;
// 					createBackground(renderer, 	window_width_px, window_height_px);

					player.health = 0;
                    if (!registry.deathTimers.has(entity)) registry.deathTimers.emplace(entity);
                    // end music
                    Mix_PlayChannel(-1, end_music, 0);
                    Motion &motion = registry.motions.get(entity);
                    motion.angle = M_PI / 2;
                    motion.scale.y = motion.scale.y / 2;
                    movable = false;
                }
				
                laser2.damagable = false;
            }
        }
	}	
	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const
{
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod)
{
	
	
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		registry.stageSelection = 0;
		registry.winner = 0;
		registry.stages.clear();
		rounds = 9;
		ScreenState &screen = registry.screenStates.components[0];
		screen.darken_screen_factor = 0;
		num_p1_wins = 0;
		num_p2_wins = 0;
		restart_game();
	}

	if (registry.intro) {
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) registry.intro = false;
		restart_game();
	}

	if (!registry.intro && registry.stageSelection) {

		Motion& motion1 = registry.motions.get(player1);
		Motion& motion2 = registry.motions.get(player2);

		Gravity& gravity1 = registry.gravities.get(player1);
		Gravity& gravity2 = registry.gravities.get(player2);
		Player& p1 = registry.players.get(player1);
		Player& p2 = registry.players.get(player2);

		if (!movable) {
			player1_shooting = 0;
			player2_shooting = 0;
			gravity1.g[0] = 0.f;
			gravity2.g[0] = 0.f;
			p1.is_moving = false;
			p2.is_moving = false;
			return;
		}

		if (key == GLFW_KEY_RIGHT_SHIFT) {
			if (action == GLFW_PRESS && player2_item) {
				player2_item = false;
				if (!p2.items.empty()) {
					Item item = p2.items.front();
					p2.items.pop();
					if (item.id == 0) {
						p2.health += 3;
						Mix_PlayChannel(-1, healthpickup_sound, 0);
					} else if (item.id == 1) {
						createGrenade(renderer, motion2.position, p2.direction, p2.side);
					} else {
						int dir = 0;
						if (p2.direction == 0) dir = -1;
						else dir = 1;
						createLaserBeam2(motion2.position + vec2({abs(motion2.scale.x / 2) * dir, 0.f}), p2.direction, p2.side);
						Mix_PlayChannel(-1, laser2_sound, 0);
					}
				}
			} else if (action == GLFW_RELEASE) {
				player2_item = true;
			}
		}

		if (key == GLFW_KEY_3) {
			if (action == GLFW_PRESS && player1_item) {
				player1_item = false;
				if (!p1.items.empty()) {
					Item item = p1.items.front();
					p1.items.pop();
					if (item.id == 0) {
						p1.health += 3;
						
						Mix_PlayChannel(-1, healthpickup_sound, 0);
					} else if (item.id == 1) {
						createGrenade(renderer, motion1.position, p1.direction, p1.side);
					} else {
						int dir = 0;
						if (p1.direction == 0) dir = -1;
						else dir = 1;
						createLaserBeam2(motion1.position + vec2({abs(motion1.scale.x / 2) * dir, 0.f}), p1.direction, p1.side);
						Mix_PlayChannel(-1, laser2_sound, 0);
					}
				}
			} else if (action == GLFW_RELEASE) {
				player1_item = true;
			}
		}
		
		if (key == GLFW_KEY_H) {
			if (action == GLFW_PRESS) {	
				helpPanel = createHelpPanel(renderer, window_width_px, window_height_px);
				
				// Add help text
				std::string instructions = 
					"Controls:\n"
					"Player 1 (Blue):\n"
					"WASD - Movement\n"
					"Q - Shoot\n\n"
					"Player 2 (Red):\n"
					"Arrow Keys - Movement\n"
					"/ - Shoot\n\n"
					"R - Restart Game";

				
			} else if (action == GLFW_RELEASE) {
				registry.remove_all_components_of(helpPanel);
				registry.remove_all_components_of(helpText);
			}
		}

		if (key == GLFW_KEY_A) {
		if (action == GLFW_PRESS) {
				gravity1.g[0] = -p1.lr_accel;
				p1.direction = 0; // Facing left
				player1_left_button = true;
				p1.is_moving = true;
			if (motion1.scale.x > 0) motion1.scale.x *=-1;
			} else if (action == GLFW_RELEASE) {
				if (!player1_right_button) {
					gravity1.g[0] = 0.f;
					p1.is_moving = false;
				}
			player1_left_button = false;
			}
		}

		if (key == GLFW_KEY_D) {
			if (action == GLFW_PRESS) {
				gravity1.g[0] = +p1.lr_accel;
				p1.direction = 1; // Facing right
				player1_right_button = true;
				p1.is_moving = true;
				if (motion1.scale.x < 0) motion1.scale.x *= -1;

			} else if (action == GLFW_RELEASE) {
				if (!player1_left_button) {
						gravity1.g[0] = 0.f;
						p1.is_moving = false;
				}
				player1_right_button = false;
			}
		}

		if (key == GLFW_KEY_W) {
			if (action == GLFW_PRESS && p1.jumpable == true) {
				motion1.velocity[1] += p1.jump_accel;
				p1.jumpable = false;
			}
		}

		if (key == GLFW_KEY_Q) {
			if (action == GLFW_PRESS) player1_shooting = 1;
			else if (action == GLFW_RELEASE) player1_shooting = 0;
		}

		if (key == GLFW_KEY_PERIOD) {
			if (action == GLFW_PRESS) player2_shooting = 1;
			else if (action == GLFW_RELEASE) player2_shooting = 0;
		}

		// shoot arrow for player 1
		if (key == GLFW_KEY_E)
		{
			if (action == GLFW_PRESS) player1_shooting = 2;
			else if (action == GLFW_RELEASE) player1_shooting = 0;
		}

		//shoot arrow for player 2
		if (key == GLFW_KEY_SLASH)
		{
			if (action == GLFW_PRESS) player2_shooting = 2;
			else if (action == GLFW_RELEASE) player2_shooting = 0;
		}


		if (key == GLFW_KEY_LEFT) {
			if (action == GLFW_PRESS) {
				gravity2.g[0] = -p2.lr_accel;
				p2.direction = 0; // Facing left
				if (motion2.scale.x > 0) motion2.scale.x *= -1;
				player2_left_button = true;
				p2.is_moving = true;
			} else if (action == GLFW_RELEASE) {
				if (!player2_right_button) {
					gravity2.g[0] = 0.f;
					p2.is_moving = false;
				}
				player2_left_button = false;
			}
		}

		if (key == GLFW_KEY_RIGHT) {
			if (action == GLFW_PRESS) {
				gravity2.g[0] = +p1.lr_accel;
				p2.direction = 1; // Facing right
				player2_right_button = true;
				p2.is_moving = true;
				if (motion2.scale.x < 0) motion2.scale.x = -motion2.scale.x;
			} else if (action == GLFW_RELEASE) {
			if (!player2_left_button) {
				gravity2.g[0] = 0.f;
				p2.is_moving = false;
			}

			player2_right_button = false;
			}
		}

		if (key == GLFW_KEY_UP) {
			if (action == GLFW_PRESS && p2.jumpable == true) {
				motion2.velocity[1] += p2.jump_accel;
				
				p2.jumpable = false;
			}
		}
	}

	// Debugging
	if (key == GLFW_KEY_G) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	if (key == GLFW_KEY_TAB ) {
		if (action == GLFW_RELEASE)
			showMatchRecords = false;
		else
			showMatchRecords = true;
	}

	
}

void WorldSystem::on_mouse_move(vec2 mouse_position)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2) mouse_position; // dummy to avoid compiler warning
}

void WorldSystem::updateLaserVelocity(Entity laserEntity, Motion& player1Motion, Motion& player2Motion) {
    Motion& laserMotion = registry.motions.get(laserEntity);
    vec2 player1Pos = player1Motion.position;
    vec2 player2Pos = player2Motion.position;
    float distToPlayer1 = calculateDistance(laserMotion.position, player1Pos);
    float distToPlayer2 = calculateDistance(laserMotion.position, player2Pos);
    // Choose target based on the nearest player

    vec2 targetPosition = (distToPlayer1 < distToPlayer2) ? player1Pos : player2Pos;
    vec2 direction = normalize(targetPosition - laserMotion.position);
    laserMotion.velocity = direction * 70.f;
}

float WorldSystem::calculateDistance(vec2 pos1, vec2 pos2) {
	return length(pos2 - pos1);
}

void WorldSystem::initializeLaserAI() {
    // Define the action lambdas
    auto idleAction = []() { if (!registry.lasers.entities.empty()) {
        Entity laserEntity = registry.lasers.entities.front();
        Motion& laserMotion = registry.motions.get(laserEntity);
        laserMotion.velocity = {0.0f, 0.0f};  // Stop laser
    }};
    auto trackPlayerAction = [this]() {if (!registry.lasers.entities.empty()) {
        Entity laserEntity = registry.lasers.entities.front();
        Motion& laserMotion = registry.motions.get(laserEntity);
        Motion& player1Motion = registry.motions.get(player1);
        Motion& player2Motion = registry.motions.get(player2);

        vec2 targetPosition = (calculateDistance(laserMotion.position, player1Motion.position) <
                               calculateDistance(laserMotion.position, player2Motion.position)) ?
                              player1Motion.position : player2Motion.position;

        vec2 direction = normalize(targetPosition - laserMotion.position);
        laserMotion.velocity = direction * 70.0f;
    } };
    auto attackPlayerAction = [this]() {
    if (registry.lasers.entities.empty()) return;

    Entity laserEntity = registry.lasers.entities.front();
    Motion& laserMotion = registry.motions.get(laserEntity);

    // Determine the nearest player’s position as the laser target
    Motion& player1Motion = registry.motions.get(player1);
    Motion& player2Motion = registry.motions.get(player2);
    target = (calculateDistance(laserMotion.position, player1Motion.position) <
                      calculateDistance(laserMotion.position, player2Motion.position))
                         ? player1Motion.position
                         : player2Motion.position;

    // Set the laser firing flag
    isLaserFiring = true;
};



    // Create action nodes using lambdas
    auto idleNode = new ActionNode(idleAction);
    auto trackNode = new ActionNode(trackPlayerAction);
    auto attackNode = new ActionNode(attackPlayerAction);

    // Condition lambda to check if any player is in range
    auto isPlayerInRange = [this]() -> bool {
        if (registry.lasers.entities.empty()) return false;
        Entity laserEntity = registry.lasers.entities.front();
        Motion& laserMotion = registry.motions.get(laserEntity);
        Motion& playerMotion1 = registry.motions.get(player1);
        Motion& playerMotion2 = registry.motions.get(player2);
		currentDelay = 0.0f;

        float distanceToPlayer1 = calculateDistance(laserMotion.position, playerMotion1.position);
        float distanceToPlayer2 = calculateDistance(laserMotion.position, playerMotion2.position);

        return distanceToPlayer1 <= laserRange || distanceToPlayer2 <= laserRange;
    };

    // Condition lambda to check if coolDown has completed
    auto isCoolDownComplete = [this]() -> bool {
        return laserCoolDownTimer <= 0;
    };

    // Create condition nodes
    auto inRangeNode = new ConditionNode(isPlayerInRange, attackNode, trackNode);
    rootNode = new ConditionNode(isCoolDownComplete, inRangeNode, idleNode);
}

bool WorldSystem::isPlayerInRange() {
	if (registry.lasers.entities.empty()) return false;
	Entity laserEntity = registry.lasers.entities.front();
	Motion& laserMotion = registry.motions.get(laserEntity);
	Motion& playerMotion1 = registry.motions.get(player1);
	Motion& playerMotion2 = registry.motions.get(player2);

	float distanceToPlayer1 = calculateDistance(laserMotion.position, playerMotion1.position);
	float distanceToPlayer2 = calculateDistance(laserMotion.position, playerMotion2.position);
	currentDelay = 0.0f;

	return distanceToPlayer1 <= laserRange || distanceToPlayer2 <= laserRange;
}

// Laser collision handling function
void WorldSystem::handleLaserCollisions() {
    for (Entity laserEntity : registry.lasers.entities) {
        Motion& laserMotion = registry.motions.get(laserEntity);

        // Check collision with each player
        for (Entity playerEntity : registry.players.entities) {
            Player& player = registry.players.get(playerEntity);
            Motion& playerMotion = registry.motions.get(playerEntity);

            // Check if player is in laser path using a helper function
            if (isLaserInRange(laserMotion.position, playerMotion.position) & movable) {
                // Reduce player health by 1 on laser hit
				if (player.health != 0) {
					if (registry.stageSelection != 6) {
						player.health -= 1;
					}
					Mix_PlayChannel(-1, laser_sound, 0);
					if (player.health <= 0 && !registry.deathTimers.has(playerEntity)) {
// 						registry.winner = player.side == 1 ? 2 : 1;
// 						createBackground(renderer, window_width_px, window_height_px);
						player.health = 0;
						registry.deathTimers.emplace(playerEntity);
						Mix_PlayChannel(-1, end_music, 0);
						playerMotion.angle = M_PI / 2;
						playerMotion.scale.y = playerMotion.scale.y / 2;
						movable = false;
					}
				}
            }
        }
    }
}

// Check if the player is within the laser's range
bool WorldSystem::isLaserInRange(vec2 laserPosition, vec2 playerPosition) {
    float distance = calculateDistance(laserPosition, playerPosition);
    return distance <= laserRange;
}

void WorldSystem::on_shoot() {
    if (!registry.gunTimers.has(player1) && player1_shooting) {
        Player& p1 = registry.players.get(player1);
        Motion& motion1 = registry.motions.get(player1);
        int dir;
        if (p1.direction == 0) dir = -1;
        else dir = 1;
        vec2 bullet_position = motion1.position + vec2({abs(motion1.scale.x / 2) * dir, 0.f});
        if (player1_shooting == 1 && remaining_bullet_shots_p1 >= 1 && reloading_time_p1 == 0) 
		{
            Entity bullet = createBullet(renderer, 1, bullet_position, p1.direction);
            registry.colors.insert(bullet, {0.6f, 1.0f, 0.6f});
			Mix_PlayChannel(-1, shoot_sound, 0);
			remaining_bullet_shots_p1 -= 1;
        } 
		else if (remaining_bullet_shots_p1 < 1 && player1_shooting == 1)
		{
			// time to reload
			// check for reload timer
			if (reloading_time_p1 <= 0)
			{
				reloading_time_p1 = 6.0f;
			}
			Mix_PlayChannel(-1, reload_sound,0);
		}
		else if (player1_shooting == 2 && remaining_buck_p1 >= 1 && reloading_time_p1 == 0) 
		{
			auto bullets = createBuckshot(renderer, 1, bullet_position, p1.direction);
            for (size_t i = 0; i < bullets.size(); i++)
            {
                registry.colors.insert(bullets[i], {0.6f, 1.0f, 0.6f});
            }
			Mix_PlayChannel(-1, buck_shot_sound, 0);
			remaining_buck_p1 -= 1;
		}
		else 
		{
			// reloading the buck shots
			if (reloading_time_p1 <= 0)
			{
				reloading_time_p1 = 6.0f;
			}
			Mix_PlayChannel(-1, reload_sound,0);
		}
        registry.gunTimers.emplace(player1);
    }
    if (!registry.gunTimers.has(player2) && player2_shooting) {
        Player& p2 = registry.players.get(player2);
        Motion& motion2 = registry.motions.get(player2);
        int dir;
        if (p2.direction == 0) dir = -1;
        else dir = 1;
        vec2 bullet_position = motion2.position + vec2({abs(motion2.scale.x / 2) * dir, 0.f});
        if (player2_shooting == 1 && remaining_bullet_shots_p2 >= 1 && reloading_time_p2 == 0) 
		{
            Entity bullet = createBullet(renderer, 2, bullet_position, p2.direction);
            registry.colors.insert(bullet, {1.0f, 0.84f, 0.0f});
			Mix_PlayChannel(-1, shoot_sound, 0);
			remaining_bullet_shots_p2 -= 1;
        } 
		else if (remaining_bullet_shots_p2 < 1 && player2_shooting == 1)
		{
			// time to reload
			// check for reload timer
			if (reloading_time_p2 <= 0)
			{
				reloading_time_p2 = 6.0f;
			}
			Mix_PlayChannel(-1, reload_sound,0);
		}
		else if (player2_shooting == 2 && remaining_buck_p2 >= 1 && reloading_time_p2 == 0) 
		{
			auto bullets = createBuckshot(renderer, 2, bullet_position, p2.direction);
            for (size_t i = 0; i < bullets.size(); i++)
            {
                registry.colors.insert(bullets[i], {1.0f, 0.84f, 0.0f});
            }
			Mix_PlayChannel(-1, buck_shot_sound, 0);
			remaining_buck_p2 -= 1;
		}
		else 
		{
			// reloading the buck shots
			if (reloading_time_p2 <= 0)
			{
				reloading_time_p2 = 6.0f;
			}
			Mix_PlayChannel(-1, reload_sound,0);
		}
        registry.gunTimers.emplace(player2);
    }
}


void WorldSystem::on_mouse_button(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        vec2 mouse_position = {static_cast<float>(xpos), static_cast<float>(ypos)};
        
        for (Entity entity : registry.stages.entities) {
            if (isMouseOverEntity(mouse_position, entity) && registry.stageSelection == 0) {
                handleEntityClick(entity);
                break; // Exit after the first entity is clicked
            }
        }
    }
}

// Function to check if the mouse is over the entity
bool WorldSystem::isMouseOverEntity(vec2 mouse_position, Entity entity) {
	if (registry.stageSelection != 0) return false;
    Motion& motion = registry.motions.get(entity);

    return (mouse_position.x >= motion.position.x - motion.scale.x / 2 &&
            mouse_position.x <= motion.position.x + motion.scale.x / 2 &&
            mouse_position.y >= motion.position.y - motion.scale.y / 2 &&
            mouse_position.y <= motion.position.y + motion.scale.y / 2);
}

void WorldSystem::handleEntityClick(Entity entity) {
    // Implement your logic here, e.g., selecting the entity or triggering an action
	if (registry.stages.has(entity)) {
		StageChoice& s = registry.stages.get(entity);
		registry.stageSelection = s.stage;
		Mix_PlayChannel(-1, select_music, 0);
		restart_game();
	}
    std::cout << "Entity clicked: " << entity << std::endl;
}

void WorldSystem::loadMatchRecords() {
	std::ifstream record_file("BattleRecord.txt");
    if (record_file.is_open()) {
        std::string line;
        while (std::getline(record_file, line)) {
            if (!line.empty()) {
                match_records.push_back(line);
            }
        }
        record_file.close();
    }

	// Keep only recent 10 battles
    while (match_records.size() > 10) {
        match_records.pop_front();
    }
}

void WorldSystem::recordMatchResult() {
	Player& player1Component = registry.players.get(player1);
    Player& player2Component = registry.players.get(player2);

	int player1hp = player1Component.health <= 0? 0: player1Component.health;
	int player2hp =  player2Component.health <= 0? 0: player2Component.health;
	
    std::ostringstream result;
    result << "Blue: " << 3 - player2hp << " - Red: " << 3 - player1hp;
    match_records.push_back(result.str());

    if (match_records.size() > 10) {
        match_records.pop_front();
    }

    std::ofstream record_file("BattleRecord.txt");
    if (record_file.is_open()) {
        for (const auto& record : match_records) {
            record_file << record << std::endl;
        }
        record_file.close();
    } else {
        std::cerr << "fail to BattleRecord.txt" << std::endl;
    }
}

void WorldSystem::createStage(int currentStage) {
    // Create the new stage
    const Stage& stage = stagesArray[currentStage];
    
    // Create players
	if (currentStage != 4) {
		player1 = createPlayer(renderer, 1, {200, stage.groundPositions[0].y}, 1);
		player2 = createPlayer(renderer, 2, {window_width_px - 200, stage.groundPositions[0].y}, 0);
		Motion& player1Motion = registry.motions.get(player1);
		Motion& player2Motion = registry.motions.get(player2);
		gun1 = createGun(renderer, 1, {player1Motion.position.x - 200, stage.groundPositions[0].y - 50});
		gun2 = createGun(renderer, 2, {player2Motion.position.x - 150, stage.groundPositions[0].y - 50});
	} else {
		player1 = createPlayer(renderer, 1, {200, stage.platformPositions[0].y}, 1);
		player2 = createPlayer(renderer, 2, {window_width_px - 200, stage.platformPositions[0].y}, 0);
		Motion& player1Motion = registry.motions.get(player1);
		Motion& player2Motion = registry.motions.get(player2);
		gun1 = createGun(renderer, 1, {player1Motion.position.x - 200, stage.platformPositions[0].y - 50});
		gun2 = createGun(renderer, 2, {player2Motion.position.x - 150, stage.platformPositions[0].y - 50});
	}


    // Create grounds
	for (size_t i = 0; i < stage.groundPositions.size(); i++) {
        vec2 pos = stage.groundPositions[i];
        vec2 size = stage.groundSizes[i];
        createBlock1(renderer, pos.x, pos.y, size.x, size.y);
    }
    
    // Create platforms
    for (size_t i = 0; i < stage.platformPositions.size(); i++) {
        vec2 pos = stage.platformPositions[i];
        vec2 size = stage.platformSizes[i];
        createBlock2(renderer, pos, size.x, size.y, stage.moving[i]);
    }

	vec2 portal1Pos;
    vec2 portal2Pos;

	if (currentStage == 1) {
		portal1Pos = stage.platformPositions[0];
		portal2Pos = stage.platformPositions[2];
	} else if (currentStage == 3) {
		portal1Pos = {stage.platformPositions[1].x, stage.groundPositions[1].y};
		portal2Pos = stage.platformPositions[1];
	} else if (currentStage == 4) {
		
	} else if (currentStage == 5) {
		portal1Pos = vec2(50.0f, stage.groundPositions[0].y); // Left side
        portal2Pos = vec2(window_width_px - 50.0f, stage.groundPositions[0].y); // Right side
	} else {
		// Generate portal positions based on random numbers
    	std::random_device rd;
    	std::mt19937 generator(rd());
    	std::uniform_int_distribution<int> dist(0, stage.platformPositions.size() - 1);

    	int rand1 = dist(generator);
    	int rand2 = dist(generator);

    	// Avoid placing both portals on the same platform
    	while (rand1 == rand2) {
        	rand2 = dist(generator);
    	}

    	// Use random platform positions for portals
    	portal1Pos = stage.platformPositions[rand1];
    	portal2Pos = stage.platformPositions[rand2];
	} 

	if (currentStage != 4) {
		// Create portal 1
    	portal1 = createPortal(renderer, {portal1Pos.x + 25, portal1Pos.y - 10}, 50, 100);
    	registry.colors.insert(portal1, {0.0f, 1.0f, 0.0f});

    	// Create portal 2
    	portal2 = createPortal(renderer, {portal2Pos.x + 25, portal2Pos.y - 10}, 50, 100);
    	registry.colors.insert(portal2, {0.0f, 1.0f, 0.0f});
	}

	// Initialize item spawn infos
    itemSpawnInfos.clear();

	if (registry.stageSelection == 6) {
		for (size_t i = 0; i < stage.platformPositions.size(); ++i) {
			vec2 pos = stage.platformPositions[i];
			// Place the item slightly above the platform position
			vec2 itemPos = pos + vec2(0, -50); // Adjust the offset as needed
			int itemType = i % 3; // Assign item types 0, 1, 2
			ItemSpawnInfo spawnInfo;
			spawnInfo.position = itemPos;
			spawnInfo.itemType = itemType;
			spawnInfo.respawnTimer = 0.0f; // Items are spawned immediately
			Motion item_motion;
			item_motion.position = itemPos;
			item_motion.scale = {30, 45};
			Entity itemEntity = createSpecificItem(renderer, item_motion, itemType);
			spawnInfo.entity = itemEntity;
			itemSpawnInfos.push_back(spawnInfo);
		}
	}
	// Additional stage-specific logic (e.g., lasers)

	if (rounds <= 6 || registry.stageSelection == 6)
	{
		createLaser(renderer);
	}
    initializeLaserAI();
}


