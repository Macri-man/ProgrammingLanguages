#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <chrono>

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <iostream>
#include <filesystem>

std::string LoadShaderWithIncludes(const std::string& path, std::unordered_set<std::string>& includedFiles) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open shader file: " << path << '\n';
        return "";
    }

    includedFiles.insert(std::filesystem::absolute(path).string());
    std::string dir = std::filesystem::path(path).parent_path().string();

    std::stringstream output;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("#include") == 0) {
            size_t start = line.find('"') + 1;
            size_t end = line.rfind('"');
            std::string includeFile = line.substr(start, end - start);
            std::string includePath = dir + "/" + includeFile;
            std::string absPath = std::filesystem::absolute(includePath).string();

            if (includedFiles.count(absPath)) {
                std::cerr << "Warning: Circular include detected: " << absPath << '\n';
                continue;
            }

            output << "// BEGIN INCLUDE: " << includeFile << "\n";
            output << LoadShaderWithIncludes(includePath, includedFiles);
            output << "// END INCLUDE: " << includeFile << "\n";
        } else {
            output << line << '\n';
        }
    }
    return output.str();
}

std::string LoadShader(const std::string& path) {
    std::unordered_set<std::string> included;
    return LoadShaderWithIncludes(path, included);
}


GLuint CompileShader(GLenum type, const std::string& src) {
    GLuint shader = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(shader, 1, &csrc, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << '\n';
    }
    return shader;
}

GLuint CreateShaderProgram(const std::string& vertPath, const std::string& fragPath) {
    std::string vertSrc = LoadShader(vertPath);
    std::string fragSrc = LoadShader(fragPath);

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Shader linking error:\n" << log << '\n';
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}


float triangle[] = {
     0.0f,  0.5f, 0.0f,
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLSL #include Simulation", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    GLuint program = CreateShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO); glBindVertexArray(VAO);
    glGenBuffers(1, &VBO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    auto startTime = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window)) {
        float time = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - startTime).count();

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glUniform1f(glGetUniformLocation(program, "time"), time);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glfwTerminate();
}
