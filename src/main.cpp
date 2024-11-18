
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
		std::string score_text = "Score: " + std::to_string(world.fps);
		renderer.renderText(score_text, 10.0f, window_height_px - text_height, 0.8f, font_color, font_trans);

		
		// flicker-free display with a double buffer
		glfwSwapBuffers(window);
		gl_has_errors();
	}

	return EXIT_SUCCESS;
}
