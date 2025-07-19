#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// Vertex shader
const char* vertexShaderSource = R"(
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in float aFaceID;

out vec2 TexCoord;
flat out int FaceID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoord = aUV;
    FaceID = int(aFaceID);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader
const char* fragmentShaderSource = R"(
#version 460 core

in vec2 TexCoord;
flat in int FaceID;

out vec4 FragColor;

uniform float time;

// Tie-dye pattern function with some radial and sinusoidal elements
vec3 tieDyePattern(vec2 uv, float t) {
    vec2 center = uv - 0.5;
    float radius = length(center);
    float angle = atan(center.y, center.x);

    // Base swirling stripes
    float swirl = 0.5 + 0.5 * sin(15.0 * angle + 20.0 * radius - t * 4.0);

    // Radial bands
    float bands = 0.5 + 0.5 * sin(30.0 * radius - t * 6.0);

    // Combine for a colorful effect
    float r = swirl * bands;
    float g = 0.5 + 0.5 * sin(10.0 * angle - t * 3.0);
    float b = 0.5 + 0.5 * cos(25.0 * radius + t * 5.0);

    return vec3(r, g, b);
}

void main() {
    // Base colors for each face
    vec3 baseColors[6] = vec3[](
        vec3(1.0, 0.3, 0.3),  // Red-ish
        vec3(0.3, 1.0, 0.3),  // Green-ish
        vec3(0.3, 0.3, 1.0),  // Blue-ish
        vec3(1.0, 1.0, 0.3),  // Yellow-ish
        vec3(1.0, 0.3, 1.0),  // Magenta-ish
        vec3(0.3, 1.0, 1.0)   // Cyan-ish
    );

    float faceTime = time + float(FaceID) * 2.5;

    vec3 patternColor = tieDyePattern(TexCoord, faceTime);

    vec3 finalColor = patternColor * baseColors[FaceID];

    FragColor = vec4(finalColor, 1.0);
}

)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
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
        std::cerr << "Shader link error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Tie-Dye Cube with Face IDs", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Vertex format: position(3), uv(2), faceID(1)
    float vertices[] = {
        // Back face (faceID = 0)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,

        // Front face (faceID = 1)
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,

        // Left face (faceID = 2)
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  2.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  2.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  2.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  2.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  2.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  2.0f,

        // Right face (faceID = 3)
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  3.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  3.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  3.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  3.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  3.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  3.0f,

        // Bottom face (faceID = 4)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  4.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  4.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  4.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  4.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  4.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  4.0f,

        // Top face (faceID = 5)
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  5.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  5.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  5.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  5.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  5.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,  5.0f,
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position (location 0) - 3 floats
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // uv (location 1) - 2 floats
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // faceID (location 2) - 1 float (will be cast to int in shader)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    GLuint shaderProgram = createShaderProgram();

    while (!glfwWindowShouldClose(window)) {
        float time = glfwGetTime();

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * 0.5f, glm::vec3(1, 1, 0));
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
