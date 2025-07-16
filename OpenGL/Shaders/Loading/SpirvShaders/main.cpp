#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <vector>

std::vector<uint32_t> LoadSPIRV(const char* path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open SPIR-V file: " << path << '\n';
        return {};
    }

    std::streamsize size = file.tellg();
    if (size % 4 != 0) {
        std::cerr << "SPIR-V file size is not 4-byte aligned: " << size << " bytes\n";
        return {};
    }

    file.seekg(0, std::ios::beg);
    std::vector<uint32_t> spirv(size / 4);
    if (!file.read(reinterpret_cast<char*>(spirv.data()), size)) {
        std::cerr << "Failed to read SPIR-V binary data from file: " << path << '\n';
        return {};
    }

    return spirv;
}


GLuint CreateShaderFromSPIRV(const char* path, GLenum type) {
    auto spirvBytes = LoadSPIRV(path);
    if (spirvBytes.empty()) {
        std::cerr << "Failed to read SPIR-V data from: " << path << "\n";
        return 0;
    }

    GLuint shader = glCreateShader(type);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvBytes.data(), spirvBytes.size() * sizeof(uint32_t));
    glSpecializeShader(shader, "main", 0, nullptr, nullptr); // "main" entry point

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "SPIR-V Shader error:\n" << log << '\n';
    }

    return shader;
}

GLuint CreateProgramFromSPIRV(const char* vertSPV, const char* fragSPV) {
    GLuint vs = CreateShaderFromSPIRV(vertSPV, GL_VERTEX_SHADER);
    GLuint fs = CreateShaderFromSPIRV(fragSPV, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program linking error:\n" << log << '\n';
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

float vertices[] = {
     0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SPIR-V Shader Example", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) { std::cerr << "Failed to initialize GLAD\n"; return -1; }

    GLuint shaderProgram = CreateProgramFromSPIRV("shaders/vertex.spv", "shaders/fragment.spv");

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
