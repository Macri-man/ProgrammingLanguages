#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

const int WIDTH = 800, HEIGHT = 800;

const char* computeShaderSrc = R"(
#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba32f, binding = 0) uniform image2D img;
uniform float time;

// -------------------- Digit Patterns --------------------
const int w = 5;
const int h = 7;

int digitPattern[10][w * h] = int[10][w * h](
    // 0
    int[w*h](1,1,1,1,1,
             1,0,0,0,1,
             1,0,0,1,1,
             1,0,1,0,1,
             1,1,0,0,1,
             1,0,0,0,1,
             1,1,1,1,1),
    // 1
    int[w*h](0,0,1,0,0,
             0,1,1,0,0,
             1,0,1,0,0,
             0,0,1,0,0,
             0,0,1,0,0,
             0,0,1,0,0,
             1,1,1,1,1),
    // 2
    int[w*h](1,1,1,1,1,
             0,0,0,0,1,
             0,0,0,1,1,
             0,0,1,1,0,
             0,1,1,0,0,
             1,1,0,0,0,
             1,1,1,1,1),
    // 3
    int[w*h](1,1,1,1,1,
             0,0,0,0,1,
             0,0,1,1,1,
             0,0,0,0,1,
             0,0,0,0,1,
             0,0,0,0,1,
             1,1,1,1,1),
    // 4
    int[w*h](1,0,0,1,0,
             1,0,0,1,0,
             1,0,0,1,0,
             1,1,1,1,1,
             0,0,0,1,0,
             0,0,0,1,0,
             0,0,0,1,0),
    // 5
    int[w*h](1,1,1,1,1,
             1,0,0,0,0,
             1,1,1,1,0,
             0,0,0,0,1,
             0,0,0,0,1,
             1,0,0,0,1,
             0,1,1,1,0),
    // 6
    int[w*h](0,1,1,1,1,
             1,0,0,0,0,
             1,0,0,0,0,
             1,1,1,1,0,
             1,0,0,0,1,
             1,0,0,0,1,
             0,1,1,1,0),
    // 7
    int[w*h](1,1,1,1,1,
             0,0,0,0,1,
             0,0,0,1,0,
             0,0,1,0,0,
             0,1,0,0,0,
             0,1,0,0,0,
             0,1,0,0,0),
    // 8
    int[w*h](0,1,1,1,0,
             1,0,0,0,1,
             1,0,0,0,1,
             0,1,1,1,0,
             1,0,0,0,1,
             1,0,0,0,1,
             0,1,1,1,0),
    // 9
    int[w*h](0,1,1,1,0,
             1,0,0,0,1,
             1,0,0,0,1,
             0,1,1,1,1,
             0,0,0,0,1,
             0,0,0,0,1,
             0,1,1,1,0)
);

// -------------------- Main --------------------
void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 resolution = imageSize(img);

    // Settings
    float digitScale = 20.0;            // size of each digit pixel
    int digitToShow = int(mod(floor(time), 10.0));
    ivec2 center = resolution / 2;      // center of screen
    ivec2 digitSize = ivec2(int(digitScale) * w, int(digitScale) * h);

    // Top-left corner of digit area
    ivec2 digitOrigin = center - digitSize / 2;

    // Check if current pixel is inside the digit area
    ivec2 rel = pixel - digitOrigin;

    // Flip control
    bool flipX = false;
    bool flipY = true;

    // Compute digit grid coords
    int gx = int(float(rel.x) / digitScale);
    int gy = int(float(rel.y) / digitScale);

    if (flipX) gx = w - 1 - gx;
    if (flipY) gy = h - 1 - gy;

    if (gx >= 0 && gx < w && gy >= 0 && gy < h) {
        int index = gy * w + gx;
        int bit = digitPattern[digitToShow][index];
        vec4 color = vec4(vec3(bit), 1.0);
        imageStore(img, pixel, color);
    } else {
        imageStore(img, pixel, vec4(0.0));
    }
}
)";




const char* fragmentShaderSrc = R"(
#version 430
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D tex;
void main() {
    FragColor = texture(tex, TexCoords);
}
)";


const char* vertexShaderSrc = R"(
#version 430
layout(location = 0) in vec2 position;
out vec2 TexCoords;
void main() {
    TexCoords = (position + 1.0) * 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
)";


GLuint CompileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint success;
    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "Shader error:\n" << log << std::endl;
    }
    return s;
}

GLuint CreateProgram(const char* vs, const char* fs) {
    GLuint p = glCreateProgram();
    GLuint v = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fs);
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

GLuint CreateComputeProgram(const char* cs) {
    GLuint c = CompileShader(GL_COMPUTE_SHADER, cs);
    GLuint p = glCreateProgram();
    glAttachShader(p, c);
    glLinkProgram(p);
    glDeleteShader(c);
    return p;
}

int main() {
    glfwInit();
    GLFWwindow* win = glfwCreateWindow(WIDTH, HEIGHT, "Compute Shader Animation", NULL, NULL);
    glfwMakeContextCurrent(win);
    // Init GLAD
    if (!gladLoadGL(glfwGetProcAddress)) { std::cerr << "Failed to initialize GLAD\n"; return -1; }

    // Quad setup
    float quad[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };
    GLuint quadVBO, quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, WIDTH, HEIGHT);
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Programs
    GLuint renderProgram = CreateProgram(vertexShaderSrc, fragmentShaderSrc);
    GLuint computeProgram = CreateComputeProgram(computeShaderSrc);

    float startTime = (float)glfwGetTime();

    while (!glfwWindowShouldClose(win)) {
        float time = (float)glfwGetTime() - startTime;

        // Dispatch compute shader
        glUseProgram(computeProgram);
        glUniform1f(glGetUniformLocation(computeProgram, "time"), time);
        glDispatchCompute(WIDTH / 16, HEIGHT / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // Render texture to quad
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(renderProgram);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
