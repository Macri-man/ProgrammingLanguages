// main.cpp
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <vector>
#include <optional>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <array>
#include <cstring>
#include <cassert>

#define VK_CHECK(x) do { VkResult err = (x); if (err != VK_SUCCESS) { std::cerr << "VK error " << err << " at " << __LINE__ << std::endl; std::exit(1);} } while(0)

static std::vector<char> readFile(const char* path) {
    std::ifstream f(path, std::ios::binary|std::ios::ate);
    if (!f) { return {}; }
    size_t sz = (size_t)f.tellg(); f.seekg(0);
    std::vector<char> buf(sz); f.read(buf.data(), sz); return buf;
}

int main() {
    const int WIDTH = 640;
    const int HEIGHT = 480;
    const int FRAMES = 180;
    const char* OUT_H264 = "output.h264";

    // GLFW init
    if (!glfwInit()) { std::cerr << "glfw init failed\n"; return -1; }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Compute -> Video (NVIDIA)", nullptr, nullptr);

    // Instance
    VkInstance instance;
    {
        VkApplicationInfo ai{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        ai.pApplicationName = "vk_compute_video";
        ai.apiVersion = VK_API_VERSION_1_3;
        VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        ici.pApplicationInfo = &ai;

        uint32_t extCount = 0;
        const char** glfwExts = glfwGetRequiredInstanceExtensions(&extCount);
        std::vector<const char*> exts(glfwExts, glfwExts + extCount);
        ici.enabledExtensionCount = (uint32_t)exts.size();
        ici.ppEnabledExtensionNames = exts.data();
        VK_CHECK(vkCreateInstance(&ici, nullptr, &instance));
    }

    // Surface (not used for presenting but created for potential queries)
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VK_CHECK(glfwCreateWindowSurface(instance, window, nullptr, &surface));

    // Pick physical device with compute queue and (preferably) video encode support
    VkPhysicalDevice phys = VK_NULL_HANDLE;
    uint32_t pcount = 0; vkEnumeratePhysicalDevices(instance, &pcount, nullptr);
    std::vector<VkPhysicalDevice> pdevs(pcount);
    vkEnumeratePhysicalDevices(instance, &pcount, pdevs.data());

    std::optional<uint32_t> queueFamilyIdx;
    for (auto &d : pdevs) {
        uint32_t qcnt=0; vkGetPhysicalDeviceQueueFamilyProperties(d, &qcnt, nullptr);
        std::vector<VkQueueFamilyProperties> qprops(qcnt);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qcnt, qprops.data());
        for (uint32_t i=0;i<qcnt;i++) {
            if (qprops[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                phys = d; queueFamilyIdx = i; break;
            }
        }
        if (phys) break;
    }
    if (!phys) { std::cerr << "No physical device with compute queue\n"; return -1; }

    // Query device extensions to check for VIDEO support at runtime (diagnostic)
    uint32_t devExtCount=0;
    vkEnumerateDeviceExtensionProperties(phys, nullptr, &devExtCount, nullptr);
    std::vector<VkExtensionProperties> devExts(devExtCount);
    vkEnumerateDeviceExtensionProperties(phys, nullptr, &devExtCount, devExts.data());
    bool has_video_queue = false, has_video_encode_queue = false, has_h264 = false;
    for (auto &e : devExts) {
        std::string name(e.extensionName);
        if (name == "VK_KHR_video_queue") has_video_queue = true;
        if (name == "VK_KHR_video_encode_queue") has_video_encode_queue = true;
        if (name == "VK_KHR_video_encode_h264") has_h264 = true;
    }
    std::cout << "Device extensions found: VK_KHR_video_queue="<<has_video_queue
              << " VK_KHR_video_encode_queue="<<has_video_encode_queue
              << " VK_KHR_video_encode_h264="<<has_h264 << std::endl;

    // Create device with requested extensions (we request them; if driver rejects creation, you'll see error)
    std::vector<const char*> requestedDeviceExts = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_video_queue",
        "VK_KHR_video_encode_queue",
        "VK_KHR_video_encode_h264"
    };

    float qprio = 1.0f;
    VkDevice device;
    VkQueue queue;
    {
        VkDeviceQueueCreateInfo dqci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        dqci.queueFamilyIndex = *queueFamilyIdx;
        dqci.queueCount = 1;
        dqci.pQueuePriorities = &qprio;

        VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        dci.queueCreateInfoCount = 1;
        dci.pQueueCreateInfos = &dqci;
        dci.enabledExtensionCount = static_cast<uint32_t>(requestedDeviceExts.size());
        dci.ppEnabledExtensionNames = requestedDeviceExts.data();

        VkResult r = vkCreateDevice(phys, &dci, nullptr, &device);
        if (r != VK_SUCCESS) {
            std::cerr << "vkCreateDevice failed: " << r << " — falling back to create device without video extensions\n";
            // Fallback: try create device without video extensions
            dci.enabledExtensionCount = 0;
            VK_CHECK(vkCreateDevice(phys, &dci, nullptr, &device));
        }
        vkGetDeviceQueue(device, *queueFamilyIdx, 0, &queue);
    }

    // Command pool + one command buffer
    VkCommandPool cmdPool;
    {
        VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pci.queueFamilyIndex = *queueFamilyIdx;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK(vkCreateCommandPool(device, &pci, nullptr, &cmdPool));
    }
    VkCommandBuffer cmd;
    {
        VkCommandBufferAllocateInfo cbai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        cbai.commandPool = cmdPool;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbai.commandBufferCount = 1;
        VK_CHECK(vkAllocateCommandBuffers(device, &cbai, &cmd));
    }

    // Create RGBA storage image (device local)
    VkImage rgbaImage;
    VkDeviceMemory rgbaMem;
    VkImageView rgbaView;
    {
        VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = VK_FORMAT_R8G8B8A8_UNORM;
        ici.extent = VkExtent3D{(uint32_t)WIDTH, (uint32_t)HEIGHT, 1};
        ici.mipLevels = 1; ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VK_CHECK(vkCreateImage(device, &ici, nullptr, &rgbaImage));
        VkMemoryRequirements mr; vkGetImageMemoryRequirements(device, rgbaImage, &mr);
        VkPhysicalDeviceMemoryProperties memProps; vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
        uint32_t memType = UINT32_MAX;
        for (uint32_t i=0;i<memProps.memoryTypeCount;i++){
            if ((mr.memoryTypeBits & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) { memType = i; break; }
        }
        if (memType==UINT32_MAX) { std::cerr<<"No device local mem type\n"; return -1; }
        VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        mai.allocationSize = mr.size;
        mai.memoryTypeIndex = memType;
        VK_CHECK(vkAllocateMemory(device, &mai, nullptr, &rgbaMem));
        VK_CHECK(vkBindImageMemory(device, rgbaImage, rgbaMem, 0));

        VkImageViewCreateInfo ivci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        ivci.image = rgbaImage; ivci.viewType = VK_IMAGE_VIEW_TYPE_2D; ivci.format = VK_FORMAT_R8G8B8A8_UNORM;
        ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; ivci.subresourceRange.baseMipLevel = 0;
        ivci.subresourceRange.levelCount = 1; ivci.subresourceRange.baseArrayLayer = 0; ivci.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(device, &ivci, nullptr, &rgbaView));
    }

    // Create NV12 image (two-plane). Format used for NV12 in Vulkan: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    VkImage nv12Image;
    VkDeviceMemory nv12Mem;
    VkImageView nv12ViewY; // plane 0 view
    VkImageView nv12ViewUV; // plane 1 view
    VkFormat nv12Format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    bool nv12_supported = true;
    {
        VkImageCreateInfo ici{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = nv12Format;
        ici.extent = VkExtent3D{(uint32_t)WIDTH, (uint32_t)HEIGHT, 1};
        ici.mipLevels = 1; ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        // include VIDEO_ENCODE_SRC usage for encoder and TRANSFER_DST so conversion can write to it
        ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        // Some drivers require VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR - not available as macro in all SDKs
        // If available use: ici.usage |= VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkResult r = vkCreateImage(device, &ici, nullptr, &nv12Image);
        if (r != VK_SUCCESS) {
            std::cerr << "Failed to create NV12 image with format " << nv12Format << " : " << r << "\n";
            nv12_supported = false;
        } else {
            VkMemoryRequirements mr; vkGetImageMemoryRequirements(device, nv12Image, &mr);
            VkPhysicalDeviceMemoryProperties memProps; vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
            uint32_t memType = UINT32_MAX;
            for (uint32_t i=0;i<memProps.memoryTypeCount;i++){
                if ((mr.memoryTypeBits & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) { memType = i; break; }
            }
            if (memType==UINT32_MAX) { std::cerr << "No device local mem type for nv12\n"; nv12_supported = false; }
            else {
                VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
                mai.allocationSize = mr.size;
                mai.memoryTypeIndex = memType;
                VK_CHECK(vkAllocateMemory(device, &mai, nullptr, &nv12Mem));
                VK_CHECK(vkBindImageMemory(device, nv12Image, nv12Mem, 0));

                // create image views for plane 0 and plane 1 (two-plane view uses viewType = 2D and VkImageSubresourceRange with aspectMask set to PLANE_0 or PLANE_1)
                VkImageViewCreateInfo ivci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
                ivci.image = nv12Image;
                ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
                ivci.format = nv12Format;
                ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT; // plane 0 = Y
                ivci.subresourceRange.baseMipLevel = 0; ivci.subresourceRange.levelCount = 1;
                ivci.subresourceRange.baseArrayLayer = 0; ivci.subresourceRange.layerCount = 1;
                VK_CHECK(vkCreateImageView(device, &ivci, nullptr, &nv12ViewY));
                ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_PLANE_1_BIT; // plane 1 = UV
                VK_CHECK(vkCreateImageView(device, &ivci, nullptr, &nv12ViewUV));
            }
        }
    }

    // Descriptor set / pipeline for visuals compute shader (RGBA write)
    VkDescriptorSetLayout dsl_visual = VK_NULL_HANDLE, dsl_conv = VK_NULL_HANDLE;
    VkPipelineLayout pl_visual = VK_NULL_HANDLE, pl_conv = VK_NULL_HANDLE;
    VkPipeline pipe_visual = VK_NULL_HANDLE, pipe_conv = VK_NULL_HANDLE;
    VkDescriptorPool dpool;
    VkDescriptorSet ds_visual = VK_NULL_HANDLE, ds_conv = VK_NULL_HANDLE;
    {
        // visuals: binding 0 = storage image (rgba)
        VkDescriptorSetLayoutBinding b0{};
        b0.binding = 0; b0.descriptorCount = 1; b0.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; b0.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        VkDescriptorSetLayoutCreateInfo dsli{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        dsli.bindingCount = 1; dsli.pBindings = &b0;
        VK_CHECK(vkCreateDescriptorSetLayout(device, &dsli, nullptr, &dsl_visual));

        // conv: binding 0 = srcRGBA (readonly storage image), binding 1 = dstY (r8), binding 2 = dstUV (rg8)
        std::array<VkDescriptorSetLayoutBinding,3> convBindings{};
        convBindings[0].binding = 0; convBindings[0].descriptorCount = 1; convBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; convBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        convBindings[1].binding = 1; convBindings[1].descriptorCount = 1; convBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; convBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        convBindings[2].binding = 2; convBindings[2].descriptorCount = 1; convBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; convBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        VkDescriptorSetLayoutCreateInfo dsli2{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        dsli2.bindingCount = (uint32_t)convBindings.size(); dsli2.pBindings = convBindings.data();
        VK_CHECK(vkCreateDescriptorSetLayout(device, &dsli2, nullptr, &dsl_conv));

        // pipeline layouts
        VkPushConstantRange pcrVis{}; pcrVis.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; pcrVis.offset = 0; pcrVis.size = sizeof(float) + 2*sizeof(int);
        VkPipelineLayoutCreateInfo plci{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        plci.setLayoutCount = 1; plci.pSetLayouts = &dsl_visual; plci.pushConstantRangeCount = 1; plci.pPushConstantRanges = &pcrVis;
        VK_CHECK(vkCreatePipelineLayout(device, &plci, nullptr, &pl_visual));

        VkPushConstantRange pcrConv{}; pcrConv.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; pcrConv.offset = 0; pcrConv.size = 2*sizeof(int);
        VkPipelineLayoutCreateInfo plci2{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        plci2.setLayoutCount = 1; plci2.pSetLayouts = &dsl_conv; plci2.pushConstantRangeCount = 1; plci2.pPushConstantRanges = &pcrConv;
        VK_CHECK(vkCreatePipelineLayout(device, &plci2, nullptr, &pl_conv));

        // load SPIR-V for visuals
        auto visCode = readFile("shaders/visuals.comp.spv");
        if (visCode.empty()) { std::cerr << "Missing shaders/visuals.comp.spv - compile with glslangValidator\n"; return -1; }
        VkShaderModuleCreateInfo smci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        smci.codeSize = visCode.size(); smci.pCode = reinterpret_cast<const uint32_t*>(visCode.data());
        VkShaderModule smVis; VK_CHECK(vkCreateShaderModule(device, &smci, nullptr, &smVis));
        VkPipelineShaderStageCreateInfo pssVis{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        pssVis.stage = VK_SHADER_STAGE_COMPUTE_BIT; pssVis.module = smVis; pssVis.pName = "main";
        VkComputePipelineCreateInfo cpci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        cpci.stage = pssVis; cpci.layout = pl_visual;
        VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci, nullptr, &pipe_visual));
        vkDestroyShaderModule(device, smVis, nullptr);

        // load SPIR-V for conv
        auto convCode = readFile("shaders/rgba_to_nv12.comp.spv");
        if (convCode.empty()) { std::cerr << "Missing shaders/rgba_to_nv12.comp.spv - compile with glslangValidator\n"; return -1; }
        smci.codeSize = convCode.size(); smci.pCode = reinterpret_cast<const uint32_t*>(convCode.data());
        VkShaderModule smConv; VK_CHECK(vkCreateShaderModule(device, &smci, nullptr, &smConv));
        VkPipelineShaderStageCreateInfo pssConv{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        pssConv.stage = VK_SHADER_STAGE_COMPUTE_BIT; pssConv.module = smConv; pssConv.pName = "main";
        VkComputePipelineCreateInfo cpci2{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        cpci2.stage = pssConv; cpci2.layout = pl_conv;
        VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &cpci2, nullptr, &pipe_conv));
        vkDestroyShaderModule(device, smConv, nullptr);

        // descriptor pool
        std::array<VkDescriptorPoolSize,2> pools{};
        pools[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; pools[0].descriptorCount = 8;
        VkDescriptorPoolCreateInfo dpci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        dpci.maxSets = 4; dpci.poolSizeCount = 1; dpci.pPoolSizes = pools.data();
        VK_CHECK(vkCreateDescriptorPool(device, &dpci, nullptr, &dpool));

        // allocate and update ds_visual
        VkDescriptorSetAllocateInfo dsai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        dsai.descriptorPool = dpool; dsai.descriptorSetCount = 1; dsai.pSetLayouts = &dsl_visual;
        VK_CHECK(vkAllocateDescriptorSets(device, &dsai, &ds_visual));
        VkDescriptorImageInfo diiVis{}; diiVis.imageView = rgbaView; diiVis.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkWriteDescriptorSet wVis{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}; wVis.dstSet = ds_visual; wVis.dstBinding = 0; wVis.descriptorCount = 1;
        wVis.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; wVis.pImageInfo = &diiVis;
        vkUpdateDescriptorSets(device, 1, &wVis, 0, nullptr);

        // allocate and update ds_conv
        VkDescriptorSetAllocateInfo dsai2{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        dsai2.descriptorPool = dpool; dsai2.descriptorSetCount = 1; dsai2.pSetLayouts = &dsl_conv;
        VK_CHECK(vkAllocateDescriptorSets(device, &dsai2, &ds_conv));
        VkDescriptorImageInfo diiSrc{}; diiSrc.imageView = rgbaView; diiSrc.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDescriptorImageInfo diiY{}; diiY.imageView = nv12ViewY; diiY.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        VkDescriptorImageInfo diiUV{}; diiUV.imageView = nv12ViewUV; diiUV.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        std::array<VkWriteDescriptorSet,3> writes{};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; writes[0].dstSet = ds_conv; writes[0].dstBinding = 0; writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; writes[0].pImageInfo = &diiSrc;
        writes[1] = writes[0]; writes[1].dstBinding = 1; writes[1].pImageInfo = &diiY;
        writes[2] = writes[0]; writes[2].dstBinding = 2; writes[2].pImageInfo = &diiUV;
        vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
    }

    // Create staging buffer for readback (RGBA raw dump fallback)
    VkBuffer stagingBuf; VkDeviceMemory stagingMem;
    {
        VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bci.size = WIDTH * HEIGHT * 4;
        bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VK_CHECK(vkCreateBuffer(device, &bci, nullptr, &stagingBuf));
        VkMemoryRequirements mr; vkGetBufferMemoryRequirements(device, stagingBuf, &mr);
        VkPhysicalDeviceMemoryProperties memProps; vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
        uint32_t memType = UINT32_MAX;
        for (uint32_t i=0;i<memProps.memoryTypeCount;i++){
            if ((mr.memoryTypeBits & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) { memType = i; break;}
        }
        if (memType==UINT32_MAX){ std::cerr<<"No host-visible mem type\n"; return -1; }
        VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        mai.allocationSize = mr.size; mai.memoryTypeIndex = memType;
        VK_CHECK(vkAllocateMemory(device, &mai, nullptr, &stagingMem));
        VK_CHECK(vkBindBufferMemory(device, stagingBuf, stagingMem, 0));
    }

    // Load video function pointers (if present)
    auto vkCreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)vkGetDeviceProcAddr(device, "vkCreateVideoSessionKHR");
    auto vkDestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)vkGetDeviceProcAddr(device, "vkDestroyVideoSessionKHR");
    auto vkCmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)vkGetDeviceProcAddr(device, "vkCmdEncodeVideoKHR");
    bool videoAvailable = (vkCreateVideoSessionKHR && vkCmdEncodeVideoKHR);

    if (!videoAvailable) {
        std::cout << "Vulkan Video encode functions not available — program will write raw frames to disk instead.\n";
    } else {
        std::cout << "Vulkan Video encode functions are available. Attempting to create encode session...\n";
    }

    // NOTE: Building a fully conformant VkVideoSessionKHR for H.264 requires many codec-specific structs (VkVideoProfileInfoKHR, VkVideoStdHeader, VkVideoEncodeH264SessionCreateInfoKHR, etc).
    // For succinctness we attempt to create a basic session if functions are present — but in practice you should copy session initialization from nvpro's vk_video_samples.
    VkVideoSessionKHR videoSession = VK_NULL_HANDLE;
    if (videoAvailable) {
        // Prepare minimal structures - **real** code must fill profile IDs / codec GUIDs exactly as spec & sample
        VkVideoSessionCreateInfoKHR sessionCI{VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR};
        sessionCI.queueFamilyIndex = *queueFamilyIdx;
        // chain codec-specific create info via pNext. Example: VkVideoEncodeH264SessionCreateInfoKHR
        // This example leaves pNext null and tries to create a generic session (may fail on some drivers).
        VkResult r = vkCreateVideoSessionKHR(device, &sessionCI, nullptr, &videoSession);
        if (r != VK_SUCCESS) {
            std::cerr << "vkCreateVideoSessionKHR failed: " << r << " — encode path disabled\n";
            videoAvailable = false;
        } else {
            std::cout << "Created VkVideoSessionKHR (handle): " << videoSession << std::endl;
        }
    }

    // Create coded buffer (destination for encoder)
    VkBuffer codedBuf = VK_NULL_HANDLE;
    VkDeviceMemory codedMem = VK_NULL_HANDLE;
    const VkDeviceSize CODED_BUF_SIZE = 8 * 1024 * 1024; // 8 MB; adjust as needed
    if (videoAvailable) {
        VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bci.size = CODED_BUF_SIZE;
        // usage flag: VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR - not always present in headers
        // include TRANSFER_SRC so we can copy out if necessary
        bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VK_CHECK(vkCreateBuffer(device, &bci, nullptr, &codedBuf));
        VkMemoryRequirements mr; vkGetBufferMemoryRequirements(device, codedBuf, &mr);
        VkPhysicalDeviceMemoryProperties memProps; vkGetPhysicalDeviceMemoryProperties(phys, &memProps);
        uint32_t memType = UINT32_MAX;
        for (uint32_t i=0;i<memProps.memoryTypeCount;i++){
            if ((mr.memoryTypeBits & (1u<<i)) && (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) { memType = i; break;}
        }
        if (memType==UINT32_MAX) { std::cerr<<"No mem type for coded buffer; disabling encode\n"; videoAvailable = false; }
        else {
            VkMemoryAllocateInfo mai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
            mai.allocationSize = mr.size; mai.memoryTypeIndex = memType;
            VK_CHECK(vkAllocateMemory(device, &mai, nullptr, &codedMem));
            VK_CHECK(vkBindBufferMemory(device, codedBuf, codedMem, 0));
        }
    }

    // Prepare file for raw fallback or for output h264 (if encode runnable)
    std::ofstream outH264;
    if (videoAvailable) outH264.open(OUT_H264, std::ios::binary);

    // Main loop: for each frame -> dispatch visuals -> dispatch RGBA->NV12 -> encode (or dump)
    auto startTime = std::chrono::steady_clock::now();
    for (int f=0; f<FRAMES && !glfwWindowShouldClose(window); ++f) {
        // begin command buffer
        VkCommandBufferBeginInfo cbbi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VK_CHECK(vkBeginCommandBuffer(cmd, &cbbi));

        // Transition images to GENERAL for compute access (simple barrier)
        auto imageBarrier = [&](VkImage img, VkImageLayout oldL, VkImageLayout newL) {
            VkImageMemoryBarrier imb{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imb.oldLayout = oldL; imb.newLayout = newL;
            imb.srcAccessMask = 0; imb.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
            imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imb.image = img; imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imb.subresourceRange.baseMipLevel = 0; imb.subresourceRange.levelCount = 1;
            imb.subresourceRange.baseArrayLayer = 0; imb.subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imb);
        };
        // rgbaImage: UNDEFINED -> GENERAL for first frame; subsequent transitions use GENERAL->GENERAL if needed
        imageBarrier(rgbaImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        if (nv12_supported) {
            // NV12 planes: treat as general for compute writes (driver-specific)
            imageBarrier(nv12Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        }

        // Bind visuals pipeline and descriptor
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_visual);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pl_visual, 0, 1, &ds_visual, 0, nullptr);
        // push constants: time, width, height
        float t = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();
        struct PCVis { float time; int width; int height; } pcvis{ t, WIDTH, HEIGHT };
        vkCmdPushConstants(cmd, pl_visual, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pcvis), &pcvis);

        uint32_t gx = (WIDTH + 15) / 16, gy = (HEIGHT + 15) / 16;
        vkCmdDispatch(cmd, gx, gy, 1);

        // Barrier to ensure shader writes to rgbaImage are visible to the next compute (NV12 conversion)
        VkMemoryBarrier membar{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        membar.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; membar.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &membar, 0, nullptr, 0, nullptr);

        if (nv12_supported) {
            // Bind conversion pipeline and descriptor (reads rgbaView, writes nv12 planes)
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipe_conv);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pl_conv, 0, 1, &ds_conv, 0, nullptr);
            struct PCConv { int width; int height; } pcconv{ WIDTH, HEIGHT };
            vkCmdPushConstants(cmd, pl_conv, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(pcconv), &pcconv);
            vkCmdDispatch(cmd, gx, gy, 1);
            // Barrier to ensure NV12 image is ready before encode
            VkMemoryBarrier membar2{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
            membar2.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; membar2.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &membar2, 0, nullptr, 0, nullptr);
        } else {
            // no NV12: copy rgbaImage to staging buffer and write raw frame (fallback)
            VkBufferImageCopy bic{};
            bic.bufferRowLength = 0; bic.bufferImageHeight = 0;
            bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bic.imageSubresource.mipLevel = 0; bic.imageSubresource.baseArrayLayer = 0; bic.imageSubresource.layerCount = 1;
            bic.imageExtent = VkExtent3D{(uint32_t)WIDTH, (uint32_t)HEIGHT, 1};
            vkCmdCopyImageToBuffer(cmd, rgbaImage, VK_IMAGE_LAYOUT_GENERAL, stagingBuf, 1, &bic);
            // memory barrier for transfer -> host
            VkBufferMemoryBarrier bmb{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
            bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; bmb.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bmb.buffer = stagingBuf; bmb.offset = 0; bmb.size = VK_WHOLE_SIZE;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 1, &bmb, 0, nullptr);
        }

        // If video available: encode; otherwise, fallback to readback raw frames
        if (videoAvailable && nv12_supported) {
            // Prepare encode command: we must fill VkVideoEncodeInfoKHR etc.
            // Full code requires populating VkVideoPictureResourceKHR for src (pointing to the NV12 image planes),
            // and a VkBuffer or VkVideoCodedBufferKHR region for dst (codedBuf).
            // Here we show the conceptual call; the driver will require a correct struct chain.
            VkVideoEncodeInfoKHR encodeInfo{VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR};
            // NOTE: In practice you must fill encodeInfo.srcPictureResource and encodeInfo.dstBitstream properly.
            // We'll attempt to call vkCmdEncodeVideoKHR if pointers exist, but it's likely you must copy the exact code
            // from nvpro encode sample for it to succeed.
            // This example intentionally does not try to fabricate all required nested structs.
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VIDEO_ENCODE_BIT_KHR, 0, 0, nullptr, 0, nullptr, 0, nullptr);
            // call encode (if available)
            vkCmdEncodeVideoKHR(cmd, &encodeInfo);
            // NOTE: many drivers require specific parameters and reference picture DPB handling.
        } else {
            // fallback: copy rgba -> staging and map
            VkBufferImageCopy bic{};
            bic.bufferRowLength = 0; bic.bufferImageHeight = 0;
            bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bic.imageSubresource.mipLevel = 0; bic.imageSubresource.baseArrayLayer = 0; bic.imageSubresource.layerCount = 1;
            bic.imageExtent = VkExtent3D{(uint32_t)WIDTH, (uint32_t)HEIGHT, 1};
            vkCmdCopyImageToBuffer(cmd, rgbaImage, VK_IMAGE_LAYOUT_GENERAL, stagingBuf, 1, &bic);
            VkBufferMemoryBarrier bmb{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
            bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; bmb.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
            bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bmb.buffer = stagingBuf; bmb.offset = 0; bmb.size = VK_WHOLE_SIZE;
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 1, &bmb, 0, nullptr);
        }

        VK_CHECK(vkEndCommandBuffer(cmd));

        // submit and wait
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
        VK_CHECK(vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE));
        VK_CHECK(vkQueueWaitIdle(queue));

        if (!videoAvailable || !nv12_supported) {
            // map staging and append to raw file
            void* mapped = nullptr;
            VK_CHECK(vkMapMemory(device, stagingMem, 0, VK_WHOLE_SIZE, 0, &mapped));
            std::ofstream fout("frames_rgba.raw", std::ios::binary | std::ios::app);
            fout.write(reinterpret_cast<char*>(mapped), WIDTH * HEIGHT * 4);
            fout.close();
            vkUnmapMemory(device, stagingMem);
            std::cout << "Wrote frame " << f << " to frames_rgba.raw\n";
        } else {
            // Encode path: after encode submission we'd map codedBuf memory and write proper NALs into output.h264
            // The exact parsing depends on vendor-encoded buffer layout (nvpro contains helpers).
            // For brevity we just print a debug message.
            std::cout << "Frame " << f << " encoded (conceptually) - see nvpro sample code to extract NAL bytes.\n";
        }

        // Wait a bit to simulate 30fps
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    // Cleanup
    if (outH264.is_open()) outH264.close();
    vkDeviceWaitIdle(device);

    if (codedBuf) vkDestroyBuffer(device, codedBuf, nullptr);
    if (codedMem) vkFreeMemory(device, codedMem, nullptr);

    vkDestroyBuffer(device, stagingBuf, nullptr);
    vkFreeMemory(device, stagingMem, nullptr);

    vkDestroyPipeline(device, pipe_visual, nullptr);
    vkDestroyPipeline(device, pipe_conv, nullptr);
    vkDestroyPipelineLayout(device, pl_visual, nullptr);
    vkDestroyPipelineLayout(device, pl_conv, nullptr);
    vkDestroyDescriptorSetLayout(device, dsl_visual, nullptr);
    vkDestroyDescriptorSetLayout(device, dsl_conv, nullptr);
    vkDestroyDescriptorPool(device, dpool, nullptr);

    if (rgbaView) vkDestroyImageView(device, rgbaView, nullptr);
    if (rgbaImage) vkDestroyImage(device, rgbaImage, nullptr);
    if (rgbaMem) vkFreeMemory(device, rgbaMem, nullptr);

    if (nv12ViewY) vkDestroyImageView(device, nv12ViewY, nullptr);
    if (nv12ViewUV) vkDestroyImageView(device, nv12ViewUV, nullptr);
    if (nv12Image) vkDestroyImage(device, nv12Image, nullptr);
    if (nv12Mem) vkFreeMemory(device, nv12Mem, nullptr);

    if (videoSession && vkDestroyVideoSessionKHR) vkDestroyVideoSessionKHR(device, videoSession, nullptr);

    vkDestroyCommandPool(device, cmdPool, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
