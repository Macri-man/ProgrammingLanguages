#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <string>

// Compile and link a compute shader program from source
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

// Compile and link a vertex+fragment shader program from source
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

const char* fractalMandelbrot = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

vec3 hsv2rgb(vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0,4,2),6.0)-3.0)-1.0,0.0,1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    vec2 uv = vec2(p) / vec2(size) * 3.0 - vec2(2.0, 1.5);

    vec2 z = vec2(0.0);
    vec2 c = uv;
    float maxIter = 300.0;
    float iter = 0.0;
    for(int i=0; i<int(maxIter); ++i) {
        if(dot(z,z) > 4.0) break;
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
        iter = float(i) + 1.0 - log(log(length(z)))/log(2.0);
    }
    float norm = iter/maxIter;
    vec3 col = hsv2rgb(vec3(0.95 - norm*0.95, 0.6 + 0.4*norm, norm));
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

const char* fractalJulia = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

vec3 hsv2rgb(vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0,4,2),6.0)-3.0)-1.0,0.0,1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    vec2 uv = vec2(p) / vec2(size) * 3.0 - vec2(1.5, 1.5);

    vec2 z = uv;
    vec2 c = vec2(-0.70176, -0.3842);
    float maxIter = 300.0;
    float iter = 0.0;
    for(int i=0; i<int(maxIter); ++i) {
        if(dot(z,z) > 4.0) break;
        z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + c;
        iter = float(i) + 1.0 - log(log(length(z)))/log(2.0);
    }
    float norm = iter/maxIter;
    vec3 col = hsv2rgb(vec3(0.95 - norm*0.95, 0.6 + 0.4*norm, norm));
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

const char* fractalFlame = R"(
#version 460
layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba8, binding = 0) uniform writeonly image2D dstImg;

vec3 hsv2rgb(vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x*6.0 + vec3(0,4,2),6.0)-3.0)-1.0,0.0,1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))*43758.5453123);
}

vec2 transform(vec2 p, float a) {
    return vec2(
        sin(a)*p.x + cos(a)*p.y,
        cos(a)*p.x - sin(a)*p.y
    );
}

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(dstImg);
    vec2 uv = vec2(p) / vec2(size) * 2.0 - vec2(1.0);

    // Simple iterative flame formula
    vec2 pt = uv;
    float brightness = 0.0;
    float angle = 1.2;
    for (int i = 0; i < 20; ++i) {
        pt = transform(pt, angle) + vec2(0.5, 0.5);
        brightness += exp(-length(pt)*10.0);
        pt = fract(pt);
    }
    brightness = clamp(brightness, 0.0, 1.0);

    vec3 col = hsv2rgb(vec3(0.6 + brightness*0.4, 0.8, brightness));
    imageStore(dstImg, p, vec4(col, 1.0));
}
)";

const char* grayscaleShader = R"(
#version 460
layout(local_size_x=16, local_size_y=16) in;
layout(rgba8, binding=0) uniform readonly image2D srcImg;
layout(rgba8, binding=1) uniform writeonly image2D dstImg;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec3 c = imageLoad(srcImg, p).rgb;
    float gray = dot(c, vec3(0.299, 0.587, 0.114));
    imageStore(dstImg, p, vec4(gray, gray, gray, 1.0));
}
)";

const char* invertShader = R"(
#version 460
layout(local_size_x=16, local_size_y=16) in;
layout(rgba8, binding=0) uniform readonly image2D srcImg;
layout(rgba8, binding=1) uniform writeonly image2D dstImg;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec4 c = imageLoad(srcImg, p);
    imageStore(dstImg, p, vec4(1.0 - c.rgb, 1.0));
}
)";

const char* blurShader = R"(
#version 460
layout(local_size_x=16, local_size_y=16) in;
layout(rgba8, binding=0) uniform readonly image2D srcImg;
layout(rgba8, binding=1) uniform writeonly image2D dstImg;

void main() {
    ivec2 p = ivec2(gl_GlobalInvocationID.xy);
    vec3 sum = vec3(0.0);
    for (int i=-1; i<=1; ++i) {
        for (int j=-1; j<=1; ++j) {
            sum += imageLoad(srcImg, p + ivec2(i,j)).rgb;
        }
    }
    sum /= 9.0;
    imageStore(dstImg, p, vec4(sum, 1.0));
}
)";

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
    GLFWwindow* window = glfwCreateWindow(800,600,"Fractal Effects",nullptr,nullptr);
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

    // Create two textures for ping-ponging
    GLuint tex1, tex2;
    glGenTextures(1,&tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA8,WIDTH,HEIGHT);

    glGenTextures(1,&tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA8,WIDTH,HEIGHT);

    // Compile shaders
    GLuint mandelbrotProg = createComputeShader(fractalMandelbrot);
    GLuint juliaProg = createComputeShader(fractalJulia);
    GLuint flameProg = createComputeShader(fractalFlame);
    GLuint grayscaleProg = createComputeShader(grayscaleShader);
    GLuint invertProg = createComputeShader(invertShader);
    GLuint blurProg = createComputeShader(blurShader);

    GLuint fullscreenProg = createShaderProgram(fullscreenVert, fullscreenFrag);

    GLuint quadVAO;
    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    enum FractalType {MANDELBROT, JULIA, FLAME};
    int currentFractal = 0;
    bool doInvert = false;
    bool doGrayscale = false;
    bool doBlur = false;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Fractal & Effects");
        ImGui::Combo("Fractal", &currentFractal, "Mandelbrot\0Julia\0Flame\0");
        ImGui::Checkbox("Invert", &doInvert);
        ImGui::Checkbox("Grayscale", &doGrayscale);
        ImGui::Checkbox("Blur", &doBlur);
        ImGui::End();

        // Step 1: Render fractal to tex1
        glBindImageTexture(0, tex1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        switch(currentFractal) {
            case MANDELBROT: dispatch(mandelbrotProg, WIDTH, HEIGHT); break;
            case JULIA: dispatch(juliaProg, WIDTH, HEIGHT); break;
            case FLAME: dispatch(flameProg, WIDTH, HEIGHT); break;
        }

        // Step 2: Apply blur if enabled
        if(doBlur) {
            glBindImageTexture(0, tex1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
            glBindImageTexture(1, tex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            dispatch(blurProg, WIDTH, HEIGHT);
            std::swap(tex1, tex2);
        }

        // Step 3: Apply grayscale if enabled
        if(doGrayscale) {
            glBindImageTexture(0, tex1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
            glBindImageTexture(1, tex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            dispatch(grayscaleProg, WIDTH, HEIGHT);
            std::swap(tex1, tex2);
        }

        // Step 4: Apply invert if enabled
        if(doInvert) {
            glBindImageTexture(0, tex1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
            glBindImageTexture(1, tex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            dispatch(invertProg, WIDTH, HEIGHT);
            std::swap(tex1, tex2);
        }

        // Render final texture to screen
        glViewport(0, 0, WIDTH, HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(fullscreenProg);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex1);
        glUniform1i(glGetUniformLocation(fullscreenProg, "tex"), 0);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
