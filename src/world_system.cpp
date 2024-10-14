// Header
#include "world_system.hpp"
#include "common.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

// create the underwater world
WorldSystem::WorldSystem() {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	
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
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
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
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	end_music = Mix_LoadWAV(audio_path("end_music.wav").c_str());
	hit_sound = Mix_LoadWAV(audio_path("hit_sound.wav").c_str());
	shoot_sound = Mix_LoadWAV(audio_path("shoot.wav").c_str());

	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());


	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("death_sound.wav").c_str(),
			audio_path("eat_sound.wav").c_str(),
			audio_path("shoot.wav").c_str(),
			audio_path("hit_sound.wav").c_str(),
			audio_path("end_music.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	std::stringstream title_ss;
	title_ss << "Game Screen";
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	// Update player1's position and enforce boundaries
    Motion& motion1 = registry.motions.get(player1);
    if (motion1.position.x <= motion1.scale[0]/2) {
        motion1.position.x = motion1.scale[0]/2; // Stop at left boundary
    } else if (motion1.position.x + motion1.scale.x > window_width_px + motion1.scale[0]/2) {
        motion1.position.x = window_width_px + motion1.scale[0]/2 - motion1.scale.x; // Stop at right boundary
    }
    if (motion1.position.y < motion1.scale[1]/2) {
        motion1.position.y = motion1.scale[1]/2; // Stop at the top boundary
    } 

    // Update player2's position and enforce boundaries
    Motion& motion2 = registry.motions.get(player2);
    if (motion2.position.x <= motion2.scale[0]/2) {
        motion2.position.x = motion2.scale[0]/2; // Stop at left boundary
    } else if (motion2.position.x + motion2.scale.x > window_width_px + motion2.scale[0]/2) {
        motion2.position.x =  window_width_px + motion2.scale[0]/2 - motion2.scale.x; // Stop at right boundary
    }
    if (motion2.position.y < motion2.scale[1]/2) {
        motion2.position.y = motion2.scale[1]/2; // Stop at the top boundary
    }

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size()-1; i>=0; --i) {
	    Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}
	}

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState &screen = registry.screenStates.components[0];

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
		    min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			int side = registry.players.get(entity).side;
			if (side == 2) std::cout << "Red Player Wins" << std::endl; // Red Wins 
			else std::cout << "Blue Player Wins" << std::endl; // Blue Wins 

			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
            restart_game();
			return true;
		}
	}
	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	for (Entity entity : registry.gunTimers.entities) {
		GunTimer& counter = registry.gunTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			registry.gunTimers.remove(entity);
		}
	}

	// if (registry.deathTimers.size() > 0) {
	// 	restart_game();
	// }

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
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
	player1 = createPlayer(renderer, 1, {window_width_px - 100, window_height_px - 100}, 1);
	registry.colors.insert(player1, {1.0f, 0.1f, 0.1f});

	player2 = createPlayer(renderer, 2, {window_width_px - 200, window_height_px - 200}, 0);
	registry.colors.insert(player2, {0.1f, 0.1f, 1.0f});

	ground = createBlock1(renderer, 0, window_height_px - 50, window_width_px, 50);
	registry.colors.insert(ground, {0.0f, 0.0f, 0.0f});

	platform1 = createBlock2(renderer, {window_width_px/4, window_height_px - 220}, 250, 20);
	registry.colors.insert(platform1, {0.0f, 0.0f, 0.0f});
	platform2 = createBlock2(renderer, {3 * window_width_px/4, window_height_px - 220}, 250, 20);
	registry.colors.insert(platform2, {0.0f, 0.0f, 0.0f});
	platform3 = createBlock2(renderer, {window_width_px/2, window_height_px - 390}, 250, 20);
	registry.colors.insert(platform3, {0.0f, 0.0f, 0.0f});
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;
		int direction = collisionsRegistry.components[i].direction;

		if (registry.players.has(entity) && registry.blocks.has(entity_other)) {
			if (direction == 1) { // top collision
				Motion& motion = registry.motions.get(entity);
				Block& block = registry.blocks.get(entity_other);
				Player& player = registry.players.get(entity);
				motion.velocity[1] = 0.0f;
				motion.position[1] = block.y - (motion.scale[1] / 2);
				player.jumpable = true;
			} else if (direction == 2) { // bot collision
				Motion& motion = registry.motions.get(entity);
				Block& block = registry.blocks.get(entity_other);
				motion.velocity[1] = 0.0f;
				motion.position[1] = block.y + block.height + (motion.scale[1] / 2);
			} else if (direction == 3) { // left collision
				Motion& motion = registry.motions.get(entity);
				Block& block = registry.blocks.get(entity_other);
				motion.velocity[0] = 0.0f;
				motion.position[0] = block.x - (motion.scale[1] / 2);
			} else if (direction == 4) { // right collision
				Motion& motion = registry.motions.get(entity);
				Block& block = registry.blocks.get(entity_other);
				motion.velocity[0] = 0.0f;
				motion.position[0] = block.x + block.width + (motion.scale[1] / 2);
			}
		}

		if (registry.players.has(entity) && registry.bullets.has(entity_other)) {
			Player& player = registry.players.get(entity);
			if (player.side != registry.bullets.get(entity_other).side) {
				player.health -= 1;

				// hit sound
				Mix_PlayChannel(-1, hit_sound, 0);

				if (player.health <= 0) {
					if (!registry.deathTimers.has(entity)) registry.deathTimers.emplace(entity);

					// end music
					Mix_PlayChannel(-1, end_music, 0);
				}
				registry.remove_all_components_of(entity_other);
			}
		}

		if (registry.bullets.has(entity) && registry.bullets.has(entity_other)) {
			registry.remove_all_components_of(entity);
			registry.remove_all_components_of(entity_other);
			
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

        restart_game();
	}

	Motion& motion1 = registry.motions.get(player1);
	Motion& motion2 = registry.motions.get(player2);
	Gravity& gravity1 = registry.gravities.get(player1);
	Gravity& gravity2 = registry.gravities.get(player2);
	Player& p1 = registry.players.get(player1);
	Player& p2 = registry.players.get(player2);
	if (key == GLFW_KEY_A) {
    	if (action == GLFW_PRESS) {
        	gravity1.g[0] = -700.f;
        	p1.direction = 0; // Facing left
			player1_left_button = true;
    	} else if (action == GLFW_RELEASE) {
        	if (!player1_right_button) {
				gravity1.g[0] = 0.f;
			}
			player1_left_button = false;
    	}
	}

	if (key == GLFW_KEY_D) {
    	if (action == GLFW_PRESS) {
        	gravity1.g[0] = +700.f;
        	p1.direction = 1; // Facing right
			player1_right_button = true;
    	} else if (action == GLFW_RELEASE) {
        	if (!player1_left_button) {
				gravity1.g[0] = 0.f;
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
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.gunTimers.has(player1)) {
			int dir;
			if (p1.direction == 0) dir = -1; // left
			else dir = 1;
			vec2 bullet_position = registry.motions.get(player1).position + vec2({(registry.motions.get(player1).scale.x / 2) * dir, 0.f});
			Entity bullet = createBullet(renderer, 1, bullet_position, p1.direction);
			registry.colors.insert(bullet, {1.0f, 0.84f, 0.0f});
			registry.gunTimers.emplace(player1);

			// shoot sound
			Mix_PlayChannel(-1, shoot_sound, 0);
		}
	}

	if (key == GLFW_KEY_LEFT) {
    	if (action == GLFW_PRESS) {
        	gravity2.g[0] = -700.f;
        	p2.direction = 0; // Facing left
			player2_left_button = true;
    	} else if (action == GLFW_RELEASE) {
			if (!player2_right_button) {
				gravity2.g[0] = 0.f;
			}
			player2_left_button = false;
    	}
	}
	if (key == GLFW_KEY_RIGHT) {
    	if (action == GLFW_PRESS) {
        	gravity2.g[0] = +700.f;
        	p2.direction = 1; // Facing right
			player2_right_button = true;
    	} else if (action == GLFW_RELEASE) {
			if (!player2_left_button) {
				gravity2.g[0] = 0.f;
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
	if (key == GLFW_KEY_SLASH) {
		if ((action == GLFW_PRESS || action == GLFW_REPEAT) && !registry.gunTimers.has(player2)) {
			int dir;
			if (p2.direction == 0) dir = -1;
			else dir = 1;
			vec2 bullet_position = registry.motions.get(player2).position + vec2({(registry.motions.get(player2).scale.x / 2) * dir, 0.f});
			Entity bullet = createBullet(renderer, 2, bullet_position, p2.direction);
			registry.colors.insert(bullet, {0.6f, 1.0f, 0.6f});
			registry.gunTimers.emplace(player2);

			// shoot sound
			Mix_PlayChannel(-1, shoot_sound, 0);
		} 
	}

	// Debugging
	if (key == GLFW_KEY_G ) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A1: HANDLE SALMON ROTATION HERE
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(vec2)mouse_position; // dummy to avoid compiler warning
}
