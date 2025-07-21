#include <glad/gl.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <filesystem>  // C++17
#include <cctype>      // for std::tolower

// --- Load all PNG/JPG/JPEG textures from folder (no namespace alias) ---
std::vector<std::string> loadTextureList(const std::string& folder) {
    std::vector<std::string> result;
    if (!std::filesystem::exists(folder)) {
        std::cerr << "Warning: Textures folder '" << folder << "' does not exist!\n";
        return result;
    }
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            for (auto& c : ext) c = static_cast<char>(std::tolower(c));
            if (ext == ".png" || ext == ".jpg" || ext == ".jpeg")
                result.push_back(entry.path().string());
        }
    }
    if (result.empty())
        std::cerr << "Warning: No images found in '" << folder << "'\n";
    return result;
}

// --- Vertex Shader for screen quad ---
const char* quadVS = R"(#version 460
out vec2 uv;
void main() {
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    uv = pos;
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
})";

// --- Fragment Shader to display texture ---
const char* quadFS = R"(#version 460
in vec2 uv;
out vec4 fragColor;
uniform sampler2D tex;
void main() {
    fragColor = texture(tex, uv);
})";

// --- Compute Shaders ---

const char* invertCS = R"(#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputTex;
layout(rgba8, binding = 1) uniform writeonly image2D outputImg;
void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = textureSize(inputTex, 0);
    if (p.x >= size.x || p.y >= size.y) return;
    vec4 c = texelFetch(inputTex, p, 0);
    imageStore(outputImg, p, vec4(1.0 - c.rgb, c.a));
})";

const char* grayscaleCS = R"(#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputTex;
layout(rgba8, binding = 1) uniform writeonly image2D outputImg;
void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = textureSize(inputTex, 0);
    if (p.x >= size.x || p.y >= size.y) return;
    vec4 c = texelFetch(inputTex, p, 0);
    float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));
    imageStore(outputImg, p, vec4(g, g, g, c.a));
})";

const char* blurCS = R"(#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputTex;
layout(rgba8, binding = 1) uniform writeonly image2D outputImg;
void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = textureSize(inputTex, 0);
    if (p.x >= size.x || p.y >= size.y) return;
    vec4 sum = vec4(0.0);
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) {
            ivec2 q = p + ivec2(dx, dy);
            if (q.x >= 0 && q.y >= 0 && q.x < size.x && q.y < size.y) {
                sum += texelFetch(inputTex, q, 0);
                count++;
            }
        }
    imageStore(outputImg, p, sum / float(count));
})";

const char* sobelCS = R"(#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputTex;
layout(rgba8, binding = 1) uniform writeonly image2D outputImg;
void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = textureSize(inputTex, 0);
    if (p.x >= size.x || p.y >= size.y) return;

    float kx[9] = float[9](-1,0,1,-2,0,2,-1,0,1);
    float ky[9] = float[9](-1,-2,-1,0,0,0,1,2,1);

    vec3 gx = vec3(0);
    vec3 gy = vec3(0);
    int idx = 0;

    for (int y = -1; y <= 1; ++y)
    for (int x = -1; x <= 1; ++x, ++idx) {
        ivec2 coord = p + ivec2(x, y);
        if (coord.x < 0 || coord.y < 0 || coord.x >= size.x || coord.y >= size.y)
            continue;
        vec3 c = texelFetch(inputTex, coord, 0).rgb;
        gx += c * kx[idx];
        gy += c * ky[idx];
    }

    float mag = length(gx + gy);
    imageStore(outputImg, p, vec4(vec3(mag), 1.0));
})";

const char* sepiaCS = R"(#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputTex;
layout(rgba8, binding = 1) uniform writeonly image2D outputImg;
void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec4 c = texelFetch(inputTex, p, 0);
    float r = dot(c.rgb, vec3(0.393, 0.769, 0.189));
    float g = dot(c.rgb, vec3(0.349, 0.686, 0.168));
    float b = dot(c.rgb, vec3(0.272, 0.534, 0.131));
    imageStore(outputImg, p, vec4(clamp(vec3(r, g, b), 0.0, 1.0), c.a));
})";

const char* binarizeCS = R"(#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0) uniform sampler2D inputTex;
layout(rgba8, binding = 1) uniform writeonly image2D outputImg;
void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec4 c = texelFetch(inputTex, p, 0);
    float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));
    float b = g > 0.5 ? 1.0 : 0.0;
    imageStore(outputImg, p, vec4(vec3(b), c.a));
})";

// --- Shader helper functions ---
GLuint createShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader error:\n" << log << std::endl;
    }
    return shader;
}

GLuint createProgram(GLenum type, const char* src) {
    GLuint shader = createShader(type, src);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, shader);
    glLinkProgram(prog);
    glDeleteShader(shader);
    return prog;
}

// --- Load texture with ping-pong pair ---
std::pair<GLuint, GLuint> loadTexturePingPong(const char* path, int& w, int& h) {
    int c;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* img = stbi_load(path, &w, &h, &c, 4);
    if (!img) {
        std::cerr << "Failed to load image: " << path << "\n";
        return {0, 0};
    }

    GLuint texA, texB;
    auto create = [&](GLuint& tex) {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    };
    create(texA);
    create(texB);
    stbi_image_free(img);
    return {texA, texB};
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    GLFWwindow* win = glfwCreateWindow(1280, 720, "Ping-Pong Compute Effects", nullptr, nullptr);
    if (!win) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(win);
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    glEnable(GL_TEXTURE_2D);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    std::vector<std::string> textures = loadTextureList("textures");
    if (textures.empty()) {
        std::cerr << "No textures found in 'textures' folder. Exiting.\n";
        return -1;
    }

    const char* shaderNames[] = {
        "Invert", "Grayscale", "Blur",
        "Sobel Edge", "Sepia", "Binarize"
    };

    const char* shaderSrcs[] = {
        invertCS, grayscaleCS, blurCS,
        sobelCS, sepiaCS, binarizeCS
    };

    bool toggles[6] = { false };

    std::vector<GLuint> computeProgs;
    for (const char* src : shaderSrcs)
        computeProgs.push_back(createProgram(GL_COMPUTE_SHADER, src));

    GLuint quadProg = glCreateProgram();
    glAttachShader(quadProg, createShader(GL_VERTEX_SHADER, quadVS));
    glAttachShader(quadProg, createShader(GL_FRAGMENT_SHADER, quadFS));
    glLinkProgram(quadProg);

    int selected = 0, texW = 0, texH = 0;
    GLuint texA = 0, texB = 0;
    bool reload = true;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

       if (ImGui::Begin("Controls")) {
    if (ImGui::Combo("Texture", &selected,
        [](void* data, int idx, const char** out) {
            auto& vec = *static_cast<std::vector<std::string>*>(data);
            *out = vec[idx].c_str();
            return true;
        }, (void*)&textures, (int)textures.size()))
        reload = true;

    for (int i = 0; i < 6; ++i)
        ImGui::Checkbox(shaderNames[i], &toggles[i]);

    if (ImGui::Button("Reset")) {
        for (int i = 0; i < 6; ++i)
            toggles[i] = false;
        reload = true;
    }
}
ImGui::End();

        if (reload) {
            if (texA) glDeleteTextures(1, &texA);
            if (texB) glDeleteTextures(1, &texB);
            std::tie(texA, texB) = loadTexturePingPong(textures[selected].c_str(), texW, texH);
            reload = false;
        }

        GLuint srcTex = texA, dstTex = texB;
        for (int i = 0; i < 6; ++i) {
            if (!toggles[i]) continue;
            glUseProgram(computeProgs[i]);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, srcTex);

            glBindImageTexture(0, srcTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
            glBindImageTexture(1, dstTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

            glUniform1i(glGetUniformLocation(computeProgs[i], "inputTex"), 0);

            glDispatchCompute((texW + 15) / 16, (texH + 15) / 16, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            std::swap(srcTex, dstTex);
        }

        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(quadProg);
        glBindTexture(GL_TEXTURE_2D, srcTex);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(win);
    }

    glDeleteProgram(quadProg);
    for (auto p : computeProgs) glDeleteProgram(p);
    glDeleteTextures(1, &texA);
    glDeleteTextures(1, &texB);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}
