#include <glad/gl.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <string>

// Your shader sources (unchanged)
const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D texture1;
void main() {
    FragColor = texture(texture1, TexCoord);
}
)";

GLuint CompileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, nullptr, info);
        std::cerr << "Shader compilation failed: " << info << std::endl;
    }
    return shader;
}

GLuint LoadTexture(const char* path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return 0;
    }

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Set wrap and filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "Texture Example with ImGui", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Setup ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Define vertices and UVs
    float vertices[] = {
        // positions        // tex coords
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f, 
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f  
    };
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Buffers
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);  
    glGenBuffers(1, &VBO);  
    glGenBuffers(1, &EBO);  

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);  

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  
    glEnableVertexAttribArray(0);  

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));  
    glEnableVertexAttribArray(1);  

    // Shader program
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Load multiple textures
    std::vector<std::string> textureFiles = {
        "flame_texture_color.png",
        "flame_texture.png",
        "flame_texture_color2.png"
    };

    std::vector<GLuint> textures;
    for (auto& file : textureFiles) {
        GLuint tex = LoadTexture(file.c_str());
        if (tex != 0)
            textures.push_back(tex);
    }
    if (textures.empty()) {
        std::cerr << "No textures loaded. Exiting.\n";
        return -1;
    }

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0); // GL_TEXTURE0

    int currentTextureIndex = 0;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window for texture selection
        ImGui::Begin("Texture Selector");
        if (ImGui::BeginCombo("Select Texture", textureFiles[currentTextureIndex].c_str())) {
            for (int n = 0; n < (int)textureFiles.size(); n++) {
                bool is_selected = (currentTextureIndex == n);
                if (ImGui::Selectable(textureFiles[n].c_str(), is_selected)) {
                    currentTextureIndex = n;
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::End();

        // Rendering
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[currentTextureIndex]);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        // Render ImGui on top
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup GL resources
    for (auto tex : textures)
        glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &VAO);  
    glDeleteBuffers(1, &VBO);  
    glDeleteBuffers(1, &EBO);  
    glDeleteProgram(shaderProgram);  
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
