#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

// Window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Vertex shader (fullscreen quad)
const char* vertexShaderSrc = R"(
#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    TexCoord = aTexCoord;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
)";

// Fragment shader (basic raytracing)
const char* fragmentShaderSrc = R"(
#version 460 core
out vec4 FragColor;
in vec2 TexCoord;

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
};

float intersectSphere(vec3 ro, vec3 rd, Sphere s, out vec3 hitNormal, out float dist) {
    vec3 oc = ro - s.center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - s.radius * s.radius;
    float h = b * b - c;
    if (h < 0.0) return -1.0;

    dist = -b - sqrt(h);
    vec3 hitPoint = ro + rd * dist;
    hitNormal = normalize(hitPoint - s.center);
    return dist;
}

void main() {
    vec2 uv = TexCoord * 2.0 - 1.0;
    uv.x *= 800.0 / 600.0;

    vec3 ro = vec3(0.0, 0.0, -5.0); // Ray origin
    vec3 rd = normalize(vec3(uv, 1.0)); // Ray direction

    Sphere s1 = Sphere(vec3(0.0, 0.0, 0.0), 1.0, vec3(1.0, 0.2, 0.2));
    Sphere s2 = Sphere(vec3(-2.0, 0.0, 0.0), 1.0, vec3(0.2, 0.2, 1.0));

    vec3 hitNormal1, hitNormal2;
    float dist1, dist2;

    float t1 = intersectSphere(ro, rd, s1, hitNormal1, dist1);
    float t2 = intersectSphere(ro, rd, s2, hitNormal2, dist2);

    vec3 color = vec3(0.0); // default = background

    if (t1 > 0.0 && (t1 < t2 || t2 < 0.0)) {
        // Visualize normal for s1
        color = hitNormal1 * 0.5 + 0.5; // remap [-1,1] to [0,1]
        // color = vec3(1.0) * (1.0 - dist1 / 10.0); // visualize depth
        // color = s1.color; // uncomment to just show object color
    }
    else if (t2 > 0.0) {
        color = hitNormal2 * 0.5 + 0.5;
        // color = vec3(1.0) * (1.0 - dist2 / 10.0);
        // color = s2.color;
    }

    FragColor = vec4(color, 1.0);
}
)";


// Compile shader
GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << '\n';
    }
    return shader;
}

// Link shader program
GLuint CreateProgram(GLuint vs, GLuint fs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "Program linking error:\n" << infoLog << '\n';
    }
    return program;
}

// Fullscreen quad
void createScreenQuad(GLuint& VAO, GLuint& VBO) {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLSL Raytracing", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) { std::cerr << "Failed to initialize GLAD\n"; return -1; }

    GLuint quadVAO, quadVBO;
    createScreenQuad(quadVAO, quadVBO);

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint renderProgram = CreateProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(renderProgram);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteProgram(renderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
