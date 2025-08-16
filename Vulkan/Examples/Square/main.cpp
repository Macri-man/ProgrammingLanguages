// main.cpp
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

static const uint32_t WIDTH = 800;
static const uint32_t HEIGHT = 600;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

struct SwapSupport {
    VkSurfaceCapabilitiesKHR caps{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> modes;
};

struct UBO { float color[4]; };

// ---------- small helpers ----------
static std::vector<char> readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) throw std::runtime_error("failed to open file: " + path);
    size_t sz = (size_t)f.tellg();
    std::vector<char> buf(sz);
    f.seekg(0);
    f.read(buf.data(), sz);
    return buf;
}
static uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties mp{};
    vkGetPhysicalDeviceMemoryProperties(phys, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; ++i)
        if ((typeBits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    throw std::runtime_error("no suitable memory type");
}

class App {
public:
    void run() {
        initWindow();
        initVulkan();
        loop();
        vkDeviceWaitIdle(device);
        cleanup();
    }

private:
    // window
    GLFWwindow* window = nullptr;

    // core
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    // queues
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue  = VK_NULL_HANDLE;
    QueueFamilyIndices qfi{};

    // swapchain
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkExtent2D swapExtent{};
    std::vector<VkImage> swapImages;
    std::vector<VkImageView> swapViews;

    // pipeline
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    // framebuffers
    std::vector<VkFramebuffer> framebuffers;

    // commands & sync
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> cmdbufs;
    VkSemaphore semImageAvail = VK_NULL_HANDLE;
    VkSemaphore semRenderDone = VK_NULL_HANDLE;
    VkFence inFlight = VK_NULL_HANDLE;

    // uniform buffer (host visible/coherent)
    VkBuffer uboBuf = VK_NULL_HANDLE;
    VkDeviceMemory uboMem = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;

    // ---------- init ----------
    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Color Square", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createPipeline();
        createFramebuffers();
        createCommandPool();
        createUniformBuffer();
        createDescriptorPoolAndSet();
        createCommandBuffers();
        createSync();
    }

    // ---------- instance/surface ----------
    void createInstance() {
        VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        appInfo.pApplicationName = "Color Square";
        appInfo.apiVersion = VK_API_VERSION_1_2;

        uint32_t extCount = 0;
        const char** exts = glfwGetRequiredInstanceExtensions(&extCount);

        VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        ci.pApplicationInfo = &appInfo;
        ci.enabledExtensionCount = extCount;
        ci.ppEnabledExtensionNames = exts;

        if (vkCreateInstance(&ci, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("vkCreateInstance failed");
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
            throw std::runtime_error("glfwCreateWindowSurface failed");
    }

    // ---------- device/queues ----------
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pd) {
        QueueFamilyIndices r{};
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, nullptr);
        std::vector<VkQueueFamilyProperties> props(count);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &count, props.data());
        for (uint32_t i = 0; i < count; ++i) {
            if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) r.graphicsFamily = i;
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &presentSupport);
            if (presentSupport) r.presentFamily = i;
            if (r.isComplete()) break;
        }
        return r;
    }

    bool deviceSuitable(VkPhysicalDevice pd) {
        auto idx = findQueueFamilies(pd);
        if (!idx.isComplete()) return false;
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(pd, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> exts(extCount);
        vkEnumerateDeviceExtensionProperties(pd, nullptr, &extCount, exts.data());
        bool hasSwapchain = false;
        for (auto& e : exts) if (std::string(e.extensionName) == std::string(VK_KHR_SWAPCHAIN_EXTENSION_NAME)) hasSwapchain = true;
        if (!hasSwapchain) return false;
        // also require at least one supported format and present mode
        auto sup = querySwapSupport(pd);
        return !sup.formats.empty() && !sup.modes.empty();
    }

    void pickPhysicalDevice() {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        if (count == 0) throw std::runtime_error("no Vulkan GPU");
        std::vector<VkPhysicalDevice> devs(count);
        vkEnumeratePhysicalDevices(instance, &count, devs.data());
        for (auto d : devs) {
            if (deviceSuitable(d)) { physicalDevice = d; break; }
        }
        if (physicalDevice == VK_NULL_HANDLE) throw std::runtime_error("no suitable GPU");
        qfi = findQueueFamilies(physicalDevice);
    }

    void createDevice() {
        std::set<uint32_t> uniques = { *qfi.graphicsFamily, *qfi.presentFamily };
        float prio = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> qcis;
        for (auto idx : uniques) {
            VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            qci.queueFamilyIndex = idx;
            qci.queueCount = 1;
            qci.pQueuePriorities = &prio;
            qcis.push_back(qci);
        }

        const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkPhysicalDeviceFeatures feats{};
        VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        dci.queueCreateInfoCount = (uint32_t)qcis.size();
        dci.pQueueCreateInfos = qcis.data();
        dci.enabledExtensionCount = 1;
        dci.ppEnabledExtensionNames = devExts;
        dci.pEnabledFeatures = &feats;

        if (vkCreateDevice(physicalDevice, &dci, nullptr, &device) != VK_SUCCESS)
            throw std::runtime_error("vkCreateDevice failed");

        vkGetDeviceQueue(device, *qfi.graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, *qfi.presentFamily, 0, &presentQueue);
    }

    // ---------- swapchain ----------
    SwapSupport querySwapSupport(VkPhysicalDevice pd) {
        SwapSupport s{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &s.caps);
        uint32_t count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &count, nullptr);
        s.formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &count, s.formats.data());
        vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &count, nullptr);
        s.modes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pd, surface, &count, s.modes.data());
        return s;
    }

    VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
        for (auto& f : formats) {
            if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return f;
        }
        return formats[0];
    }
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
        for (auto m : modes) if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& caps) {
        if (caps.currentExtent.width != UINT32_MAX) return caps.currentExtent;
        int w, h; glfwGetFramebufferSize(window, &w, &h);
        VkExtent2D e{ (uint32_t)w, (uint32_t)h };
        e.width  = std::max(caps.minImageExtent.width,  std::min(caps.maxImageExtent.width,  e.width));
        e.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, e.height));
        return e;
    }

    void createSwapchain() {
        auto sup = querySwapSupport(physicalDevice);
        auto fmt = chooseFormat(sup.formats);
        auto pm  = choosePresentMode(sup.modes);
        auto ext = chooseExtent(sup.caps);

        uint32_t imageCount = sup.caps.minImageCount + 1;
        if (sup.caps.maxImageCount > 0 && imageCount > sup.caps.maxImageCount) imageCount = sup.caps.maxImageCount;

        VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        ci.surface = surface;
        ci.minImageCount = imageCount;
        ci.imageFormat = fmt.format;
        ci.imageColorSpace = fmt.colorSpace;
        ci.imageExtent = ext;
        ci.imageArrayLayers = 1;
        ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t qidx[] = { *qfi.graphicsFamily, *qfi.presentFamily };
        if (*qfi.graphicsFamily != *qfi.presentFamily) {
            ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            ci.queueFamilyIndexCount = 2;
            ci.pQueueFamilyIndices = qidx;
        } else {
            ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        ci.preTransform = sup.caps.currentTransform;
        ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        ci.presentMode = pm;
        ci.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(device, &ci, nullptr, &swapchain) != VK_SUCCESS)
            throw std::runtime_error("vkCreateSwapchainKHR failed");

        uint32_t cnt = 0;
        vkGetSwapchainImagesKHR(device, swapchain, &cnt, nullptr);
        swapImages.resize(cnt);
        vkGetSwapchainImagesKHR(device, swapchain, &cnt, swapImages.data());

        swapFormat = fmt.format;
        swapExtent = ext;
    }

    void createImageViews() {
        swapViews.resize(swapImages.size());
        for (size_t i = 0; i < swapImages.size(); ++i) {
            VkImageViewCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            ci.image = swapImages[i];
            ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            ci.format = swapFormat;
            ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            ci.subresourceRange.levelCount = 1;
            ci.subresourceRange.layerCount = 1;
            if (vkCreateImageView(device, &ci, nullptr, &swapViews[i]) != VK_SUCCESS)
                throw std::runtime_error("vkCreateImageView failed");
        }
    }

    // ---------- render pass / pipeline ----------
    void createRenderPass() {
        VkAttachmentDescription color{};
        color.format = swapFormat;
        color.samples = VK_SAMPLE_COUNT_1_BIT;
        color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub{};
        sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments = &colorRef;

        VkSubpassDependency dep{};
        dep.srcSubpass = VK_SUBPASS_EXTERNAL;
        dep.dstSubpass = 0;
        dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        rp.attachmentCount = 1;
        rp.pAttachments = &color;
        rp.subpassCount = 1;
        rp.pSubpasses = &sub;
        rp.dependencyCount = 1;
        rp.pDependencies = &dep;

        if (vkCreateRenderPass(device, &rp, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("vkCreateRenderPass failed");
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding b{};
        b.binding = 0;
        b.descriptorCount = 1;
        b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo ci{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        ci.bindingCount = 1;
        ci.pBindings = &b;
        if (vkCreateDescriptorSetLayout(device, &ci, nullptr, &descSetLayout) != VK_SUCCESS)
            throw std::runtime_error("vkCreateDescriptorSetLayout failed");
    }

    VkShaderModule makeModule(const char* path) {
        auto code = readFile(path);
        VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        ci.codeSize = code.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule m{};
        if (vkCreateShaderModule(device, &ci, nullptr, &m) != VK_SUCCESS)
            throw std::runtime_error(std::string("shader create failed: ") + path);
        return m;
    }

    void createPipeline() {
        // shaders
        VkShaderModule vert = makeModule("Shaders/vertex.spv");
        VkShaderModule frag = makeModule("Shaders/fragment.spv");

        VkPipelineShaderStageCreateInfo vs{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        vs.stage = VK_SHADER_STAGE_VERTEX_BIT; vs.module = vert; vs.pName = "main";
        VkPipelineShaderStageCreateInfo fs{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        fs.stage = VK_SHADER_STAGE_FRAGMENT_BIT; fs.module = frag; fs.pName = "main";
        VkPipelineShaderStageCreateInfo stages[] = {vs, fs};

        // no vertex buffers (using gl_VertexIndex)
        VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        VkViewport vp{};
        vp.width  = (float)swapExtent.width;
        vp.height = (float)swapExtent.height;
        vp.minDepth = 0.f; vp.maxDepth = 1.f;

        VkRect2D sc{}; sc.extent = swapExtent;

        VkPipelineViewportStateCreateInfo vps{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        vps.viewportCount = 1; vps.pViewports = &vp;
        vps.scissorCount = 1;  vps.pScissors = &sc;

        VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.cullMode = VK_CULL_MODE_BACK_BIT;
        rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rs.lineWidth = 1.f;

        VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState cba{};
        cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
        VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
        cb.attachmentCount = 1; cb.pAttachments = &cba;

        VkPipelineLayoutCreateInfo pl{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pl.setLayoutCount = 1;
        pl.pSetLayouts = &descSetLayout;
        if (vkCreatePipelineLayout(device, &pl, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("vkCreatePipelineLayout failed");

        VkGraphicsPipelineCreateInfo gp{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
        gp.stageCount = 2; gp.pStages = stages;
        gp.pVertexInputState = &vi; gp.pInputAssemblyState = &ia;
        gp.pViewportState = &vps; gp.pRasterizationState = &rs;
        gp.pMultisampleState = &ms; gp.pColorBlendState = &cb;
        gp.layout = pipelineLayout; gp.renderPass = renderPass; gp.subpass = 0;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gp, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("vkCreateGraphicsPipelines failed");

        vkDestroyShaderModule(device, frag, nullptr);
        vkDestroyShaderModule(device, vert, nullptr);
    }

    // ---------- framebuffers ----------
    void createFramebuffers() {
        framebuffers.resize(swapViews.size());
        for (size_t i = 0; i < swapViews.size(); ++i) {
            VkImageView att[] = { swapViews[i] };
            VkFramebufferCreateInfo ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            ci.renderPass = renderPass;
            ci.attachmentCount = 1;
            ci.pAttachments = att;
            ci.width = swapExtent.width;
            ci.height = swapExtent.height;
            ci.layers = 1;
            if (vkCreateFramebuffer(device, &ci, nullptr, &framebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("vkCreateFramebuffer failed");
        }
    }

    // ---------- cmd pool/buffers ----------
    void createCommandPool() {
        VkCommandPoolCreateInfo ci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        ci.queueFamilyIndex = *qfi.graphicsFamily;
        ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(device, &ci, nullptr, &cmdPool) != VK_SUCCESS)
            throw std::runtime_error("vkCreateCommandPool failed");
    }

    // ---------- uniforms / descriptors ----------
    void createUniformBuffer() {
        VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bi.size = sizeof(UBO);
        bi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device, &bi, nullptr, &uboBuf) != VK_SUCCESS)
            throw std::runtime_error("vkCreateBuffer(ubo) failed");

        VkMemoryRequirements mr{};
        vkGetBufferMemoryRequirements(device, uboBuf, &mr);

        VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        ai.allocationSize = mr.size;
        ai.memoryTypeIndex = findMemoryType(
            physicalDevice, mr.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &ai, nullptr, &uboMem) != VK_SUCCESS)
            throw std::runtime_error("vkAllocateMemory(ubo) failed");

        vkBindBufferMemory(device, uboBuf, uboMem, 0);
    }

    void createDescriptorPoolAndSet() {
        VkDescriptorPoolSize ps{};
        ps.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ps.descriptorCount = 1;

        VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        pci.maxSets = 1;
        pci.poolSizeCount = 1;
        pci.pPoolSizes = &ps;
        if (vkCreateDescriptorPool(device, &pci, nullptr, &descPool) != VK_SUCCESS)
            throw std::runtime_error("vkCreateDescriptorPool failed");

        VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        ai.descriptorPool = descPool;
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &descSetLayout;
        if (vkAllocateDescriptorSets(device, &ai, &descSet) != VK_SUCCESS)
            throw std::runtime_error("vkAllocateDescriptorSets failed");

        VkDescriptorBufferInfo dbi{};
        dbi.buffer = uboBuf; dbi.offset = 0; dbi.range = sizeof(UBO);

        VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        w.dstSet = descSet;
        w.dstBinding = 0;
        w.descriptorCount = 1;
        w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        w.pBufferInfo = &dbi;
        vkUpdateDescriptorSets(device, 1, &w, 0, nullptr);
    }

    // ---------- record commands ----------
    void createCommandBuffers() {
        cmdbufs.resize(framebuffers.size());
        VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        ai.commandPool = cmdPool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = (uint32_t)cmdbufs.size();
        if (vkAllocateCommandBuffers(device, &ai, cmdbufs.data()) != VK_SUCCESS)
            throw std::runtime_error("vkAllocateCommandBuffers failed");

        for (size_t i = 0; i < cmdbufs.size(); ++i) {
            VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            vkBeginCommandBuffer(cmdbufs[i], &bi);

            VkClearValue clear{};
            clear.color = { {0.05f, 0.05f, 0.08f, 1.0f} };

            VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
            rp.renderPass = renderPass;
            rp.framebuffer = framebuffers[i];
            rp.renderArea.extent = swapExtent;
            rp.clearValueCount = 1;
            rp.pClearValues = &clear;
            vkCmdBeginRenderPass(cmdbufs[i], &rp, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(cmdbufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindDescriptorSets(cmdbufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, 0, 1, &descSet, 0, nullptr);

            // draw the square (two triangles) using triangle strip with 4 verts
            vkCmdDraw(cmdbufs[i], 4, 1, 0, 0);

            vkCmdEndRenderPass(cmdbufs[i]);
            vkEndCommandBuffer(cmdbufs[i]);
        }
    }

    // ---------- sync ----------
    void createSync() {
        VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateSemaphore(device, &si, nullptr, &semImageAvail) != VK_SUCCESS ||
            vkCreateSemaphore(device, &si, nullptr, &semRenderDone) != VK_SUCCESS ||
            vkCreateFence(device, &fi, nullptr, &inFlight) != VK_SUCCESS)
            throw std::runtime_error("sync objects creation failed");
    }

    // ---------- main loop ----------
    void loop() {
        auto start = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // update UBO color
            auto now = std::chrono::high_resolution_clock::now();
            float t = std::chrono::duration<float>(now - start).count();
            UBO u{};
            u.color[0] = (std::sin(t * 2.0f) + 1.f) * 0.5f;
            u.color[1] = (std::sin(t * 0.7f) + 1.f) * 0.5f;
            u.color[2] = (std::sin(t * 1.3f) + 1.f) * 0.5f;
            u.color[3] = 1.f;
            void* mapped = nullptr;
            vkMapMemory(device, uboMem, 0, sizeof(UBO), 0, &mapped);
            std::memcpy(mapped, &u, sizeof(UBO));
            vkUnmapMemory(device, uboMem);

            drawFrame();
        }
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlight, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlight);

        uint32_t imageIndex = 0;
        VkResult acq = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semImageAvail, VK_NULL_HANDLE, &imageIndex);
        if (acq != VK_SUCCESS) throw std::runtime_error("vkAcquireNextImageKHR failed");

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &semImageAvail;
        submit.pWaitDstStageMask = waitStages;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdbufs[imageIndex];
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &semRenderDone;

        if (vkQueueSubmit(graphicsQueue, 1, &submit, inFlight) != VK_SUCCESS)
            throw std::runtime_error("vkQueueSubmit failed");

        VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present.waitSemaphoreCount = 1;
        present.pWaitSemaphores = &semRenderDone;
        present.swapchainCount = 1;
        present.pSwapchains = &swapchain;
        present.pImageIndices = &imageIndex;

        if (vkQueuePresentKHR(presentQueue, &present) != VK_SUCCESS)
            throw std::runtime_error("vkQueuePresentKHR failed");
    }

    // ---------- cleanup ----------
    void cleanup() {
        vkDestroyFence(device, inFlight, nullptr);
        vkDestroySemaphore(device, semRenderDone, nullptr);
        vkDestroySemaphore(device, semImageAvail, nullptr);

        vkDestroyDescriptorPool(device, descPool, nullptr);
        vkDestroyBuffer(device, uboBuf, nullptr);
        vkFreeMemory(device, uboMem, nullptr);

        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, descSetLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto fb : framebuffers) vkDestroyFramebuffer(device, fb, nullptr);
        for (auto v : swapViews) vkDestroyImageView(device, v, nullptr);

        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroyCommandPool(device, cmdPool, nullptr);

        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    try { App().run(); }
    catch (const std::exception& e) { std::cerr << "Error: " << e.what() << "\n"; return 1; }
    return 0;
}
