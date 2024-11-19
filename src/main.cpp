
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

		if (world.toogle_life_timer > 0 && world.toogle_life > 0)
		{
				// Display high score achievement message
				printf("!");
				std::string new_high_score_text = "New High Score Achieved!";
				float new_high_score_text_width = renderer.getTextWidth(new_high_score_text, 1.5f);
				glm::mat4 font_trans = glm::mat4(1.0f);
				renderer.renderText(new_high_score_text, (window_width_px - new_high_score_text_width) / 2.0f, window_height_px / 2.0f, 1.5f, glm::vec3(1.0f, 0.0f, 0.0f), font_trans);
		}
	}

	glfwSwapBuffers(window);

	return EXIT_SUCCESS;
}
