#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath> 

// --- Shader utility functions ---

GLuint compileShader(GLenum type, const std::string& source, const std::string& name) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "[ERROR] Shader compilation failed (" << name << "):\n" << log << std::endl;
    }
    return shader;
}

GLuint createShaderProgram(const std::string& vertexSrc,
                           const std::string& tessCtrlSrc,
                           const std::string& tessEvalSrc,
                           const std::string& fragmentSrc) {
    GLuint program = glCreateProgram();

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSrc, "Vertex");
    GLuint tcs = compileShader(GL_TESS_CONTROL_SHADER, tessCtrlSrc, "Tessellation Control");
    GLuint tes = compileShader(GL_TESS_EVALUATION_SHADER, tessEvalSrc, "Tessellation Eval");
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSrc, "Fragment");

    glAttachShader(program, vs);
    glAttachShader(program, tcs);
    glAttachShader(program, tes);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "[ERROR] Shader program linking failed:\n" << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(tcs);
    glDeleteShader(tes);
    glDeleteShader(fs);

    return program;
}

// --- GLSL shader sources ---

const std::string vertexShaderSrc = R"GLSL(
#version 400 core
layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)GLSL";

const std::string tessControlShaderSrc = R"GLSL(
#version 400 core
layout(vertices = 4) out;

uniform float tessLevel;

void main() {
    if (gl_InvocationID == 0) {
        float level = max(tessLevel, 1.0); // clamp to min 1.0
        gl_TessLevelOuter[0] = level;
        gl_TessLevelOuter[1] = level;
        gl_TessLevelOuter[2] = level;
        gl_TessLevelOuter[3] = level;
        gl_TessLevelInner[0] = level;
        gl_TessLevelInner[1] = level;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
)GLSL";

const std::string tessEvalShaderSrc = R"GLSL(
#version 400 core
layout(quads, equal_spacing, ccw) in;

void main() {
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec4 p0 = gl_in[0].gl_Position;
    vec4 p1 = gl_in[1].gl_Position;
    vec4 p2 = gl_in[2].gl_Position;
    vec4 p3 = gl_in[3].gl_Position;

    vec4 a = mix(p0, p1, u);
    vec4 b = mix(p3, p2, u);
    vec4 pos = mix(a, b, v);

    // Displacement for better visualization:
    float disp = 0.1 * sin(10.0 * u) * cos(10.0 * v);
    pos.z += disp;

    gl_Position = pos;
}
)GLSL";

const std::string fragmentShaderSrc = R"GLSL(
#version 400 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.3, 0.7, 1.0, 1.0);
}
)GLSL";

// --- GLFW callback ---

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

// --- Setup VAO/VBO for a quad patch ---

void setupQuadVAO(GLuint& vao, GLuint& vbo) {
    float quadVertices[] = {
        -0.5f, -0.5f,  // bottom-left
         0.5f, -0.5f,  // bottom-right
         0.5f,  0.5f,  // top-right
        -0.5f,  0.5f   // top-left
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

// --- Main ---

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Tessellation Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions using GLAD2
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Enable wireframe mode to visualize tessellation edges
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Compile and link shader program
    GLuint shaderProgram = createShaderProgram(vertexShaderSrc, tessControlShaderSrc, tessEvalShaderSrc, fragmentShaderSrc);

    // Setup quad VAO/VBO
    GLuint vao = 0, vbo = 0;
    setupQuadVAO(vao, vbo);

    // Set patch parameters
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // Get uniform location for tessLevel
    GLint tessLevelLoc = glGetUniformLocation(shaderProgram, "tessLevel");

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Animate tessellation level from 1 to 8 using sine wave
        float time = static_cast<float>(glfwGetTime());
        float tessLevel = 1.0f + 7.0f * (0.5f + 0.5f * std::sin(time));
        if (tessLevelLoc != -1) {
            glUniform1f(tessLevelLoc, tessLevel);
        }

        glBindVertexArray(vao);
        glDrawArrays(GL_PATCHES, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
