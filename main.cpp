
#include "tiny_ecs_registry.hpp"
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;

	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	renderer.init(window);
	world.init(&renderer);

	// variable timestep loop
	auto t = Clock::now();
	while (!world.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		world.step(elapsed_ms);
		physics.step(elapsed_ms);
		world.handle_collisions();

		renderer.draw();

		float text_height = 50.0f;

		// In both fontInit and draw
    	glm::mat4 font_trans = glm::mat4(1.0f);
    	glm::vec3 font_color = glm::vec3(1.0, 1.0, 1.0);

		// Render the game score
		std::string score_text = "FPS: " + std::to_string(world.fps);
		renderer.renderText(score_text, 10.0f, window_height_px - text_height, 0.8f, font_color, font_trans);
		// Render dynamic HP text for players
		if (registry.stageSelection != 0) {
			for (Entity player_entity : registry.players.entities) {
			Player& player = registry.players.get(player_entity);
			Motion& motion = registry.motions.get(player_entity);

			// Adjust for top-left origin of player and bottom-left origin of renderText
			float hp_x = motion.position.x - renderer.getTextWidth("HP: 3", 1.0f) / 2; // Center horizontally
			float hp_y = motion.position.y - motion.scale.y; // Adjust Y for bottom-left origin

			std::string hp_text = "HP: " + std::to_string(player.health);

			renderer.renderText(hp_text, hp_x, window_height_px - hp_y + 10.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f), glm::mat4(1.0f));
			}
		}
		
		if (world.toogle_life_timer > 0 && world.toogle_life > 0 && registry.stageSelection != 0)
        {
            for (size_t i = 0; i < registry.players.size(); i++)
            {
                auto &player = registry.players.entities[i];
                Motion &player_motion = registry.motions.get(player);

                // Prepare text
				std::string text = "health + 3";
				glm::vec3 text_color = glm::vec3(0.f, 1.0f, 0.f);
				glm::mat4 font_trans = glm::mat4(1.0f);

				// Calculate text position (above the fish)
				float scale = .5f; // Adjust as needed
				float text_x = player_motion.position.x - 10;
				float text_y = player_motion.position.y - 30.f;

				// Render the text
				renderer.renderText(text, text_x, window_height_px - text_y + 10.0f, scale, text_color, font_trans);
            }
        }

		if (world.showMatchRecords) {
			renderer.renderMatchRecords(world.match_records);
		}

		// flicker-free display with a double buffer
		glfwSwapBuffers(window);
		gl_has_errors();
	}


	return EXIT_SUCCESS;
}
