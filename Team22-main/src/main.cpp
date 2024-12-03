
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

		// Render health bars and HP text for players
// Render health bars and HP text for players
if (registry.stageSelection != 0) {
    for (Entity player_entity : registry.players.entities) {
        Player& player = registry.players.get(player_entity);
        Motion& motion = registry.motions.get(player_entity);

        // Health bar parameters
        const float max_health = 10.0f; // Adjust if maximum health differs
        const vec2 health_bar_size = {50.0f, 5.0f}; // Full health bar size (width, height)

        // Calculate health bar width based on current health
        float health_ratio = player.health / max_health;
        if (health_ratio < 0.0f) health_ratio = 0.0f;

        vec2 hb_size = {health_bar_size.x * health_ratio, health_bar_size.y};

        // Position health bar above the player's head
        vec2 hb_position = {
            motion.position.x - health_bar_size.x / 2, // Centered horizontally
            motion.position.y - motion.scale.y / 2 - 15.0f // Adjust vertically above the player
        };

        // Set health bar color based on player side
        vec3 hb_color = (player.side == 1) ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);

        // Render the health bar background (gray bar representing missing health)
        vec2 hb_bg_size = {health_bar_size.x, health_bar_size.y};
        vec3 hb_bg_color = vec3(0.5f, 0.5f, 0.5f); // Gray color

        renderer.renderHealthBar(hb_position, hb_bg_size, hb_bg_color);

        // Render the actual health bar (colored bar representing current health)
        renderer.renderHealthBar(hb_position, hb_size, hb_color);

        // // Render the HP text to the left of the health bar
        // std::string hp_text = "HP: " + std::to_string(player.health);
        // float hp_text_width = renderer.getTextWidth(hp_text, 1.0f);
        // vec2 hp_text_position = {
        //     hb_position.x - hp_text_width - 5.0f, // Align text left of the health bar with some spacing
        //     hb_position.y + (health_bar_size.y / 2) - 10.0f // Center the text vertically with the health bar
        // };

        // // Render the HP text with the same color as the health bar
        // renderer.renderText(hp_text, hp_text_position.x, window_height_px - hp_text_position.y, 1.0f, hb_color, glm::mat4(1.0f));


		// render the remaining bullets and buckshots on screen
		vec3 p1_color =  vec3(0.0f, 0.0f, 1.0f);
		vec3 p2_color =  vec3(1.0f, 0.0f, 0.0f);

		std::string buck_text_p1 = "buckshots: " + std::to_string(world.remaining_buck_p1);
		renderer.renderText(buck_text_p1, 10.0f, window_height_px / 2, 0.8f, p1_color, font_trans);

		std::string buck_text_p2 = "buckshots: " + std::to_string(world.remaining_buck_p2);
		renderer.renderText(buck_text_p2, window_width_px - 150.f, window_height_px / 2, 0.8f, p2_color, font_trans);

		std::string bullet_text_p1 = "bullets: " + std::to_string(world.remaining_bullet_shots_p1);
		renderer.renderText(bullet_text_p1, 10.0f, window_height_px / 2 - 20, 0.8f, p1_color, font_trans);
		std::string bullet_text_p2 = "bullets: " + std::to_string(world.remaining_bullet_shots_p2);
		renderer.renderText(bullet_text_p2, window_width_px - 150.f, window_height_px / 2 - 20, 0.8f, p2_color, font_trans);


		// render rounds and each player wins
		vec3 round_text_color =  vec3(1.0f, 1.0f, 1.0f);
		vec3 round_header_color = vec3(1.0, 1.0, 0.0);



		std::string player_win_text = std::to_string(world.num_p1_wins) + " : " + std::to_string(world.num_p2_wins);
		std::string round_text = std::to_string(world.rounds);

		std::string round_header = "Round";
		renderer.renderText(round_header, window_width_px / 2, window_height_px - text_height, 1.2f, round_header_color, font_trans);
		renderer.renderText(round_text, window_width_px / 2 + 35.f, window_height_px - text_height - 25, 1.0f, round_text_color, font_trans);

		renderer.renderText(std::to_string(world.num_p1_wins), window_width_px / 2 - 25, window_height_px - text_height - 50, 1.0f, p1_color, font_trans);
		renderer.renderText(":", window_width_px / 2 + 35.f, window_height_px - text_height - 50, 1.0f, round_text_color, font_trans);
		renderer.renderText(std::to_string(world.num_p2_wins), window_width_px / 2 + 95.f, window_height_px - text_height - 50, 1.0f, p2_color, font_trans);

    }
}


		// flicker-free display with a double buffer
		glfwSwapBuffers(window);
		gl_has_errors();
	}


	return EXIT_SUCCESS;
}
