#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>

constexpr float PI = 3.14159265358979323846f;

// Shader sources
const char* vertexShaderSrc = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 model, view, projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSrc = R"(
#version 460 core
out vec4 FragColor;
uniform vec4 shapeColor;
void main() {
    FragColor = shapeColor;
}
)";

// === Utility Functions ===
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << '\n';
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << '\n';
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// === Shape Generation ===
void make2DShape(std::vector<float>& vertices, int sides, float startAngle = 0.0f, float radius = 1.0f) {
    if (sides < 3) return;
    vertices.clear();

    // Center point
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    float angleStep = 2 * PI / sides;
    for (int i = 0; i <= sides; ++i) {
        float angle = startAngle + i * angleStep;
        float x = std::cos(angle) * radius;
        float y = std::sin(angle) * radius;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f); // Z = 0 for 2D
    }
}

// === Shape Struct ===
struct Shape {
    GLuint vao, vbo;
    int vertexCount;
    glm::vec3 position;
    glm::vec4 color;
};

// === Create VAO/VBO ===
void createVAO(GLuint& vao, GLuint& vbo, const std::vector<float>& data) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
}

int main() {
    // GLFW init
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Multiple 2D Shapes", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "GLAD failed\n";
        return -1;
    }

    GLuint program = createShaderProgram();
    glUseProgram(program);

    GLint locModel = glGetUniformLocation(program, "model");
    GLint locView = glGetUniformLocation(program, "view");
    GLint locProj = glGetUniformLocation(program, "projection");
    GLint locColor = glGetUniformLocation(program, "shapeColor");

    // === Projection/View ===
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3.0f));
    glUniformMatrix4fv(locView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(locProj, 1, GL_FALSE, glm::value_ptr(projection));

    // === Create Shapes ===
    std::vector<Shape> shapes;

    auto addShape = [&](int sides, glm::vec3 pos, glm::vec4 color, float radius = 0.4f) {
        std::vector<float> verts;
        make2DShape(verts, sides, 0.0f, radius);
        Shape shape;
        shape.position = pos;
        shape.color = color;
        shape.vertexCount = verts.size() / 3;
        createVAO(shape.vao, shape.vbo, verts);
        shapes.push_back(shape);
    };

    addShape(3, {-0.8f, 0.8f, 0.0f}, {1, 0, 0, 1});     // Triangle
    addShape(4, {0.8f, 0.8f, 0.0f}, {0, 1, 0, 1});      // Square
    addShape(5, {-0.8f, -0.8f, 0.0f}, {0, 0, 1, 1});    // Pentagon
    addShape(6, {0.8f, -0.8f, 0.0f}, {1, 1, 0, 1});     // Hexagon
    addShape(10, {0.0f, 0.0f, 0.0f}, {0.5f, 0.2f, 1, 1}); // Decagon

    // === Render Loop ===
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);

        for (const Shape& shape : shapes) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), shape.position);
            glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4fv(locColor, 1, glm::value_ptr(shape.color));
            glBindVertexArray(shape.vao);
            glDrawArrays(GL_TRIANGLE_FAN, 0, shape.vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // === Cleanup ===
    for (auto& shape : shapes) {
        glDeleteVertexArrays(1, &shape.vao);
        glDeleteBuffers(1, &shape.vbo);
    }

    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
