#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

// Vertex and Fragment Shader source
const char* vertexShaderSrc = R"(
#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

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


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok; glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLchar log[512]; glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader error:\n" << log << '\n';
    }
    return shader;
}

unsigned int createShaderProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}


constexpr float PI = 3.14159265358979323846f;

void make2DShape(std::vector<float>& vertices, int sides, float startAngle = 0.0f, float radius = 1.0f) {
    if (sides < 3) return;
    vertices.clear();

    // Center of shape
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    float angleStep = 2 * PI / sides;

    for (int i = 0; i <= sides; ++i) {
        float angle = startAngle + i * angleStep;
        float x = std::cos(angle) * radius;
        float y = std::sin(angle) * radius;
        vertices.push_back(x);
        vertices.push_back(y);
    }
}

void make3DShape(std::vector<float>& vertices, int sides, float startAngle = 0.0f, float radius = 1.0f) {
    if (sides < 3) return;
    vertices.clear();

    // Center
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
        vertices.push_back(0.0f); // Flat in Z
    }
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Multiple Shapes", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    GLuint shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");

    // Triangle data
    float triangleVertices[] = {
        -0.8f, -0.5f, 0.0f,
        -0.4f,  0.5f, 0.0f,
         0.0f, -0.5f, 0.0f
    };

    // Square data
    float squareVertices[] = {
         0.4f, -0.5f, 0.0f,
         0.8f, -0.5f, 0.0f,
         0.8f,  0.5f, 0.0f,
         0.4f,  0.5f, 0.0f
    };

    unsigned int squareIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Triangle VAO/VBO
    GLuint triVAO, triVBO;
    glGenVertexArrays(1, &triVAO);
    glGenBuffers(1, &triVBO);

    glBindVertexArray(triVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Square VAO/VBO/EBO
    GLuint sqVAO, sqVBO, sqEBO;
    glGenVertexArrays(1, &sqVAO);
    glGenBuffers(1, &sqVBO);
    glGenBuffers(1, &sqEBO);

    glBindVertexArray(sqVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sqVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sqEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Draw triangle
        glBindVertexArray(triVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Draw square
        glBindVertexArray(sqVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &triVAO);
    glDeleteBuffers(1, &triVBO);
    glDeleteVertexArrays(1, &sqVAO);
    glDeleteBuffers(1, &sqVBO);
    glDeleteBuffers(1, &sqEBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
