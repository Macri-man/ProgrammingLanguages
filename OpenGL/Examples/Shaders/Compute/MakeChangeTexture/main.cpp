#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>


const char* computeShaderSrc = R"(
#version 430
layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba32f, binding = 0) uniform image2D img;
uniform float time;

void main() {
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = coords / vec2(imageSize(img));
    vec3 color = vec3(uv.x, uv.y, abs(sin(time)));
    imageStore(img, coords, vec4(color, 1.0));
}
)";

const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;
out vec2 TexCoords;

void main() {
    TexCoords = tex;
    gl_Position = vec4(pos, 0.0, 1.0);
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D tex;

void main() {
    FragColor = texture(tex, TexCoords);
}
)";

// Helper to compile shader
GLuint CompileShader(const char* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return shader;
}

// Helper to link program
GLuint LinkProgram(GLuint* shaders, int count) {
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; ++i) {
        glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }
    for (int i = 0; i < count; ++i) {
        glDeleteShader(shaders[i]);
    }
    return program;
}

int main() {
    // Initialize GLFW and OpenGL context
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 800, "Compute Shader Embedded Example", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    // Init GLAD
    if (!gladLoadGL(glfwGetProcAddress)) { std::cerr << "Failed to initialize GLAD\n"; return -1; }

    // Compile shaders
    GLuint compShader = CompileShader(computeShaderSrc, GL_COMPUTE_SHADER);
    GLuint computeProgram = LinkProgram(&compShader, 1);

    GLuint vs = CompileShader(vertexShaderSrc, GL_VERTEX_SHADER);
    GLuint fs = CompileShader(fragmentShaderSrc, GL_FRAGMENT_SHADER);
    GLuint shaders[] = { vs, fs };
    GLuint screenProgram = LinkProgram(shaders, 2);

    // Setup fullscreen quad
    float quadVertices[] = {
        // positions   // tex coords
        -1.f, -1.f,   0.f, 0.f,
         1.f, -1.f,   1.f, 0.f,
         1.f,  1.f,   1.f, 1.f,
        -1.f,  1.f,   0.f, 1.f,
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // pos
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // tex
    glEnableVertexAttribArray(1);

    // Create GPU texture (512x512 RGBA32F)
    const int texSize = 512;
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, texSize, texSize);

    // Bind image unit 0 for compute shader write
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Tell the screen shader to use texture unit 0
    glUseProgram(screenProgram);
    glUniform1i(glGetUniformLocation(screenProgram, "tex"), 0);

    auto start = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window)) {
        // Calculate elapsed time
        float time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();

        // Run compute shader
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "time"), time);
        glDispatchCompute(texSize / 16, texSize / 16, 1);

        // Ensure compute shader writes are finished before rendering
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Render textured quad
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screenProgram);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
