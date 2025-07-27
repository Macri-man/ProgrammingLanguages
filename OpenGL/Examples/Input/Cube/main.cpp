#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// Vertex and fragment shaders (fixed flat qualifier as before)
const char* vertexShaderSource = R"glsl(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aFaceID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

flat out int faceID;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    faceID = int(aFaceID);
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 460 core
flat in int faceID;
out vec4 FragColor;

uniform float time;

vec3 faceColor(int id, float t) {
    float r = 0.5 + 0.5 * sin(t + id * 2.5);
    float g = 0.5 + 0.5 * sin(t + id * 5.0 + 1.0);
    float b = 0.5 + 0.5 * sin(t + id * 7.5 + 2.0);
    return vec3(r, g, b);
}

void main() {
    FragColor = vec4(faceColor(faceID, time), 1.0);
}
)glsl";

float vertices[] = {
    // Positions         // FaceID
    -0.5f, -0.5f,  0.5f, 0,
     0.5f, -0.5f,  0.5f, 0,
     0.5f,  0.5f,  0.5f, 0,
     0.5f,  0.5f,  0.5f, 0,
    -0.5f,  0.5f,  0.5f, 0,
    -0.5f, -0.5f,  0.5f, 0,

    -0.5f, -0.5f, -0.5f, 1,
     0.5f, -0.5f, -0.5f, 1,
     0.5f,  0.5f, -0.5f, 1,
     0.5f,  0.5f, -0.5f, 1,
    -0.5f,  0.5f, -0.5f, 1,
    -0.5f, -0.5f, -0.5f, 1,

    -0.5f,  0.5f,  0.5f, 2,
    -0.5f,  0.5f, -0.5f, 2,
    -0.5f, -0.5f, -0.5f, 2,
    -0.5f, -0.5f, -0.5f, 2,
    -0.5f, -0.5f,  0.5f, 2,
    -0.5f,  0.5f,  0.5f, 2,

     0.5f,  0.5f,  0.5f, 3,
     0.5f,  0.5f, -0.5f, 3,
     0.5f, -0.5f, -0.5f, 3,
     0.5f, -0.5f, -0.5f, 3,
     0.5f, -0.5f,  0.5f, 3,
     0.5f,  0.5f,  0.5f, 3,

    -0.5f,  0.5f, -0.5f, 4,
     0.5f,  0.5f, -0.5f, 4,
     0.5f,  0.5f,  0.5f, 4,
     0.5f,  0.5f,  0.5f, 4,
    -0.5f,  0.5f,  0.5f, 4,
    -0.5f,  0.5f, -0.5f, 4,

    -0.5f, -0.5f, -0.5f, 5,
     0.5f, -0.5f, -0.5f, 5,
     0.5f, -0.5f,  0.5f, 5,
     0.5f, -0.5f,  0.5f, 5,
    -0.5f, -0.5f,  0.5f, 5,
    -0.5f, -0.5f, -0.5f, 5,
};

float moveSpeed = 2.5f;
float rotSpeed = 2.0f;

glm::vec3 cubePos(0.0f);
glm::vec3 cubeRot(0.0f);

void processInput(GLFWwindow* window, float deltaTime) {
    float moveStep = moveSpeed * deltaTime;
    float rotStep = rotSpeed * deltaTime;

    // Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cubePos.z -= moveStep;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cubePos.z += moveStep;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cubePos.x -= moveStep;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cubePos.x += moveStep;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cubePos.y += moveStep;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cubePos.y -= moveStep;

    // Rotation
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) cubeRot.x += rotStep;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) cubeRot.x -= rotStep;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) cubeRot.y += rotStep;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) cubeRot.y -= rotStep;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) cubeRot.z += rotStep;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) cubeRot.z -= rotStep;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Cube Move & Rotate", nullptr, nullptr);
    if (!window) return -1;

    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);

    // Compile shaders
    auto compileShader = [](unsigned int type, const char* src) -> unsigned int {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char info[512];
            glGetShaderInfoLog(shader, 512, nullptr, info);
            std::cerr << "Shader compile error:\n" << info << std::endl;
        }
        return shader;
    };

    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup buffers
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Projection & view
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window, deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePos);
        model = glm::rotate(model, cubeRot.x, glm::vec3(1, 0, 0));
        model = glm::rotate(model, cubeRot.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, cubeRot.z, glm::vec3(0, 0, 1));

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -4.0f));

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(shaderProgram, "time"), currentTime);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
