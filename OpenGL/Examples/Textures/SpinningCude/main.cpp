#include <glad/gl.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <random>

// Shader sources
const char* vertexShaderSrc = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aFaceID;

out vec2 TexCoord;
flat out float faceID;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    faceID = aFaceID;
}
)";

const char* fragmentShaderSrc = R"(
#version 460 core
out vec4 FragColor;
in vec2 TexCoord;
flat in float faceID;

uniform sampler2D textures[6];

void main() {
    FragColor = texture(textures[int(faceID)], TexCoord);
}
)";

GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());
        std::cerr << "Shader compile error:\n" << infoLog.data() << std::endl;
    }
    return shader;
}

GLuint LoadTexture(const char* path) {
    int w, h, c;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &c, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format = (c == 4) ? GL_RGBA : GL_RGB;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
    float faceID;
};

Vertex cubeVerts[] = {
    // Front (+Z)
    {{-0.5f,-0.5f, 0.5f}, {0.0f, 0.0f}, 0.0f},
    {{ 0.5f,-0.5f, 0.5f}, {1.0f, 0.0f}, 0.0f},
    {{ 0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, 0.0f},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}, 0.0f},

    // Back (-Z)
    {{ 0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}, 1.0f},
    {{-0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}, 1.0f},
    {{-0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}, 1.0f},
    {{ 0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}, 1.0f},

    // Left (-X)
    {{-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}, 2.0f},
    {{-0.5f,-0.5f, 0.5f}, {1.0f, 0.0f}, 2.0f},
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, 2.0f},
    {{-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}, 2.0f},

    // Right (+X)
    {{ 0.5f,-0.5f, 0.5f}, {0.0f, 0.0f}, 3.0f},
    {{ 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}, 3.0f},
    {{ 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}, 3.0f},
    {{ 0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}, 3.0f},

    // Top (+Y)
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, 4.0f},
    {{ 0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}, 4.0f},
    {{ 0.5f, 0.5f,-0.5f}, {1.0f, 1.0f}, 4.0f},
    {{-0.5f, 0.5f,-0.5f}, {0.0f, 1.0f}, 4.0f},

    // Bottom (-Y)
    {{-0.5f,-0.5f,-0.5f}, {0.0f, 0.0f}, 5.0f},
    {{ 0.5f,-0.5f,-0.5f}, {1.0f, 0.0f}, 5.0f},
    {{ 0.5f,-0.5f, 0.5f}, {1.0f, 1.0f}, 5.0f},
    {{-0.5f,-0.5f, 0.5f}, {0.0f, 1.0f}, 5.0f}
};

GLuint indices[] = {
    0, 1, 2, 2, 3, 0,       // Front
    4, 5, 6, 6, 7, 4,       // Back
    8, 9,10,10,11, 8,       // Left
   12,13,14,14,15,12,       // Right
   16,17,18,18,19,16,       // Top
   20,21,22,22,23,20        // Bottom
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Spinning Cube", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Compile shaders and create program
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint logLen;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(logLen);
        glGetProgramInfoLog(shaderProgram, logLen, nullptr, log.data());
        std::cerr << "Shader link error:\n" << log.data() << std::endl;
        return -1;
    }

    // Setup VAO, VBO, EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, faceID));
    glEnableVertexAttribArray(2);

    // Load textures randomly from folder
    std::vector<std::string> allFiles;
    for (auto& e : std::filesystem::directory_iterator("textures")) {
        if (e.path().extension() == ".png")
            allFiles.push_back(e.path().string());
    }

    if (allFiles.size() < 6) {
        std::cerr << "Error: Need at least 6 PNG textures in 'textures/' folder.\n";
        return -1;
    }

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::random_shuffle(allFiles.begin(), allFiles.end());

    std::vector<GLuint> textures(6);
    for (int i = 0; i < 6; ++i) {
        textures[i] = LoadTexture(allFiles[i].c_str());
        if (textures[i] == 0) {
            std::cerr << "Failed to load texture: " << allFiles[i] << std::endl;
            return -1;
        }
    }

    // Tell shader which texture units to use
    glUseProgram(shaderProgram);
    for (int i = 0; i < 6; ++i) {
        std::string name = "textures[" + std::to_string(i) + "]";
        glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), i);
    }

    glEnable(GL_DEPTH_TEST);

    float angleX = static_cast<float>(std::rand() % 360);
    float angleY = static_cast<float>(std::rand() % 360);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.12f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        angleX += 0.3f;
        angleY += 0.2f;

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(angleX), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(angleY), glm::vec3(0, 1, 0));
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        for (int i = 0; i < 6; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    for (GLuint tex : textures)
        glDeleteTextures(1, &tex);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
