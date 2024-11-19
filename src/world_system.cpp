// Header
#include "world_system.hpp"
#include "GLFW/glfw3.h"
#include "common.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

#include "animation_system.hpp"

#include "tiny_ecs_registry.hpp"
#include "decisionTree.hpp"

// for portal randomization
#include <random>

using namespace std;


// create the underwater world
WorldSystem::WorldSystem()
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem()
{

	// destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
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

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	end_music = Mix_LoadWAV(audio_path("end_music.wav").c_str());
	hit_sound = Mix_LoadWAV(audio_path("hit_sound.wav").c_str());
	shoot_sound = Mix_LoadWAV(audio_path("shoot.wav").c_str());
	laser_sound = Mix_LoadWAV(audio_path("laser.wav").c_str());
	portal_sound = Mix_LoadWAV(audio_path("portal.wav").c_str());
	buck_shot_sound = Mix_LoadWAV(audio_path("buck_shot.wav").c_str());

	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr)
	{
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
				audio_path("music.wav").c_str(),
				audio_path("death_sound.wav").c_str(),
				audio_path("eat_sound.wav").c_str(),
				audio_path("shoot.wav").c_str(),
				audio_path("hit_sound.wav").c_str(),
				audio_path("end_music.wav").c_str(),
				audio_path("portal.wav").c_str(),
				audio_path("buck_shot.wav").c_str(),
				audio_path("laser.wav").c_str());
		return nullptr;
	}

	// Adjust the volume for the background music
	Mix_VolumeMusic(2);

	// Adjust the volume for the sound effects
	Mix_VolumeChunk(hit_sound, 6);
	Mix_VolumeChunk(shoot_sound, 6);
	Mix_VolumeChunk(end_music, 10);

	glfwSetMouseButtonCallback(window, [](GLFWwindow* wnd, int button, int action, int mods) {
		((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(button, action, mods);
	});

	return window;
}

void WorldSystem::init(RenderSystem *renderer_arg)
{
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
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

    if (total_time > 1000.0f) {
        float fps = frame_count / (total_time / 1000.0f);
		std::stringstream title_ss;
        title_ss << "Game Screen - FPS: " << static_cast<int>(fps);
        glfwSetWindowTitle(window, title_ss.str().c_str());

        total_time = 0.0f;
        frame_count = 0;
    }

	if (!registry.intro && registry.stageSelection) {
	
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

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


	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i)
	{
		Motion &motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f)
		{
			if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
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

			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
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
	toogle_life = 0;
	toogle_life_timer = 0;
	// Create an intro screen
	if (registry.intro) {
		// Create intro entities
		Entity introBackground = createIntro(renderer, window_width_px, window_height_px);

		// Create stage selection button entities
		// Entity stageButton1 = createBlock2(renderer, {window_width_px / 4, window_height_px / 2}, 200, 50);
		// Entity stageButton2 = createBlock2(renderer, {window_width_px / 2, window_height_px / 2}, 200, 50);
		// Entity stageButton3 = createBlock2(renderer, {3 * window_width_px / 4, window_height_px / 2}, 200, 50);

		// // Handle stage button clicks
		// if (registry.mouseButtons.has(stageButton1)) {
		// 	// Select stage 1
		// 	registry.stageSelection = false;
		// 	registry.intro = false;
		// 	// Load stage 1
		// } else if (registry.mouseButtons.has(stageButton2)) {
		// 	// Select stage 2
		// 	registry.stageSelection = false;
		// 	registry.intro = false;
		// 	// Load stage 2
		// } else if (registry.mouseButtons.has(stageButton3)) {
		// 	// Select stage 3
		// 	registry.stageSelection = false;
		// 	registry.intro = false;
		// 	// Load stage 3
		// }
	}

	// Create a Stage Selection screen when a key is pressed
	if (!registry.intro && !registry.stageSelection) {
		// Create stage selection entities
		Entity stageSelectionBackground = createBackground(renderer, window_width_px, window_height_px);

		// Create stage button entities
		Entity stageButton1 = createStageChoice(renderer, 10, window_height_px / 2, 400, 200, 1);
		Entity stageButton2 = createStageChoice(renderer, window_width_px / 2-200, window_height_px / 2, 400, 200, 2);
		Entity stageButton3 = createStageChoice(renderer, 3 * window_width_px / 4-100, window_height_px / 2, 400, 200, 3);

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

	// create a new Salmon

    background = createBackground(renderer, window_width_px, window_height_px);

	player1 = createPlayer(renderer, 1, {200, window_height_px - 50}, 1);
	Motion& player1Motion = registry.motions.get(player1);
	gun1 = createGun(renderer, 1, {player1Motion.position.x - 200, window_height_px - 100});
	
	//red player
	player2 = createPlayer(renderer, 2, {window_width_px - 200, window_height_px - 50}, 0);
	Motion& player2Motion = registry.motions.get(player2);
	gun2 = createGun(renderer, 2, {player2Motion.position.x - 150, window_height_px - 200});

	ground = createBlock1(renderer, 0, window_height_px - 50, window_width_px, 50);
	
	platform1 = createBlock2(renderer, {window_width_px/4, window_height_px - 220}, 250, 20);
	platform2 = createBlock2(renderer, {3 * window_width_px/4, window_height_px - 220}, 250, 20);
	platform3 = createBlock2(renderer, {window_width_px/2, window_height_px - 390}, 250, 20);

	//generate portal position based on rand num generated
	random_device rd;                        
    mt19937 generator(rd());                 
    uniform_int_distribution<int> dist(0, 2);

	int rand1 = dist(generator);

	int rand2 = dist(generator);

	// Avoid hash collision
	while (rand1 == rand2)
	{
		rand2 = dist(generator);
	}
	
	if (rand1 == 0)
	{
		// use platform 1 for portal 1
		portal1 = createPortal(renderer, {window_width_px/4, window_height_px - 220 - 10}, 50, 100);
	    registry.colors.insert(portal1, {1.0f, 0.5f, 0.3f});
	}
	else if (rand1 == 1)
	{
		// use platform 2 for portal 1
		portal1 = createPortal(renderer, {3 * window_width_px/4, window_height_px - 220 - 10}, 50, 100);
	    registry.colors.insert(portal1, {1.0f, 0.5f, 0.3f});
	}
	else if (rand1 == 2)
	{
		//use platform 3 for portal 1
		portal1 = createPortal(renderer, {window_width_px/2, window_height_px - 390 - 10}, 50, 100);
	    registry.colors.insert(portal1, {1.0f, 0.5f, 0.3f});
	}

	if (rand2 == 0)
	{
		// use platform 1 for portal 2
		portal2 = createPortal(renderer, {window_width_px/4, window_height_px - 220 - 10}, 50, 100);
	    registry.colors.insert(portal2, {1.0f, 0.5f, 0.3f});
	}
	else if (rand2 == 1)
	{
		// use platform 2 for portal 2
		portal2 = createPortal(renderer, {3 * window_width_px/4, window_height_px - 220 - 10}, 50, 100);
	    registry.colors.insert(portal2, {1.0f, 0.5f, 0.3f});
	}
	else if (rand2 == 2)
	{
		//use platform 3 for portal 2
		portal2 = createPortal(renderer, {window_width_px/2, window_height_px - 390 - 10}, 50, 100);
	    registry.colors.insert(portal2, {1.0f, 0.5f, 0.3f});
	}
	
  
  	createLaser(renderer);
    initializeLaserAI();
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

				Player& player = registry.players.get(entity);
			if (direction == 1) { // top collision
				
				motion.velocity[1] = 0.0f;
				
				motion.position[1] = block.y - abs(motion.scale[1] / 2);
				player.jumpable = true;
        
			} else if (direction == 2) { // bot collision
				
				// player.jumpable = false;
				motion.velocity[1] = 0.0f;
				motion.position[1] = block.y + abs(motion.scale[1] / 2) + block.height;
			} else if (direction == 3) { // left collision
				
				motion.velocity[0] = 0.0f;
				motion.position[0] = block.x - (abs(motion.scale[0]) / 2);
			} else if (direction == 4) { // right collision
				
				motion.velocity[0] = 0.0f;
				motion.position[0] = block.x + block.width + (abs(motion.scale[0]) / 2);
			}
		}

		// add collision between blocks & bullets such that bullets should disappear when colliding with the block
		if ( registry.blocks.has(entity) && registry.bullets.has(entity_other))
		{
			registry.remove_all_components_of(entity_other);
		}
		

		if (registry.players.has(entity) && registry.bullets.has(entity_other))
		{
			Player &player = registry.players.get(entity);
			if (player.side != registry.bullets.get(entity_other).side)
			{
				player.health -= 1;

				// hit sound
				Mix_PlayChannel(-1, hit_sound, 0);
				if (player.health <= 0)
				{
					if (!registry.deathTimers.has(entity))
						registry.deathTimers.emplace(entity);
					// end music
					Mix_PlayChannel(-1, end_music, 0);
					Motion &motion = registry.motions.get(entity);
					motion.angle = M_PI / 2;
					motion.scale.y = motion.scale.y / 2;
					movable = false;
					registry.winner = player.side == 1 ? 2 : 1;

				}
				registry.remove_all_components_of(entity_other);
			}
		}

		if (registry.players.has(entity) && registry.portals.has(entity_other))
		{
			Player &player = registry.players.get(entity);
			Portal &portal = registry.portals.get(entity_other);
			Mix_PlayChannel(-1, portal_sound, 0);
			// since there are just 2 portals
			if (portal.x ==  registry.portals.get(portal1).x && portal.y == registry.portals.get(portal1).y)
			{
				// teleport player to the pos of portal2
				Motion &motion_portal2 = registry.motions.get(portal2);
				Motion &motion_player = registry.motions.get(entity);

				if (player.direction == 1)
				{
					motion_player.position =  {motion_portal2.position.x + 65, motion_portal2.position.y};
				}

				else 
				{
					motion_player.position =  {motion_portal2.position.x - 65, motion_portal2.position.y};
				}
				
			}
			else
			{
				Motion &motion_portal1 = registry.motions.get(portal1);
				Motion &motion_player = registry.motions.get(entity);

				if (player.direction == 1)
				{
					motion_player.position =  {motion_portal1.position.x + 65, motion_portal1.position.y};
				}

				else 
				{
					motion_player.position =  {motion_portal1.position.x - 65, motion_portal1.position.y};
				}
			}
			
		}
		if (registry.portals.has(entity) && registry.bullets.has(entity_other))
		{
			// updated behaviour such that bullets can be teleported too
			Portal &portal = registry.portals.get(entity);
			Motion &motion_portal1 = registry.motions.get(portal1);
			Motion &motion_bullet = registry.motions.get(entity_other);
			Motion &motion_portal2 = registry.motions.get(portal2);

			// since there are just 2 portals
			if (portal.x ==  registry.portals.get(portal1).x && portal.y == registry.portals.get(portal1).y)
			{
				// teleport player to the pos of portal2

				

				if (motion_bullet.velocity.x >= 0)
				{
					motion_bullet.position =  {motion_portal2.position.x + 65, motion_portal2.position.y + (motion_bullet.position.y - motion_portal1.position.y)};
				}

				else 
				{
					motion_bullet.position =  {motion_portal2.position.x - 65, motion_portal2.position.y + (motion_bullet.position.y - motion_portal1.position.y)};
				}
				
			}
			else
			{

				if (motion_bullet.velocity.x >= 0)
				{
					motion_bullet.position =  {motion_portal1.position.x + 65, motion_portal1.position.y + (motion_bullet.position.y - motion_portal2.position.y)};
				}

				else 
				{
					motion_bullet.position =  {motion_portal1.position.x - 65, motion_portal1.position.y + (motion_bullet.position.y - motion_portal2.position.y)};
				}
			}

			

		}

		if (registry.bullets.has(entity) && registry.bullets.has(entity_other))
		{
			registry.remove_all_components_of(entity);
			registry.remove_all_components_of(entity_other);
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
		registry.stageSelection =0;
		registry.winner = 0;
		registry.stages.clear();
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
				gravity1.g[0] = -1000.f;
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
				gravity1.g[0] = +1000.f;
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
				motion1.velocity[1] += -600;
				p1.jumpable = false;
			}
		}

		

		if (key == GLFW_KEY_Q) {
			if (action == GLFW_PRESS) player1_shooting = 1;
			else if (action == GLFW_RELEASE) player1_shooting = 0;
		}

		if (key == GLFW_KEY_SLASH) {
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
		if (key == GLFW_KEY_RIGHT_SHIFT)
		{
			if (action == GLFW_PRESS) player2_shooting = 2;
			else if (action == GLFW_RELEASE) player2_shooting = 0;
		}


		if (key == GLFW_KEY_LEFT) {
			if (action == GLFW_PRESS) {
				gravity2.g[0] = -1000.f;
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

		// for toogling +3 lives

		if (key == GLFW_KEY_T)
		{
			toogle_life = 1;
			// printing the text for 3s.
			toogle_life_timer = 3000.f;

			Player &player1_e = registry.players.get(player1);
			Player &player2_e = registry.players.get(player2);

			player1_e.health += 3;
			player2_e.health += 3;
		}
		

		if (key == GLFW_KEY_RIGHT) {
			if (action == GLFW_PRESS) {
				gravity2.g[0] = +1000.f;
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
				motion2.velocity[1] += -600;
				
				p2.jumpable = false;
			}
		}
	}

	// Debugging
	if (key == GLFW_KEY_G ) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
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
    laserMotion.velocity = direction * 100.f;
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
        laserMotion.velocity = direction * 100.0f;
    } };
    auto attackPlayerAction = [this]() {
    if (registry.lasers.entities.empty()) return;

    Entity laserEntity = registry.lasers.entities.front();
    Motion& laserMotion = registry.motions.get(laserEntity);

    // Determine the nearest player’s position as the laser target
    Motion& player1Motion = registry.motions.get(player1);
    Motion& player2Motion = registry.motions.get(player2);
    vec2 targetPosition = (calculateDistance(laserMotion.position, player1Motion.position) <
                           calculateDistance(laserMotion.position, player2Motion.position)) ?
                          player1Motion.position : player2Motion.position;

    // Spawn a new laser beam entity from the top-center toward target position
    Entity laserBeam = createLaserBeam({window_width_px / 2, 0}, targetPosition);

    // Check for any players in the path and handle them
    handleLaserCollisions();

    // Reset cooldown timer after the attack
    laserCoolDownTimer = 3000;  // 3 seconds cooldown
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
            if (isLaserInRange(laserMotion.position, playerMotion.position)) {

                // Reduce player health by 1 on laser hit
                player.health -= 1;
                Mix_PlayChannel(-1, laser_sound, 0);
                if (player.health <= 0 && !registry.deathTimers.has(playerEntity)) {
                    registry.deathTimers.emplace(playerEntity);
                    Mix_PlayChannel(-1, end_music, 0);
                    playerMotion.angle = M_PI / 4;
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
        if (player1_shooting == 1) {
            Entity bullet = createBullet(renderer, 1, bullet_position, p1.direction);
            registry.colors.insert(bullet, {0.6f, 1.0f, 0.6f});
			Mix_PlayChannel(-1, shoot_sound, 0);
        } else {
            auto bullets = createBuckshot(renderer, 1, bullet_position, p1.direction);
            for (size_t i = 0; i < bullets.size(); i++)
            {
                registry.colors.insert(bullets[i], {0.6f, 1.0f, 0.6f});
            }
			Mix_PlayChannel(-1, buck_shot_sound, 0);
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
        if (player2_shooting == 1) {
            Entity bullet = createBullet(renderer, 2, bullet_position, p2.direction);
            registry.colors.insert(bullet, {1.0f, 0.84f, 0.0f});
			Mix_PlayChannel(-1, shoot_sound, 0);
        } else {
            auto bullets = createBuckshot(renderer, 2, bullet_position, p2.direction);
            for (size_t i = 0; i < bullets.size(); i++)
            {
                registry.colors.insert(bullets[i], {1.0f, 0.84f, 0.0f});
            }
			Mix_PlayChannel(-1, buck_shot_sound, 0);
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
            if (isMouseOverEntity(mouse_position, entity)) {
                handleEntityClick(entity);
                break; // Exit after the first entity is clicked
            }
        }
    }
}

// Function to check if the mouse is over the entity
bool WorldSystem::isMouseOverEntity(vec2 mouse_position, Entity entity) {
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
		restart_game();
	}
    std::cout << "Entity clicked: " << entity << std::endl;
}