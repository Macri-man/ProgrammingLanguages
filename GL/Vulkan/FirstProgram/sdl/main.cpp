//g++ vulkan_example.cpp -o vulkan_example -I/path/to/vulkan/include -I/path/to/SDL2/include -L/path/to/vulkan/lib -L/path/to/SDL2/lib -lvulkan -lSDL2
//./vulkan_example

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

class VulkanExample {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    SDL_Window* window;
    VkInstance instance;

    void initWindow() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("Failed to initialize SDL");
        }

        window = SDL_CreateWindow("Vulkan Example", 
                                   SDL_WINDOWPOS_CENTERED, 
                                   SDL_WINDOWPOS_CENTERED, 
                                   800, 600, 
                                   SDL_WINDOW_VULKAN);
        if (!window) {
            throw std::runtime_error("Failed to create SDL window");
        }
    }

    void initVulkan() {
        createInstance();
    }

    void createInstance() {
        // Application information
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Vulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Instance creation info
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Create the Vulkan instance
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance");
        }
    }

    void mainLoop() {
        bool running = true;
        SDL_Event event;
        while (running) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
            }
        }
    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr); // Destroy Vulkan instance
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
};

int main() {
    VulkanExample example;
    try {
        example.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
