#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <string>

// Shader compilation helpers
GLuint createComputeShader(const char* src) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Compute shader compile error:\n" << log << std::endl;
        return 0;
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);
    glLinkProgram(prog);
    glDeleteShader(shader);
    GLint linked;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "Shader program link error:\n" << log << std::endl;
        return 0;
    }
    return prog;
}

GLuint createShaderProgram(const char* vertSrc, const char* fragSrc) {
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertSrc, nullptr);
    glCompileShader(vert);
    GLint success;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(vert, 1024, nullptr, log);
        std::cerr << "Vertex shader compile error:\n" << log << std::endl;
        return 0;
    }
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragSrc, nullptr);
    glCompileShader(frag);
    glGetShaderiv(frag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(frag, 1024, nullptr, log);
        std::cerr << "Fragment shader compile error:\n" << log << std::endl;
        return 0;
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "Shader program link error:\n" << log << std::endl;
        return 0;
    }
    return prog;
}

// Fullscreen quad shaders
const char* fullscreenVert = R"(
#version 460
const vec2 verts[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(3.0, -1.0),
    vec2(-1.0, 3.0)
);
out vec2 uv;
void main() {
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
    uv = (verts[gl_VertexID] + 1.0) * 0.5;
}
)";

const char* fullscreenFrag = R"(
#version 460
in vec2 uv;
out vec4 FragColor;
uniform sampler2D tex;
void main() {
    FragColor = texture(tex, uv);
}
)";

// Mandelbrot compute shader
const char* fractalMandelbrot = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

uniform vec2 u_offset;
uniform float u_zoom;
uniform int u_maxIter;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    if (p.x >= size.x || p.y >= size.y) return;

    vec2 uv = (vec2(p) / vec2(size) - 0.5) * 3.0 / u_zoom + u_offset;

    vec2 z = vec2(0.0);
    vec2 c = uv;
    int iter = 0;
    for (int i = 0; i < u_maxIter; ++i) {
        if(dot(z,z) > 4.0) break;
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
        iter = i;
    }
    float norm = float(iter) / float(u_maxIter);
    vec3 col = mix(vec3(0.0, 0.0, 0.8), vec3(1.0, 0.3, 0.0), norm);
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

// Julia compute shader
const char* fractalJulia = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

uniform vec2 u_offset;
uniform float u_zoom;
uniform int u_maxIter;
uniform vec2 u_c;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    if (p.x >= size.x || p.y >= size.y) return;

    vec2 uv = (vec2(p) / vec2(size) - 0.5) * 3.0 / u_zoom + u_offset;

    vec2 z = uv;
    vec2 c = u_c;
    int iter = 0;
    for (int i = 0; i < u_maxIter; ++i) {
        if(dot(z,z) > 4.0) break;
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
        iter = i;
    }
    float norm = float(iter) / float(u_maxIter);
    vec3 col = mix(vec3(0.0, 0.8, 0.0), vec3(1.0, 1.0, 0.0), norm);
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

// Burning Ship fractal compute shader
const char* fractalBurningShip = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

uniform vec2 u_offset;
uniform float u_zoom;
uniform int u_maxIter;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    if (p.x >= size.x || p.y >= size.y) return;

    vec2 uv = (vec2(p) / vec2(size) - 0.5) * 3.0 / u_zoom + u_offset;

    vec2 z = vec2(0.0);
    vec2 c = uv;
    int iter = 0;
    for (int i = 0; i < u_maxIter; ++i) {
        if(dot(z,z) > 4.0) break;
        z = vec2(abs(z.x), abs(z.y));
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
        iter = i;
    }
    float norm = float(iter) / float(u_maxIter);
    vec3 col = mix(vec3(0.5, 0.0, 0.0), vec3(1.0, 0.7, 0.0), norm);
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

// Sierpinski carpet compute shader
const char* fractalSierpinski = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

uniform int u_levels;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    if (p.x >= size.x || p.y >= size.y) return;

    int x = p.x;
    int y = p.y;

    bool isFilled = true;
    int levels = u_levels;

    while(levels > 0) {
        if ((x % 3 == 1) && (y % 3 == 1)) {
            isFilled = false;
            break;
        }
        x /= 3;
        y /= 3;
        levels--;
    }

    vec3 col = isFilled ? vec3(0.0) : vec3(1.0);
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

void dispatch(GLuint prog, int w, int h) {
    glUseProgram(prog);
    glDispatchCompute((w + 15) / 16, (h + 15) / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,6);
    GLFWwindow* window = glfwCreateWindow(800,600,"Fractal Viewer",nullptr,nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 460");

    const int WIDTH = 800;
    const int HEIGHT = 600;

    // Create texture for fractal output
    GLuint fractalTex;
    glGenTextures(1, &fractalTex);
    glBindTexture(GL_TEXTURE_2D, fractalTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, WIDTH, HEIGHT);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Compile compute shaders
    GLuint mandelbrotProg = createComputeShader(fractalMandelbrot);
    GLuint juliaProg = createComputeShader(fractalJulia);
    GLuint burningShipProg = createComputeShader(fractalBurningShip);
    GLuint sierpinskiProg = createComputeShader(fractalSierpinski);

    // Compile fullscreen quad shader
    GLuint fullscreenProg = createShaderProgram(fullscreenVert, fullscreenFrag);

    GLuint quadVAO;
    glGenVertexArrays(1, &quadVAO);

    enum FractalType {MANDELBROT=0, JULIA=1, BURNING_SHIP=2, SIERPINSKI=3};
    int currentFractal = MANDELBROT;

    // Parameters structs
    struct MandelbrotParams {
        float offsetX = -0.5f;
        float offsetY = 0.0f;
        float zoom = 1.0f;
        int maxIter = 300;
    } mandelbrotParams;

    struct JuliaParams {
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float zoom = 1.0f;
        int maxIter = 300;
        float cRe = -0.70176f;
        float cIm = -0.3842f;
    } juliaParams;

    struct BurningShipParams {
        float offsetX = -0.5f;
        float offsetY = -0.5f;
        float zoom = 1.0f;
        int maxIter = 300;
    } burningShipParams;

    struct SierpinskiParams {
        int levels = 5; // recursion depth
    } sierpinskiParams;

    bool paramsDirty = true;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Fractal & Settings");
        int oldFractal = currentFractal;
        ImGui::Combo("Fractal", &currentFractal, "Mandelbrot\0Julia\0Burning Ship\0Sierpinski Carpet\0");

        if (currentFractal == MANDELBROT) {
            paramsDirty |= ImGui::SliderFloat("Offset X", &mandelbrotParams.offsetX, -2.0f, 2.0f);
            paramsDirty |= ImGui::SliderFloat("Offset Y", &mandelbrotParams.offsetY, -2.0f, 2.0f);
            paramsDirty |= ImGui::SliderFloat("Zoom", &mandelbrotParams.zoom, 0.1f, 10.0f);
            paramsDirty |= ImGui::SliderInt("Max Iterations", &mandelbrotParams.maxIter, 10, 1000);
        } else if (currentFractal == JULIA) {
            paramsDirty |= ImGui::SliderFloat("Offset X", &juliaParams.offsetX, -2.0f, 2.0f);
            paramsDirty |= ImGui::SliderFloat("Offset Y", &juliaParams.offsetY, -2.0f, 2.0f);
            paramsDirty |= ImGui::SliderFloat("Zoom", &juliaParams.zoom, 0.1f, 10.0f);
            paramsDirty |= ImGui::SliderInt("Max Iterations", &juliaParams.maxIter, 10, 1000);
            paramsDirty |= ImGui::SliderFloat("c Real", &juliaParams.cRe, -1.0f, 1.0f);
            paramsDirty |= ImGui::SliderFloat("c Imag", &juliaParams.cIm, -1.0f, 1.0f);
        } else if (currentFractal == BURNING_SHIP) {
            paramsDirty |= ImGui::SliderFloat("Offset X", &burningShipParams.offsetX, -2.0f, 2.0f);
            paramsDirty |= ImGui::SliderFloat("Offset Y", &burningShipParams.offsetY, -2.0f, 2.0f);
            paramsDirty |= ImGui::SliderFloat("Zoom", &burningShipParams.zoom, 0.1f, 10.0f);
            paramsDirty |= ImGui::SliderInt("Max Iterations", &burningShipParams.maxIter, 10, 1000);
        } else if (currentFractal == SIERPINSKI) {
            paramsDirty |= ImGui::SliderInt("Levels", &sierpinskiParams.levels, 1, 7);
        }

        if (oldFractal != currentFractal)
            paramsDirty = true;

        ImGui::End();

        if (paramsDirty) {
            glBindImageTexture(0, fractalTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

            if (currentFractal == MANDELBROT) {
                glUseProgram(mandelbrotProg);
                glUniform2f(glGetUniformLocation(mandelbrotProg, "u_offset"), mandelbrotParams.offsetX, mandelbrotParams.offsetY);
                glUniform1f(glGetUniformLocation(mandelbrotProg, "u_zoom"), mandelbrotParams.zoom);
                glUniform1i(glGetUniformLocation(mandelbrotProg, "u_maxIter"), mandelbrotParams.maxIter);
                dispatch(mandelbrotProg, WIDTH, HEIGHT);
            }
            else if (currentFractal == JULIA) {
                glUseProgram(juliaProg);
                glUniform2f(glGetUniformLocation(juliaProg, "u_offset"), juliaParams.offsetX, juliaParams.offsetY);
                glUniform1f(glGetUniformLocation(juliaProg, "u_zoom"), juliaParams.zoom);
                glUniform1i(glGetUniformLocation(juliaProg, "u_maxIter"), juliaParams.maxIter);
                glUniform2f(glGetUniformLocation(juliaProg, "u_c"), juliaParams.cRe, juliaParams.cIm);
                dispatch(juliaProg, WIDTH, HEIGHT);
            }
            else if (currentFractal == BURNING_SHIP) {
                glUseProgram(burningShipProg);
                glUniform2f(glGetUniformLocation(burningShipProg, "u_offset"), burningShipParams.offsetX, burningShipParams.offsetY);
                glUniform1f(glGetUniformLocation(burningShipProg, "u_zoom"), burningShipParams.zoom);
                glUniform1i(glGetUniformLocation(burningShipProg, "u_maxIter"), burningShipParams.maxIter);
                dispatch(burningShipProg, WIDTH, HEIGHT);
            }
            else if (currentFractal == SIERPINSKI) {
                glUseProgram(sierpinskiProg);
                glUniform1i(glGetUniformLocation(sierpinskiProg, "u_levels"), sierpinskiParams.levels);
                dispatch(sierpinskiProg, WIDTH, HEIGHT);
            }
            paramsDirty = false;
        }

        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(fullscreenProg);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fractalTex);
        glUniform1i(glGetUniformLocation(fullscreenProg, "tex"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteProgram(mandelbrotProg);
    glDeleteProgram(juliaProg);
    glDeleteProgram(burningShipProg);
    glDeleteProgram(sierpinskiProg);
    glDeleteProgram(fullscreenProg);
    glDeleteTextures(1, &fractalTex);
    glDeleteVertexArrays(1, &quadVAO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
