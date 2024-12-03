// internal
#include "render_system.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"


// matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#include "physics_system.hpp"

#include <chrono>
#include <thread>


void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
    transform.rotate(motion.angle);
    transform.scale(motion.scale);
	// !!! TODO A1: add rotation to the chain of transformations, mind the order
	// of transformations

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::SALMON || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();

		if (render_request.used_effect == EFFECT_ASSET_ID::SALMON)
		{
			// Light up?
			GLint light_up_uloc = glGetUniformLocation(program, "light_up");
			assert(light_up_uloc >= 0);

			// !!! TODO A1: set the light_up shader variable using glUniform1i,
			// similar to the glUniform1f call below. The 1f or 1i specified the type, here a single int.
			gl_has_errors();
		}
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(water_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}
void RenderSystem::renderText(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans)
	{
		// TODO: use program, load variables, bind to VAO, then iterate thru chars
		
		// activate the shader program
		glUseProgram(m_font_shaderProgram);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// get shader uniforms
		GLint textColor_location =
			glGetUniformLocation(m_font_shaderProgram, "textColor");
		glUniform3f(textColor_location, color.x, color.y, color.z);

		GLint transformLoc =
			glGetUniformLocation(m_font_shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		glBindVertexArray(m_font_VAO);

		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = m_ftCharacters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};

			// render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

			// update content of VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, m_font_VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// render quad
			glDrawArrays(GL_TRIANGLES, 0, 6);

			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		glBindBuffer(GL_ARRAY_BUFFER,1);
    	glBindVertexArray(1);
	}
// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(GLfloat(255 / 255), GLfloat(255 / 255), GLfloat(255 / 255), 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	// Draw all textured meshes that have a position and size component
	for (Entity entity : registry.renderRequests.entities)
	{
		if (!registry.motions.has(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		drawTexturedMesh(entity, projection_2D);
	}
	
	if (registry.intro) {
		float max_line_width = window_width_px * 0.7f; // 80% of screen width
		float scale = 1.0f;
		std::string story_text =
			"In a fantasy world, the red and blue nations are locked in a century's old rivalry. "
			"Each nation selects a champion to fight in one-on-one duels in a deadly arena. "
			"Two players will control a champion from each nation as they battle each other in the arena, for glory. "
			"To spice up the battlefield, the arena will include a variety of features such as items, "
			"a deadly laser, and portals to keep gameplay dynamic and fresh. "
			"The arena is certainly a dangerous front where steel and guns collide, and one nation will collapse.";

		std::vector<std::string> wrapped_text = wrapText(story_text, max_line_width, scale);

		float start_y = window_height_px / 2 + 100.0f;
		for (const std::string& line : wrapped_text) {
			float line_width = getTextWidth(line, scale);
			float x_position = (window_width_px - line_width) / 2;
			renderText(line, x_position, start_y, scale, {1.0f, 1.0f, 1.0f}, glm::mat4(1.0f));
			start_y -= 30.0f; // Adjust line spacing
		}

		// Title "RED VS BLUE ARENA"
		std::string title_text = "RED VS BLUE ARENA";
		float title_scale = 2.0f;

		// Calculate the width for each individual word
		float red_width = getTextWidth("RED", title_scale);
		float vs_width = getTextWidth(" VS ", title_scale);
		float blue_width = getTextWidth("BLUE", title_scale);
		float arena_width = getTextWidth(" ARENA", title_scale);

		// Total width of the title
		float total_title_width = red_width + vs_width + blue_width + arena_width;

		// Calculate starting x position to center the entire title
		float title_x = (window_width_px - total_title_width) / 2;

		// Render each part of the title
		renderText("RED", title_x, window_height_px / 2 + 200.0f, title_scale, {1.0f, 0.0f, 0.0f}, glm::mat4(1.0f));
		title_x += red_width;

		renderText(" VS ", title_x, window_height_px / 2 + 200.0f, title_scale, {1.0f, 1.0f, 1.0f}, glm::mat4(1.0f));
		title_x += vs_width;

		renderText("BLUE", title_x, window_height_px / 2 + 200.0f, title_scale, {0.0f, 0.0f, 1.0f}, glm::mat4(1.0f));
		title_x += blue_width;

		renderText(" ARENA", title_x, window_height_px / 2 + 200.0f, title_scale, {1.0f, 1.0f, 1.0f}, glm::mat4(1.0f));

		// Instructions "Press Space to start"
		std::string instruction_text = "Press Space to start";
		float instruction_scale = 1.5f;
		float instruction_width = getTextWidth(instruction_text, instruction_scale);
		float instruction_x = (window_width_px - instruction_width) / 2;
		renderText(instruction_text, instruction_x, window_height_px / 2 - 200.0f, instruction_scale, {1.0f, 1.0f, 1.0f}, glm::mat4(1.0f));
	}

	if (!registry.stageSelection && !registry.intro) {
		renderText("SELECT STAGE", window_width_px/2-150, window_height_px/2 + 120.0f, 2.0f, {1.0, 1.0, 1.0}, glm::mat4(1.0f));
	}

	// Truely render to the screen
	drawToScreen();

}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

float RenderSystem::getTextWidth(const std::string& text, float scale)
{
    float width = 0.0f;
    for (const char& c : text)
    {
        Character ch = m_ftCharacters[c];
        width += (ch.Advance >> 6) * scale;
    }
    return width;
}

std::vector<std::string> RenderSystem::wrapText(const std::string& text, float max_width, float scale) {
    std::istringstream words(text);
    std::string word;
    std::string line;
    std::vector<std::string> lines;
    float line_width = 0.0f;

    while (words >> word) {
        float word_width = getTextWidth(word + " ", scale);
        if (line_width + word_width > max_width) {
            lines.push_back(line);
            line = word + " ";
            line_width = word_width;
        } else {
            line += word + " ";
            line_width += word_width;
        }
    }

    if (!line.empty()) {
        lines.push_back(line);
    }

    return lines;
}

// render_system.cpp
void RenderSystem::renderHealthBar(vec2 position, vec2 size, vec3 color)
{
    // Use the SALMON effect for colored geometry without textures
    const GLuint used_effect_enum = (GLuint)EFFECT_ASSET_ID::SALMON;
    const GLuint program = (GLuint)effects[used_effect_enum];

    // Set shaders
    glUseProgram(program);
    gl_has_errors();

    const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::HEALTH_BAR];
    const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::HEALTH_BAR];

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl_has_errors();

    GLint in_position_loc = glGetAttribLocation(program, "in_position");
    GLint in_color_loc = glGetAttribLocation(program, "in_color");
    gl_has_errors();

    glEnableVertexAttribArray(in_position_loc);
    glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void *)0);
    gl_has_errors();

    glEnableVertexAttribArray(in_color_loc);
    glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void *)sizeof(vec3));
    gl_has_errors();

    // Transformation
    Transform transform;
    transform.translate(position);
    transform.scale(size);

    // Set color uniform
    GLint color_uloc = glGetUniformLocation(program, "fcolor");
    glUniform3fv(color_uloc, 1, (float *)&color);
    gl_has_errors();

    // Set transformation matrices
    GLuint transform_loc = glGetUniformLocation(program, "transform");
    mat3 projection = createProjectionMatrix();
    glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
    GLuint projection_loc = glGetUniformLocation(program, "projection");
    glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
    gl_has_errors();

    // Draw the health bar
    GLint size_of_indices = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size_of_indices);
    gl_has_errors();

    GLsizei num_indices = size_of_indices / sizeof(uint16_t);

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
    gl_has_errors();
}

