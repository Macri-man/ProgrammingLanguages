#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include <stb_image.h>
#include <iostream>

// Settings
const unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH/2.0f, lastY = SCR_HEIGHT/2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window,double xpos,double ypos);
void scroll_callback(GLFWwindow* window,double xoffset,double yoffset);
void processInput(GLFWwindow* window);

// Cubemap loader
unsigned int loadCubemap(std::vector<std::string> faces);

int main() {
    // GLFW init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"Reflection Demo",NULL,NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
    glfwSetCursorPosCallback(window,mouse_callback);
    glfwSetScrollCallback(window,scroll_callback);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    // Shaders
    Shader reflectShader("shaders/reflection.vert","shaders/reflection.frag");
    Shader skyboxShader("shaders/skybox.vert","shaders/skybox.frag");

    // Load model
    Model ourModel("models/your_model.obj");

    // Skybox setup
    float skyboxVertices[] = {
        // positions (36 vertices making a cube)
        -1.0f,  1.0f, -1.0f, /* ... all skybox cube coordinates ... */
    };
    unsigned int skyVAO, skyVBO;
    glGenVertexArrays(1,&skyVAO);
    glGenBuffers(1,&skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER,skyVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(skyboxVertices),&skyboxVertices,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);

    // Cubemap textures
    std::vector<std::string> faces = {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw reflective model
        reflectShader.use();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH/SCR_HEIGHT,0.1f,100.0f);
        glm::mat4 model = glm::mat4(1.0f);
        reflectShader.setMat4("view", view);
        reflectShader.setMat4("projection", proj);
        reflectShader.setMat4("model", model);
        reflectShader.setVec3("cameraPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        reflectShader.setInt("skybox", 0);
        ourModel.Draw(reflectShader);

        // Draw skybox (last)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", viewNoTrans);
        skyboxShader.setMat4("projection", proj);
        glBindVertexArray(skyVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES,0,36);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
