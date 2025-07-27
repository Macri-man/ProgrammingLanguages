#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Vertex + Fragment shaders
const char* vertexShaderSrc = R"(
#version 460 core
layout(location = 0) in vec2 aPos;
uniform vec2 offset;
void main() {
    gl_Position = vec4(aPos + offset, 0.0, 1.0);
}
)";

const char* fragmentShaderSrc = R"(
#version 460 core
out vec4 FragColor;
uniform float time;

void main() {
    float r = 0.5 + 0.5 * sin(time);
    float g = 0.5 + 0.5 * sin(time + 2.0);
    float b = 0.5 + 0.5 * sin(time + 4.0);
    FragColor = vec4(r, g, b, 1.0);
}
)";

// Handle input state
float xOffset = 0.0f;
float yOffset = 0.0f;
const float speed = 0.01f;

// Key input callback
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        xOffset -= speed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        xOffset += speed;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        yOffset += speed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        yOffset -= speed;
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader Compile Error:\n" << log << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Moving Square", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // Load GLAD2
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD2" << std::endl;
        return -1;
    }

    // Square vertex data
    float square[] = {
        -0.1f, -0.1f,
         0.1f, -0.1f,
         0.1f,  0.1f,
        -0.1f,  0.1f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint shader = createShaderProgram();
    GLint offsetLoc = glGetUniformLocation(shader, "offset");

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        glUniform2f(offsetLoc, xOffset, yOffset);
        float time = static_cast<float>(glfwGetTime());
        glUniform1f(glGetUniformLocation(shader, "time"), time);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}
