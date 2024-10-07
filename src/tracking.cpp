
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <iostream>
#include <cstdlib>    // For rand() and srand()
#include <ctime>      // For time()

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 position;
    uniform vec4 color;
    out vec4 vertexColor;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        gl_PointSize = 100.0;   // Double the point size for better visibility
        vertexColor = color;
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    in vec4 vertexColor;
    out vec4 FragColor;
    void main() {
        FragColor = vertexColor;
    }
)glsl";

float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// Function to move point B towards point A(core function)
void moveTowards(glm::vec2& b, const glm::vec2& a, float speed) {
    glm::vec2 direction = a - b;
    float distance = glm::length(direction);

    if (distance > 0.01f) {
        b += glm::normalize(direction) * speed;
    }
}

GLuint createShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Clean up shaders as we no longer need them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Moving Point A and Tracking Point B", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (gl3w_init()) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return -1;
    }

    // Set the viewport
    glViewport(0, 0, 800, 600);
    srand(static_cast<unsigned int>(time(0)));

    glm::vec2 a = { randomFloat(-0.9f, 0.9f), randomFloat(-0.9f, 0.9f) };
    glm::vec2 b = { randomFloat(-0.9f, 0.9f), randomFloat(-0.9f, 0.9f) };

    float speed = 0.01f / 3.0f;

    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Move point A randomly
        a.x += randomFloat(-0.02f, 0.02f);   // Increased movement range for visibility
        a.y += randomFloat(-0.02f, 0.02f);

        if (a.x > 1.0f) a.x = 1.0f;
        if (a.x < -1.0f) a.x = -1.0f;
        if (a.y > 1.0f) a.y = 1.0f;
        if (a.y < -1.0f) a.y = -1.0f;

        moveTowards(b, a, speed);

        glm::vec2 points[] = { a, b };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);

        glUniform4f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, 1);

        glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 1, 1);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
