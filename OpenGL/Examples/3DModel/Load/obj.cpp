#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>

// Define vertex, texture, and face structures
struct Vertex {
    float x, y, z;
};

struct Texture {
    float u, v;
};

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> texCoordIndices;
};

// Function to load .obj file
void loadOBJ(const std::string& path, std::vector<Vertex>& vertices, std::vector<Texture>& texCoords, std::vector<Face>& faces) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            Vertex vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (prefix == "vt") {
            Texture texCoord;
            ss >> texCoord.u >> texCoord.v;
            texCoords.push_back(texCoord);
        } else if (prefix == "f") {
            Face face;
            std::string vertexData;
            while (ss >> vertexData) {
                std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
                std::istringstream vertexStream(vertexData);
                int vertexIndex, texCoordIndex;
                vertexStream >> vertexIndex;
                vertexStream >> texCoordIndex;
                face.vertexIndices.push_back(vertexIndex - 1);
                face.texCoordIndices.push_back(texCoordIndex - 1);
            }
            faces.push_back(face);
        }
    }
}

// OpenGL variables
GLuint VAO, VBO, EBO;
std::vector<float> vertexData;
std::vector<unsigned int> indices;

// Function to set up model buffers
void setupModel(const std::vector<Vertex>& vertices, const std::vector<Texture>& texCoords, const std::vector<Face>& faces) {
    // Prepare data for VBO
    for (const auto& face : faces) {
        for (size_t i = 0; i < face.vertexIndices.size(); ++i) {
            const Vertex& vertex = vertices[face.vertexIndices[i]];
            const Texture& texCoord = texCoords[face.texCoordIndices[i]];
            vertexData.push_back(vertex.x);
            vertexData.push_back(vertex.y);
            vertexData.push_back(vertex.z);
            vertexData.push_back(texCoord.u);
            vertexData.push_back(texCoord.v);
        }
        indices.insert(indices.end(), face.vertexIndices.begin(), face.vertexIndices.end());
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); // Texture Coordinate
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// Function to compile and link shaders
GLuint createShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D texture1;
        void main() {
            FragColor = texture(texture1, TexCoord);
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Main function
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Model Loading", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Load model
    std::vector<Vertex> vertices;
    std::vector<Texture> texCoords;
    std::vector<Face> faces;
    loadOBJ("path/to/your/model.obj", vertices, texCoords, faces);

    // Set up model
    setupModel(vertices, texCoords, faces);

    // Create and use shader program
    GLuint shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        // Set uniform matrices (model, view, projection)
        // You will need to set these matrices based on your camera and object transformation

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
