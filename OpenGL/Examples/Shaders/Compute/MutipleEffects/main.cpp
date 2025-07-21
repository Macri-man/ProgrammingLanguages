#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <vector>

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader error:\n" << log << "\n";
    }
    return shader;
}

GLuint createProgram(const char* src, GLenum type) {
    GLuint shader = compileShader(type, src);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);
    glLinkProgram(prog);
    glDeleteShader(shader);
    return prog;
}

// Vertex and Fragment for fullscreen triangle
const char* fullscreenVert = R"(
#version 460 core
const vec2 pos[3] = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1,3));
void main() { gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0); }
)";

const char* fullscreenFrag = R"(
#version 460 core
out vec4 FragColor;
uniform sampler2D screenTex;
void main() {
    vec2 uv = gl_FragCoord.xy / vec2(800.0, 600.0);
    FragColor = texture(screenTex, uv);
}
)";

// Compute shader effects
const char* computeShaders[] = {
    // Invert
    R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform image2D img;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(img);

    // Bounds check to avoid out-of-bounds errors
    if (p.x >= size.x || p.y >= size.y) return;

    vec4 color = imageLoad(img, p);
    
    // Optional: preserve alpha, invert only RGB
    vec3 inverted = vec3(1.0) - color.rgb;

    // Optional: apply gamma or contrast tweak
    // inverted = pow(inverted, vec3(0.8));

    imageStore(img, p, vec4(inverted, color.a));
}
)",

    // Grayscale
    R"(
    #version 460
    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba8, binding = 0) uniform image2D img;
    void main() {
        ivec2 p = ivec2(gl_GlobalInvocationID.xy);
        vec4 c = imageLoad(img, p);
        float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));
        imageStore(img, p, vec4(g, g, g, 1.0));
    })",

    // Checkerboard
    R"(
    #version 460
    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba8, binding = 0) uniform image2D img;
    void main() {
        ivec2 p = ivec2(gl_GlobalInvocationID.xy);
        int check = (p.x / 32 + p.y / 32) % 2;
        vec3 color = check == 0 ? vec3(0.2, 0.2, 0.2) : vec3(1.0, 1.0, 1.0);
        imageStore(img, p, vec4(color, 1.0));
    })",

    // Blur
    R"(
    #version 460
    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba8, binding = 0) uniform image2D img;
    void main() {
        ivec2 p = ivec2(gl_GlobalInvocationID.xy);
        vec3 sum = vec3(0.0);
        for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
            sum += imageLoad(img, p + ivec2(x,y)).rgb;
        imageStore(img, p, vec4(sum / 9.0, 1.0));
    })",

    // Edge Detection
   R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform image2D img;

void main() {
    ivec2 size = imageSize(img);
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);

    if (p.x <= 0 || p.y <= 0 || p.x >= size.x - 1 || p.y >= size.y - 1)
        return;

    float Gx[3][3] = float[3][3](
        float[3](-1.0, 0.0, 1.0),
        float[3](-2.0, 0.0, 2.0),
        float[3](-1.0, 0.0, 1.0)
    );
    float Gy[3][3] = float[3][3](
        float[3](-1.0, -2.0, -1.0),
        float[3]( 0.0,  0.0,  0.0),
        float[3]( 1.0,  2.0,  1.0)
    );

    float sx = 0.0;
    float sy = 0.0;

    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            vec3 rgb = imageLoad(img, p + ivec2(i, j)).rgb;
            float intensity = dot(rgb, vec3(0.299, 0.587, 0.114)); // luminance
            sx += intensity * Gx[i + 1][j + 1];
            sy += intensity * Gy[i + 1][j + 1];
        }
    }

    float edgeStrength = length(vec2(sx, sy)); // sobel magnitude
    edgeStrength = clamp(edgeStrength, 0.0, 1.0);
    
    // Optional enhancement:
    edgeStrength = pow(edgeStrength, 0.5); // gamma correction for visual contrast
    // float threshold = 0.2;
    // edgeStrength = edgeStrength > threshold ? 1.0 : 0.0;

    vec3 edgeColor = vec3(edgeStrength);
    imageStore(img, p, vec4(edgeColor, 1.0));
}
)",

    // Mandelbrot
    R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform image2D img;

vec3 hsv2rgb(vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x * 6.0 + vec3(0,4,2), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec2 uv = (vec2(p) / vec2(800.0, 600.0)) * 3.0 - vec2(2.0, 1.5);

    vec2 z = vec2(0.0);
    vec2 c = uv;

    float maxIter = 300.0;
    float iter = 0.0;

    for (int i = 0; i < int(maxIter); ++i) {
        if (dot(z, z) > 4.0) break;
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        iter = float(i) + 1.0 - log(log(length(z))) / log(2.0);  // Smooth coloring
    }

    float norm = iter / maxIter;
    vec3 color = hsv2rgb(vec3(0.95 - norm * 0.95, 0.6 + 0.4 * norm, norm)); // hue, sat, val
    imageStore(img, p, vec4(color, 1.0));
}
)",

    // Julia
    R"(
    #version 460
    layout(local_size_x = 16, local_size_y = 16) in;
    layout(rgba8, binding = 0) uniform image2D img;
    void main() {
        ivec2 p = ivec2(gl_GlobalInvocationID.xy);
        vec2 uv = (vec2(p) / vec2(800.0, 600.0)) * 3.0 - vec2(1.5, 1.5);
        vec2 z = uv;
        vec2 c = vec2(-0.70176, -0.3842);
        int iter = 0, maxIter = 100;
        while (dot(z,z) < 4.0 && iter < maxIter) {
            z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
            iter++;
        }
        float t = float(iter) / maxIter;
        vec3 col = vec3(0.5 + 0.5*cos(6.2831*t), 0.5 + 0.5*cos(6.2831*t + 2.0), 0.5 + 0.5*cos(6.2831*t + 4.0));
        imageStore(img, p, vec4(col, 1.0));
    })"
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(800, 600, "Compute Shader Effects", 0, 0);
    glfwMakeContextCurrent(win);
    gladLoadGL(glfwGetProcAddress);

    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 800, 600);

    GLuint vao; glGenVertexArrays(1, &vao); glBindVertexArray(vao);
    GLuint vs = compileShader(GL_VERTEX_SHADER, fullscreenVert);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fullscreenFrag);
    GLuint quadProg = glCreateProgram();
    glAttachShader(quadProg, vs); glAttachShader(quadProg, fs); glLinkProgram(quadProg);

    std::vector<GLuint> programs;
    for (const char* src : computeShaders)
        programs.push_back(createProgram(src, GL_COMPUTE_SHADER));

    const char* labels[] = {
        "Invert", "Grayscale", "Checkerboard", "Blur", "Edge", "Mandelbrot", "Julia"
    };
    int currentEffect = 0;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame(); ImGui_ImplGlfw_NewFrame(); ImGui::NewFrame();

        ImGui::Begin("Shader Selector");
        ImGui::Combo("Effect", &currentEffect, labels, IM_ARRAYSIZE(labels));
        ImGui::End();

        glUseProgram(programs[currentEffect]);
        glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
        glDispatchCompute(800 / 16, 600 / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(quadProg);
        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }

    for (auto p : programs) glDeleteProgram(p);
    glDeleteProgram(quadProg);
    glDeleteTextures(1, &tex);
    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext();
    glfwDestroyWindow(win); glfwTerminate();
    return 0;
}
