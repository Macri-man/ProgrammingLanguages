// main.cpp
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <stb_image.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <random>
#include <string>
#include <filesystem>
#include <optional>
#include <array>
#include <cstdint>
#include <algorithm>

VkInstance gInstance;
VkPhysicalDevice gPhysicalDevice;
VkDevice gDevice;
VkQueue gGraphicsQueue;
VkSurfaceKHR gSurface;
VkSwapchainKHR gSwapchain;
VkFormat gSwapchainImageFormat;
VkExtent2D gSwapchainExtent;
std::vector<VkImage> gSwapchainImages;
std::vector<VkImageView> gSwapchainImageViews;
VkRenderPass gRenderPass;
std::vector<VkFramebuffer> gFramebuffers;
VkCommandPool gCommandPool;
std::vector<VkCommandBuffer> gCommandBuffers;
VkDescriptorPool gImGuiDescriptorPool;
GLFWwindow* gWindow;
VkSemaphore gImageAvailableSemaphore, gRenderFinishedSemaphore;
VkFence gInFlightFence;

// Texture
ImTextureID gTextureID;
VkImage gTextureImage;
VkDeviceMemory gTextureMemory;
VkImageView gTextureView;
VkSampler gTextureSampler;

// ----------------------
// Random PNG loader
// ----------------------
std::string GetRandomTexture(const std::string& folder) {
    std::vector<std::string> pngFiles;
    for (const auto& p : std::filesystem::directory_iterator(folder)) {
        if (p.path().extension() == ".png") {
            pngFiles.push_back(p.path().string());
        }
    }
    if (pngFiles.empty())
        throw std::runtime_error("No PNG files found in folder: " + folder);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, pngFiles.size() - 1);
    return pngFiles[dis(gen)];
}

// ----------------------
// Vulkan helpers
// ----------------------
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    int i = 0;
    for (const auto& qf : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, gSurface, &presentSupport);
        if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
        if (presentSupport) indices.presentFamily = i;
        if (indices.isComplete()) break;
        i++;
    }
    return indices;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& available : availableFormats) {
        if (available.format == VK_FORMAT_B8G8R8A8_UNORM &&
            available.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return available;
    }
    return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes) {
    for (const auto& mode : availableModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) return capabilities.currentExtent;
    VkExtent2D actualExtent = {800, 600};
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
}

// ----------------------
// Vulkan init
// ----------------------
void InitVulkan() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan ImGui Example";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    if (vkCreateInstance(&createInfo, nullptr, &gInstance) != VK_SUCCESS)
        throw std::runtime_error("failed to create instance");

    if (glfwCreateWindowSurface(gInstance, gWindow, nullptr, &gSurface) != VK_SUCCESS)
        throw std::runtime_error("failed to create surface");

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(gInstance, &deviceCount, nullptr);
    if (deviceCount == 0) throw std::runtime_error("no GPUs with Vulkan support");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(gInstance, &deviceCount, devices.data());
    gPhysicalDevice = devices[0];

    QueueFamilyIndices indices = FindQueueFamilies(gPhysicalDevice);
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    if (vkCreateDevice(gPhysicalDevice, &deviceInfo, nullptr, &gDevice) != VK_SUCCESS)
        throw std::runtime_error("failed to create device");

    vkGetDeviceQueue(gDevice, indices.graphicsFamily.value(), 0, &gGraphicsQueue);

    // Swapchain
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gPhysicalDevice, gSurface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gPhysicalDevice, gSurface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gPhysicalDevice, gSurface, &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gPhysicalDevice, gSurface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gPhysicalDevice, gSurface, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(presentModes);
    gSwapchainExtent = ChooseSwapExtent(capabilities);
    gSwapchainImageFormat = surfaceFormat.format;

    VkSwapchainCreateInfoKHR scInfo{};
    scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scInfo.surface = gSurface;
    scInfo.minImageCount = capabilities.minImageCount + 1;
    scInfo.imageFormat = gSwapchainImageFormat;
    scInfo.imageColorSpace = surfaceFormat.colorSpace;
    scInfo.imageExtent = gSwapchainExtent;
    scInfo.imageArrayLayers = 1;
    scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    scInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    scInfo.preTransform = capabilities.currentTransform;
    scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scInfo.presentMode = presentMode;
    scInfo.clipped = VK_TRUE;
    scInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(gDevice, &scInfo, nullptr, &gSwapchain) != VK_SUCCESS)
        throw std::runtime_error("failed to create swapchain");

    uint32_t imageCount;
    vkGetSwapchainImagesKHR(gDevice, gSwapchain, &imageCount, nullptr);
    gSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(gDevice, gSwapchain, &imageCount, gSwapchainImages.data());

    gSwapchainImageViews.resize(gSwapchainImages.size());
    for (size_t i = 0; i < gSwapchainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = gSwapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = gSwapchainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(gDevice, &viewInfo, nullptr, &gSwapchainImageViews[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create image view");
    }

    // Render pass
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = gSwapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments = &colorAttachment;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(gDevice, &rpInfo, nullptr, &gRenderPass) != VK_SUCCESS)
        throw std::runtime_error("failed to create render pass");

    gFramebuffers.resize(gSwapchainImageViews.size());
    for (size_t i = 0; i < gSwapchainImageViews.size(); i++) {
        VkImageView attachments[] = { gSwapchainImageViews[i] };
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = gRenderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = attachments;
        fbInfo.width = gSwapchainExtent.width;
        fbInfo.height = gSwapchainExtent.height;
        fbInfo.layers = 1;
        if (vkCreateFramebuffer(gDevice, &fbInfo, nullptr, &gFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer");
    }

    // Command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = FindQueueFamilies(gPhysicalDevice).graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(gDevice, &poolInfo, nullptr, &gCommandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool");

    // Command buffers
    gCommandBuffers.resize(gFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)gCommandBuffers.size();
    if (vkAllocateCommandBuffers(gDevice, &allocInfo, gCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers");

    // Sync objects
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateSemaphore(gDevice, &semInfo, nullptr, &gImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(gDevice, &semInfo, nullptr, &gRenderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(gDevice, &fenceInfo, nullptr, &gInFlightFence) != VK_SUCCESS)
        throw std::runtime_error("failed to create sync objects");

    // ImGui descriptor pool
    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 } };
    VkDescriptorPoolCreateInfo dpInfo{};
    dpInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpInfo.poolSizeCount = 1;
    dpInfo.pPoolSizes = pool_sizes;
    dpInfo.maxSets = 1000;
    if (vkCreateDescriptorPool(gDevice, &dpInfo, nullptr, &gImGuiDescriptorPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create ImGui descriptor pool");
}

// ----------------------
// ImGui init
// ----------------------
void InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(gWindow, true);
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = gInstance;
    initInfo.PhysicalDevice = gPhysicalDevice;
    initInfo.Device = gDevice;
    initInfo.Queue = gGraphicsQueue;
    initInfo.DescriptorPool = gImGuiDescriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = (uint32_t)gSwapchainImages.size();
    ImGui_ImplVulkan_Init(&initInfo, gRenderPass);
}

// ----------------------
// Load texture
// ----------------------
void LoadTexture() {
    std::string randomTexture = GetRandomTexture("Textures");
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(randomTexture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels) throw std::runtime_error("failed to load texture");

    VkDeviceSize imageSize = texWidth * texHeight * 4;
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
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(gDevice, &imageInfo, nullptr, &gTextureImage) != VK_SUCCESS)
        throw std::runtime_error("failed to create image");

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(gDevice, gTextureImage, &memReq);
    VkMemoryAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = memReq.size;
    alloc.memoryTypeIndex = 0; // simplified
    if (vkAllocateMemory(gDevice, &alloc, nullptr, &gTextureMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory");
    vkBindImageMemory(gDevice, gTextureImage, gTextureMemory, 0);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = gTextureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(gDevice, &viewInfo, nullptr, &gTextureView) != VK_SUCCESS)
        throw std::runtime_error("failed to create image view");

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    if (vkCreateSampler(gDevice, &samplerInfo, nullptr, &gTextureSampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create sampler");

    ImGui_ImplVulkan_CreateTexture(gTextureSampler, gTextureView,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   &gTextureID);

    stbi_image_free(pixels);
}

// ----------------------
// Record ImGui commands
// ----------------------
void RecordCommandBuffer(VkCommandBuffer cmd, VkFramebuffer fb) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = gRenderPass;
    rpBegin.framebuffer = fb;
    rpBegin.renderArea.offset = {0,0};
    rpBegin.renderArea.extent = gSwapchainExtent;
    VkClearValue clearColor = {0.1f, 0.1f, 0.1f, 1.0f};
    rpBegin.clearValueCount = 1;
    rpBegin.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    vkCmdEndRenderPass(cmd);

    vkEndCommandBuffer(cmd);
}

// ----------------------
// Main loop
// ----------------------
void MainLoop() {
    while (!glfwWindowShouldClose(gWindow)) {
        glfwPollEvents();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Random Texture Example");
        ImGui::Text("Hello Vulkan + ImGui!");
        ImGui::Image(gTextureID, ImVec2(256,256));
        ImGui::End();

        ImGui::Render();

        vkWaitForFences(gDevice, 1, &gInFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(gDevice, 1, &gInFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(gDevice, gSwapchain, UINT64_MAX, gImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        RecordCommandBuffer(gCommandBuffers[imageIndex], gFramebuffers[imageIndex]);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {gImageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &gCommandBuffers[imageIndex];
        VkSemaphore signalSemaphores[] = {gRenderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(gGraphicsQueue, 1, &submitInfo, gInFlightFence) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapchains[] = {gSwapchain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        vkQueuePresentKHR(gGraphicsQueue, &presentInfo);
    }
}

// ----------------------
// Main
// ----------------------
int main() {
    try {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        gWindow = glfwCreateWindow(800, 600, "Vulkan ImGui Random Texture", nullptr, nullptr);

        InitVulkan();
        InitImGui();
        LoadTexture();

        MainLoop();

        vkDeviceWaitIdle(gDevice);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
