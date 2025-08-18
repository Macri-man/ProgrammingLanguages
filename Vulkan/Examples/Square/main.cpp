#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <cstring>

struct Vertex { float pos[2]; };
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}}, {{0.5f, -0.5f}},
    {{0.5f, 0.5f}}, {{-0.5f, 0.5f}}
};
const std::vector<uint16_t> indices = {0,1,2,2,3,0};

struct UniformBufferObject { float color[4]; };

class SquareApp {
public:
    void run() { initWindow(); initVulkan(); mainLoop(); cleanup(); }

private:
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue, presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    std::vector<VkImageView> swapchainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VkBuffer vertexBuffer; VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer; VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    // ---------------- Utilities ----------------
    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename,std::ios::ate|std::ios::binary);
        if(!file.is_open()) throw std::runtime_error("failed to open file");
        size_t size=file.tellg();
        std::vector<char> buffer(size);
        file.seekg(0);
        file.read(buffer.data(),size);
        file.close();
        return buffer;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize=code.size();
        info.pCode=reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule module;
        if(vkCreateShaderModule(device,&info,nullptr,&module)!=VK_SUCCESS)
            throw std::runtime_error("failed to create shader module");
        return module;
    }

    // ---------------- Initialization ----------------
    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
        window = glfwCreateWindow(800,600,"Vulkan Square",nullptr,nullptr);
    }

    void initVulkan() {
        createInstance();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName="Color Square";
        appInfo.applicationVersion=VK_MAKE_VERSION(1,0,0);
        appInfo.pEngineName="No Engine";
        appInfo.engineVersion=VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion=VK_API_VERSION_1_0;

        VkInstanceCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo=&appInfo;

        uint32_t extCount=0;
        const char** ext=glfwGetRequiredInstanceExtensions(&extCount);
        info.enabledExtensionCount=extCount;
        info.ppEnabledExtensionNames=ext;

        if(vkCreateInstance(&info,nullptr,&instance)!=VK_SUCCESS)
            throw std::runtime_error("failed to create instance");
    }

    void createSurface() {
        if(glfwCreateWindowSurface(instance,window,nullptr,&surface)!=VK_SUCCESS)
            throw std::runtime_error("failed to create window surface");
    }

    void pickPhysicalDevice() {
        uint32_t count=0; vkEnumeratePhysicalDevices(instance,&count,nullptr);
        if(count==0) throw std::runtime_error("no GPU");
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance,&count,devices.data());
        physicalDevice=devices[0];
    }

    void createLogicalDevice() {
        uint32_t queueCount=0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueCount,nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueCount,queues.data());

        int graphicsFamily=-1, presentFamily=-1;
        for(int i=0;i<queueCount;i++){
            if(queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsFamily=i;
            VkBool32 present=false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,i,surface,&present);
            if(present) presentFamily=i;
        }

        float priority=1.0f;
        std::vector<VkDeviceQueueCreateInfo> qInfos;
        std::vector<uint32_t> unique={(uint32_t)graphicsFamily,(uint32_t)presentFamily};
        for(uint32_t f:unique){
            VkDeviceQueueCreateInfo qci{};
            qci.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qci.queueFamilyIndex=f; qci.queueCount=1; qci.pQueuePriorities=&priority;
            qInfos.push_back(qci);
        }

        VkDeviceCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount=(uint32_t)qInfos.size();
        info.pQueueCreateInfos=qInfos.data();
        VkPhysicalDeviceFeatures features{};
        info.pEnabledFeatures=&features;

        if(vkCreateDevice(physicalDevice,&info,nullptr,&device)!=VK_SUCCESS)
            throw std::runtime_error("failed to create device");

        vkGetDeviceQueue(device,graphicsFamily,0,&graphicsQueue);
        vkGetDeviceQueue(device,presentFamily,0,&presentQueue);
    }

    void createSwapchain() {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,surface,&caps);
        swapchainExtent=caps.currentExtent;
        swapchainImageFormat=VK_FORMAT_B8G8R8A8_SRGB;

        VkSwapchainCreateInfoKHR info{};
        info.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface=surface;
        info.minImageCount=caps.minImageCount+1;
        info.imageFormat=swapchainImageFormat;
        info.imageColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        info.imageExtent=swapchainExtent;
        info.imageArrayLayers=1;
        info.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform=caps.currentTransform;
        info.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode=VK_PRESENT_MODE_FIFO_KHR;
        info.clipped=VK_TRUE;

        if(vkCreateSwapchainKHR(device,&info,nullptr,&swapchain)!=VK_SUCCESS)
            throw std::runtime_error("failed swapchain");

        uint32_t count=0;
        vkGetSwapchainImagesKHR(device,swapchain,&count,nullptr);
        swapchainImages.resize(count);
        vkGetSwapchainImagesKHR(device,swapchain,&count,swapchainImages.data());
    }

    void createImageViews() {
        swapchainImageViews.resize(swapchainImages.size());
        for(size_t i=0;i<swapchainImages.size();i++){
            VkImageViewCreateInfo info{};
            info.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image=swapchainImages[i];
            info.viewType=VK_IMAGE_VIEW_TYPE_2D;
            info.format=swapchainImageFormat;
            info.components.r=VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g=VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b=VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a=VK_COMPONENT_SWIZZLE_IDENTITY;
            info.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel=0;
            info.subresourceRange.levelCount=1;
            info.subresourceRange.baseArrayLayer=0;
            info.subresourceRange.layerCount=1;
            if(vkCreateImageView(device,&info,nullptr,&swapchainImageViews[i])!=VK_SUCCESS)
                throw std::runtime_error("failed image view");
        }
    }

    void createRenderPass() {
        VkAttachmentDescription color{};
        color.format=swapchainImageFormat;
        color.samples=VK_SAMPLE_COUNT_1_BIT;
        color.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
        color.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
        color.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
        color.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment=0; colorRef.layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount=1;
        subpass.pColorAttachments=&colorRef;

        VkRenderPassCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount=1; info.pAttachments=&color;
        info.subpassCount=1; info.pSubpasses=&subpass;

        if(vkCreateRenderPass(device,&info,nullptr,&renderPass)!=VK_SUCCESS)
            throw std::runtime_error("failed render pass");
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding ubo{};
        ubo.binding=0; ubo.descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo.descriptorCount=1; ubo.stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount=1; info.pBindings=&ubo;

        if(vkCreateDescriptorSetLayout(device,&info,nullptr,&descriptorSetLayout)!=VK_SUCCESS)
            throw std::runtime_error("failed descriptor set layout");
    }

    void createGraphicsPipeline() {
        auto vertCode=readFile("Shaders/vert.spv");
        auto fragCode=readFile("Shaders/frag.spv");

        VkShaderModule vertModule=createShaderModule(vertCode);
        VkShaderModule fragModule=createShaderModule(fragCode);

        VkPipelineShaderStageCreateInfo vertStage{};
        vertStage.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertStage.stage=VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module=vertModule; vertStage.pName="main";

        VkPipelineShaderStageCreateInfo fragStage{};
        fragStage.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage=VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module=fragModule; fragStage.pName="main";

        VkPipelineShaderStageCreateInfo stages[]={vertStage,fragStage};

        VkPipelineVertexInputStateCreateInfo vi{};
        vi.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo ia{};
        ia.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport viewport{};
        viewport.x=0; viewport.y=0;
        viewport.width=(float)swapchainExtent.width;
        viewport.height=(float)swapchainExtent.height;
        viewport.minDepth=0.0f; viewport.maxDepth=1.0f;

        VkRect2D scissor{};
        scissor.offset={0,0}; scissor.extent=swapchainExtent;

        VkPipelineViewportStateCreateInfo vp{};
        vp.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vp.viewportCount=1; vp.pViewports=&viewport;
        vp.scissorCount=1; vp.pScissors=&scissor;

        VkPipelineRasterizationStateCreateInfo rs{};
        rs.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs.polygonMode=VK_POLYGON_MODE_FILL; rs.lineWidth=1.0f;
        rs.cullMode=VK_CULL_MODE_BACK_BIT; rs.frontFace=VK_FRONT_FACE_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo ms{};
        ms.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState cb{};
        cb.colorWriteMask=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|
                          VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo cbState{};
        cbState.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        cbState.attachmentCount=1; cbState.pAttachments=&cb;

        VkPipelineLayoutCreateInfo pl{};
        pl.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pl.setLayoutCount=1; pl.pSetLayouts=&descriptorSetLayout;

        if(vkCreatePipelineLayout(device,&pl,nullptr,&pipelineLayout)!=VK_SUCCESS)
            throw std::runtime_error("failed pipeline layout");

        VkGraphicsPipelineCreateInfo gp{};
        gp.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        gp.stageCount=2; gp.pStages=stages;
        gp.pVertexInputState=&vi; gp.pInputAssemblyState=&ia;
        gp.pViewportState=&vp; gp.pRasterizationState=&rs;
        gp.pMultisampleState=&ms; gp.pColorBlendState=&cbState;
        gp.layout=pipelineLayout; gp.renderPass=renderPass; gp.subpass=0;

        if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&gp,nullptr,&graphicsPipeline)!=VK_SUCCESS)
            throw std::runtime_error("failed graphics pipeline");

        vkDestroyShaderModule(device,vertModule,nullptr);
        vkDestroyShaderModule(device,fragModule,nullptr);
    }

    // ---------------- Command Buffers ----------------
    void createCommandPool() {
        uint32_t queueCount=0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueCount,nullptr);
        std::vector<VkQueueFamilyProperties> queues(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&queueCount,queues.data());
        int graphicsFamily=-1;
        for(int i=0;i<queueCount;i++){ if(queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsFamily=i; }
        VkCommandPoolCreateInfo info{};
        info.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.queueFamilyIndex=graphicsFamily;
        if(vkCreateCommandPool(device,&info,nullptr,&commandPool)!=VK_SUCCESS)
            throw std::runtime_error("failed command pool");
    }

    // ---------------- Vertex & Index Buffers ----------------
    void createVertexBuffer() {
        VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = size;
        info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &info, nullptr, &vertexBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create vertex buffer");

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(device, vertexBuffer, &memReq);

        VkMemoryAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = memReq.size;
        alloc.memoryTypeIndex = 0;
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if ((memReq.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                alloc.memoryTypeIndex = i;
                break;
            }
        }

        if (vkAllocateMemory(device, &alloc, nullptr, &vertexBufferMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate vertex buffer memory");

        vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

        void* data;
        vkMapMemory(device, vertexBufferMemory, 0, size, 0, &data);
        memcpy(data, vertices.data(), (size_t)size);
        vkUnmapMemory(device, vertexBufferMemory);
    }

    void createIndexBuffer() {
        VkDeviceSize size = sizeof(indices[0]) * indices.size();

        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = size;
        info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &info, nullptr, &indexBuffer) != VK_SUCCESS)
            throw std::runtime_error("failed to create index buffer");

        VkMemoryRequirements memReq;
        vkGetBufferMemoryRequirements(device, indexBuffer, &memReq);

        VkMemoryAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc.allocationSize = memReq.size;

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
            if ((memReq.memoryTypeBits & (1 << i)) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
                (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                alloc.memoryTypeIndex = i;
                break;
            }
        }

        if (vkAllocateMemory(device, &alloc, nullptr, &indexBufferMemory) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate index buffer memory");

        vkBindBufferMemory(device, indexBuffer, indexBufferMemory, 0);

        void* data;
        vkMapMemory(device, indexBufferMemory, 0, size, 0, &data);
        memcpy(data, indices.data(), (size_t)size);
        vkUnmapMemory(device, indexBufferMemory);
    }

    // ---------------- Uniform Buffers & Descriptor Sets ----------------
    void createUniformBuffers() {
        VkDeviceSize size = sizeof(UniformBufferObject);
        uniformBuffers.resize(swapchainImages.size());
        uniformBuffersMemory.resize(swapchainImages.size());

        for (size_t i = 0; i < swapchainImages.size(); i++) {
            VkBufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            info.size = size;
            info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            if (vkCreateBuffer(device, &info, nullptr, &uniformBuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create uniform buffer");

            VkMemoryRequirements memReq;
            vkGetBufferMemoryRequirements(device, uniformBuffers[i], &memReq);

            VkMemoryAllocateInfo alloc{};
            alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc.allocationSize = memReq.size;

            VkPhysicalDeviceMemoryProperties memProps;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
            for (uint32_t j = 0; j < memProps.memoryTypeCount; j++) {
                if ((memReq.memoryTypeBits & (1 << j)) &&
                    (memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
                    (memProps.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                    alloc.memoryTypeIndex = j;
                    break;
                }
            }

            if (vkAllocateMemory(device, &alloc, nullptr, &uniformBuffersMemory[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to allocate uniform buffer memory");

            vkBindBufferMemory(device, uniformBuffers[i], uniformBuffersMemory[i], 0);
        }
    }

    void createDescriptorPool() {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(swapchainImages.size());

        VkDescriptorPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.poolSizeCount = 1;
        info.pPoolSizes = &poolSize;
        info.maxSets = static_cast<uint32_t>(swapchainImages.size());

        if (vkCreateDescriptorPool(device, &info, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("failed to create descriptor pool");
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(swapchainImages.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc.descriptorPool = descriptorPool;
        alloc.descriptorSetCount = static_cast<uint32_t>(swapchainImages.size());
        alloc.pSetLayouts = layouts.data();

        descriptorSets.resize(swapchainImages.size());
        if (vkAllocateDescriptorSets(device, &alloc, descriptorSets.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate descriptor sets");

        for (size_t i = 0; i < swapchainImages.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSets[i];
            write.dstBinding = 0;
            write.dstArrayElement = 0;
            write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        }
    }

    // ---------------- Framebuffers ----------------
    void createFramebuffers() {
        swapchainFramebuffers.resize(swapchainImageViews.size());
        for (size_t i = 0; i < swapchainImageViews.size(); i++) {
            VkImageView attachments[] = { swapchainImageViews[i] };

            VkFramebufferCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = renderPass;
            info.attachmentCount = 1;
            info.pAttachments = attachments;
            info.width = swapchainExtent.width;
            info.height = swapchainExtent.height;
            info.layers = 1;

            if (vkCreateFramebuffer(device, &info, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer");
        }
    }

    // ---------------- Command Buffers ----------------
    void createCommandBuffers() {
        commandBuffers.resize(swapchainFramebuffers.size());
        VkCommandBufferAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc.commandPool = commandPool;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &alloc, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers");

        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo begin{};
            begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            vkBeginCommandBuffer(commandBuffers[i], &begin);

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

            VkRenderPassBeginInfo rp{};
            rp.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rp.renderPass = renderPass;
            rp.framebuffer = swapchainFramebuffers[i];
            rp.renderArea.offset = {0,0};
            rp.renderArea.extent = swapchainExtent;
            rp.clearValueCount = 1;
            rp.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffers[i], &rp, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
            vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

            vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);
            vkEndCommandBuffer(commandBuffers[i]);
        }
    }

    // ---------------- Main Loop ----------------
    void mainLoop() {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(device);
    }

    void updateUniform(uint32_t imageIndex) {
        UniformBufferObject u{};
        float t=static_cast<float>(glfwGetTime());
        u.color[0]=std::sin(t)*0.5f+0.5f;
        u.color[1]=std::cos(t*0.7f)*0.5f+0.5f;
        u.color[2]=std::sin(t*0.3f)*0.5f+0.5f;
        u.color[3]=1.0f;
        void* data;
        vkMapMemory(device,uniformBuffersMemory[imageIndex],0,sizeof(u),0,&data);
        std::memcpy(data,&u,sizeof(u));
        vkUnmapMemory(device,uniformBuffersMemory[imageIndex]);
    }

    void drawFrame() {
        vkWaitForFences(device,1,&inFlightFence,VK_TRUE,UINT64_MAX);
        vkResetFences(device,1,&inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device,swapchain,UINT64_MAX,imageAvailableSemaphore,VK_NULL_HANDLE,&imageIndex);

        updateUniform(imageIndex);

        VkPipelineStageFlags waitStages[]={VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submit{};
        submit.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount=1;
        submit.pWaitSemaphores=&imageAvailableSemaphore;
        submit.pWaitDstStageMask=waitStages;
        submit.commandBufferCount=1;
        submit.pCommandBuffers=&commandBuffers[imageIndex];
        submit.signalSemaphoreCount=1;
        submit.pSignalSemaphores=&renderFinishedSemaphore;

        if(vkQueueSubmit(graphicsQueue,1,&submit,inFlightFence)!=VK_SUCCESS)
            throw std::runtime_error("submit failed");

        VkPresentInfoKHR present{};
        present.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.waitSemaphoreCount=1;
        present.pWaitSemaphores=&renderFinishedSemaphore;
        VkSwapchainKHR swapchains[]={swapchain};
        present.swapchainCount=1; present.pSwapchains=swapchains;
        present.pImageIndices=&imageIndex;

        vkQueuePresentKHR(presentQueue,&present);
    }

    void cleanup() {
        vkDeviceWaitIdle(device);
        vkDestroyPipeline(device,graphicsPipeline,nullptr);
        vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
        vkDestroyRenderPass(device,renderPass,nullptr);
        for(auto view:swapchainImageViews) vkDestroyImageView(device,view,nullptr);
        vkDestroySwapchainKHR(device,swapchain,nullptr);
        vkDestroyDevice(device,nullptr);
        vkDestroySurfaceKHR(instance,surface,nullptr);
        vkDestroyInstance(instance,nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    SquareApp app;
    try{ app.run(); }
    catch(const std::exception& e){ std::cerr<<e.what()<<"\n"; return EXIT_FAILURE; }
    return EXIT_SUCCESS;
}
