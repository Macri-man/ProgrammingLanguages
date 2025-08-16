#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <iostream>

VkInstance instance;
VkDevice device;
VkPhysicalDevice physicalDevice;
VkQueue graphicsQueue;
VkCommandPool commandPool;
VkDescriptorPool imguiDescriptorPool;
GLFWwindow* window;

ImTextureID myTextureID;
VkImage myTextureImage;
VkDeviceMemory myTextureMemory;
VkImageView myTextureView;
VkSampler myTextureSampler;

// --- Vulkan helpers ---
VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");
    return shaderModule;
}

// --- Load PNG texture using stb_image ---
void LoadTextureFromFile(const char* filename) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) throw std::runtime_error("Failed to load texture image");

    VkDeviceSize imageSize = texWidth * texHeight * 4;

    // Create VkImage (simplified, real usage should use staging buffer)
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &myTextureImage) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, myTextureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // Memory type 0 (simplified, in production query correct type)
    allocInfo.memoryTypeIndex = 0;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &myTextureMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate image memory");

    vkBindImageMemory(device, myTextureImage, myTextureMemory, 0);

    // Create ImageView
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = myTextureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &myTextureView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view");

    // Create sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkCreateSampler(device, &samplerInfo, nullptr, &myTextureSampler);

    // Upload to ImGui
    ImGui_ImplVulkan_CreateTexture(myTextureSampler, myTextureView,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   &myTextureID);

    stbi_image_free(pixels);
}

// --- Main ---
int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "Vulkan + ImGui Texture", nullptr, nullptr);

    // Initialize Vulkan instance, device, swapchain, command pool...
    // (omitted for brevity: you can use glfwCreateWindowSurface, vkCreateInstance, pick physical device, etc.)

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Create Vulkan descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = 1000;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiDescriptorPool);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE); // render pass placeholder

    LoadTextureFromFile("texture.png");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Texture Example");
        ImGui::Text("Hello Vulkan + ImGui!");
        ImGui::Image(myTextureID, ImVec2(256, 256));
        ImGui::End();

        ImGui::Render();
        // Submit command buffer with ImGui draw data (omitted, you need a Vulkan render pass & command buffer)
    }

    vkDeviceWaitIdle(device);
    vkDestroySampler(device, myTextureSampler, nullptr);
    vkDestroyImageView(device, myTextureView, nullptr);
    vkDestroyImage(device, myTextureImage, nullptr);
    vkFreeMemory(device, myTextureMemory, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}
