#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Util/Logger.h"

#include <cassert>

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIRenderImageAttachment.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexImage.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexImageView.h"

#define COMPILE_VULKAN_VERSION VK_API_VERSION_1_3
#define DESCRIPTOR_POOL_SIZE 5

void VulkanRHI::InitializeVulkan()
{
    CreateVulkanInstance();
    LoadExtensionFunctions();

    if(VulkanContext->EnableDebug)
    {
        SetupMessengerUtil();
    }
    CreateWindowSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    LoadDeviceExtensionFunctions();
    CreateSwapchain();
    CreateSwapchainImageView();
    CreateDefaultAttachments();
    CreateVmaAllocator();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSyncPrimitives();
    CreateDescriptorPool();
}

void VulkanRHI::CreateVulkanInstance()
{
    VkApplicationInfo VulkanAppInfo{};
    VulkanAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    VulkanAppInfo.pApplicationName = "OpenRender";
    VulkanAppInfo.pEngineName = "OpenRender Engine";
    VulkanAppInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    VulkanAppInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    VulkanAppInfo.apiVersion = COMPILE_VULKAN_VERSION;

    VkInstanceCreateInfo VulkanInstanceInfo{};
    VulkanInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    VulkanInstanceInfo.pApplicationInfo = &VulkanAppInfo;

    VkResult EnumResult;
    std::vector<const char*> PlatformExtensionRequirements;
    
    if(VulkanContext->EnableDebug)
    {
        VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};

        VulkanInstanceInfo.pNext = &DebugCreateInfo;
        
        DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ;
        DebugCreateInfo.pfnUserCallback = VKTool::ValidationLayerDebugCallBack;

        LOG_DEBUG_FUNCTION("Debug enabled, check validation layer support");
        auto LayerProperties = VKTool::EnumAndGetAttrib<VkLayerProperties>(vkEnumerateInstanceLayerProperties, EnumResult);
        bool CheckLayer = VKTool::CheckSupport<VkLayerProperties>(
            {VULKAN_CXT_VALIDATION_LAYER_REQUIRE},
            LayerProperties,
            [](const char* Name, const VkLayerProperties& Property){
            return strcmp(Name, Property.layerName) == 0; 
        });

        assert(CheckLayer);

        LayerRequires.push_back(VULKAN_CXT_VALIDATION_LAYER_REQUIRE);
        PlatformExtensionRequirements.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VulkanInstanceInfo.enabledLayerCount = (uint32_t)LayerRequires.size();
    if(VulkanInstanceInfo.enabledLayerCount)
    {
        VulkanInstanceInfo.ppEnabledLayerNames = LayerRequires.data();
    }

    
    uint32_t PlatformExtensionCount = 0;
    const char **  PlatformExtensions = glfwGetRequiredInstanceExtensions(&PlatformExtensionCount);

    PlatformExtensionRequirements.insert(PlatformExtensionRequirements.end(), PlatformExtensions, PlatformExtensions + PlatformExtensionCount);
    PlatformExtensionRequirements.insert(PlatformExtensionRequirements.end(), InstanceExtensionRequires.begin(), InstanceExtensionRequires.end());
    
    LOG_DEBUG_FUNCTION("Check extension support");
    auto ExtensionProperties = VKTool::EnumAndGetFrom<VkExtensionProperties>((const char*)nullptr,vkEnumerateInstanceExtensionProperties, EnumResult);
    bool CheckExtension = VKTool::CheckSupport<VkExtensionProperties>(PlatformExtensionRequirements,ExtensionProperties,
        [](const char* Name, const VkExtensionProperties& Properties)
        {
            return strcmp(Name, Properties.extensionName) == 0;
        });

    assert(CheckExtension);

    VulkanInstanceInfo.enabledExtensionCount = (uint32_t)PlatformExtensionRequirements.size();
    VulkanInstanceInfo.ppEnabledExtensionNames = PlatformExtensionRequirements.data();

    LOG_DEBUG_FUNCTION("Creating vulkan instance");
    VkResult CreateInstanceResult = vkCreateInstance(&VulkanInstanceInfo, nullptr, &VulkanContext->Instance);
    assert(CreateInstanceResult == VK_SUCCESS);
}

void VulkanRHI::LoadExtensionFunctions()
{
    VulkanEXT.VKCreateDebugUtilsMessengerEXT =
        VKTool::LoadFunction<PFN_vkCreateDebugUtilsMessengerEXT>(
            VulkanContext,
            "vkCreateDebugUtilsMessengerEXT");
    VulkanEXT.VKDstroyDebugUtilsMessengerEXT =
        VKTool::LoadFunction<PFN_vkDestroyDebugUtilsMessengerEXT>(
            VulkanContext,
            "vkDestroyDebugUtilsMessengerEXT");
}

void VulkanRHI::SetupMessengerUtil()
{
    VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};
    
    DebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    DebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ;
    DebugCreateInfo.pfnUserCallback = VKTool::ValidationLayerDebugCallBack;

    LOG_DEBUG_FUNCTION("Debug enabled, create messenger");
    
    VkResult CreateResult = VulkanEXT.VKCreateDebugUtilsMessengerEXT(
        VulkanContext->Instance,
        &DebugCreateInfo,
        nullptr,
        &VulkanContext->DebugMessenger);

    assert(CreateResult == VK_SUCCESS);
}

void VulkanRHI::CreateWindowSurface()
{
    LOG_DEBUG_FUNCTION("Creating window surface");
    VkResult CreateResult = glfwCreateWindowSurface(VulkanContext->Instance, WindowPtr, nullptr, &VulkanContext->WindowSurface);
    assert(CreateResult == VK_SUCCESS);
}

void VulkanRHI::PickPhysicalDevice()
{
    LOG_DEBUG_FUNCTION("Start picking physical device");
    std::vector<std::pair<size_t, VkPhysicalDevice>> RankedDevices = VKTool::GetRankedGPU(VulkanContext);

    auto TestGPU = [&](VkPhysicalDevice PhysicalDevice) -> bool
    {
        int GraphicsQueueIndex = VKTool::FindQueueFamilyByBit(PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
        int ComputeQueueIndex = VKTool::FindQueueFamilyByBit(PhysicalDevice, VK_QUEUE_COMPUTE_BIT);
        int PresentQueueIndex = VKTool::FindPresentQueueFamily(PhysicalDevice, VulkanContext);

        if(GraphicsQueueIndex == -1 || PresentQueueIndex == -1)
        {
            return false;
        }

        VkResult QueryResult;
        std::vector<VkExtensionProperties> DeviceExtensions = VKTool::EnumAndGetFrom2P<VkExtensionProperties>(
            PhysicalDevice,
            (const char*)nullptr,
            vkEnumerateDeviceExtensionProperties,
            QueryResult);

        bool CheckSupport = VKTool::CheckSupport<VkExtensionProperties>(
            DeviceExtensionRequires,
            DeviceExtensions, [](const char* Name, const VkExtensionProperties& Properties)
            {
                return strcmp(Name, Properties.extensionName) == 0;
            });

        if(!CheckSupport)
        {
            return false;
        }

        std::vector<VkSurfaceFormatKHR> Formats = VKTool::GetPropertiesArray2P<VkSurfaceFormatKHR>(
            PhysicalDevice,
            VulkanContext->WindowSurface,
            vkGetPhysicalDeviceSurfaceFormatsKHR);
        std::vector<VkPresentModeKHR> PresentModes = VKTool::GetPropertiesArray2P<VkPresentModeKHR>(
            PhysicalDevice,
            VulkanContext->WindowSurface,
            vkGetPhysicalDeviceSurfacePresentModesKHR);

        if(Formats.empty() || PresentModes.empty())
        {
            return false;
        }

        VulkanContext->SurfaceFormats = std::move(Formats);
        VulkanContext->PresentMods = std::move(PresentModes);
        VulkanContext->GraphicsQueueFamilyIndex = GraphicsQueueIndex;
        VulkanContext->PresentQueueFamilyIndex = PresentQueueIndex;
        VulkanContext->ComputeQueueFamilyIndex = ComputeQueueIndex;

        return true;
    };
    
    for(auto& GPUPair : RankedDevices)
    {
        if(GPUPair.first <= 0)
        {
            continue;
        }

        if(TestGPU(GPUPair.second))
        {
            VulkanContext->PhysicalDevice = GPUPair.second;
        }
    }

    
    assert(VulkanContext->PhysicalDevice);
    
}

void VulkanRHI::CreateLogicalDevice()
{

    LOG_DEBUG_FUNCTION("Creating logical device");
    
    VkDeviceCreateInfo DeviceInfo{};
    DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    int Total = 3;
    
    std::unordered_map<int, int> QueueIndexMap;
    int GraphicsQueueIndexStart = QueueIndexMap[VulkanContext->GraphicsQueueFamilyIndex];
    QueueIndexMap[VulkanContext->GraphicsQueueFamilyIndex] += 1;
    
    int PresentQueueIndex = QueueIndexMap[VulkanContext->PresentQueueFamilyIndex];
    QueueIndexMap[VulkanContext->PresentQueueFamilyIndex] += 1;

    int ComputeQueueIndex = QueueIndexMap[VulkanContext->ComputeQueueFamilyIndex];
    QueueIndexMap[VulkanContext->ComputeQueueFamilyIndex] += 1;

    std::vector<VkDeviceQueueCreateInfo> QueueInfos;
    std::vector<float> Priorities(Total, 1.0f);
    
    for(auto& QueueFamily : QueueIndexMap)
    {
        VkDeviceQueueCreateInfo QueueInfo {};
        QueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueInfo.queueFamilyIndex = QueueFamily.first;
        QueueInfo.pQueuePriorities = Priorities.data();
        QueueInfo.queueCount = QueueFamily.second;

        QueueInfos.push_back(std::move(QueueInfo));
    }

    VkPhysicalDeviceFeatures DeviceFeatures{};
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.geometryShader = VK_TRUE;
    DeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
    DeviceFeatures.independentBlend = VK_TRUE;
    DeviceFeatures.fillModeNonSolid = VK_TRUE;
    DeviceFeatures.wideLines = VK_TRUE;

    DeviceInfo.queueCreateInfoCount = (uint32_t)QueueInfos.size();
    DeviceInfo.pQueueCreateInfos = QueueInfos.data();
    DeviceInfo.enabledExtensionCount = (uint32_t)DeviceExtensionRequires.size();
    DeviceInfo.ppEnabledExtensionNames = DeviceExtensionRequires.data();
    DeviceInfo.pEnabledFeatures = nullptr;
    DeviceInfo.enabledLayerCount = 0;

    /*
     * Don't ask me why this
     * Validation layer will gave mesh shader ext not enabled if use ext
     * But actually both will run, use _NV will give you less validation
     * You will still receive a validation that you shall gave a vertex shader in pipeline, that's
     * an validation layer bug
     */
    VkPhysicalDeviceMeshShaderFeaturesEXT MeshShaderFeatureEnable {};
    MeshShaderFeatureEnable.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
    MeshShaderFeatureEnable.meshShader = VK_TRUE;
    MeshShaderFeatureEnable.taskShader = VK_TRUE;

    VkPhysicalDeviceFeatures2 DeviceFeature2 {};
    DeviceFeature2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    DeviceFeature2.features = DeviceFeatures;
    DeviceFeature2.pNext = &MeshShaderFeatureEnable;

    DeviceInfo.pNext = &DeviceFeature2;

    VkPhysicalDeviceVulkan12Features Features12 {};
    Features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    Features12.shaderInt8 = VK_TRUE;
    Features12.storageBuffer8BitAccess = VK_TRUE;
    MeshShaderFeatureEnable.pNext = &Features12;
    
    VkResult CreateResult = vkCreateDevice(VulkanContext->PhysicalDevice, &DeviceInfo, nullptr, &VulkanContext->Device);
    assert(CreateResult == VK_SUCCESS);
    

    vkGetDeviceQueue(
        VulkanContext->Device,
        VulkanContext->GraphicsQueueFamilyIndex,
        GraphicsQueueIndexStart,
        &VulkanContext->GraphicsQueue);
    assert(VulkanContext->GraphicsQueue);
    
    vkGetDeviceQueue(
        VulkanContext->Device,
        VulkanContext->PresentQueueFamilyIndex,
        PresentQueueIndex,
        &VulkanContext->PresentQueue);
    assert(VulkanContext->PresentQueue);

    vkGetDeviceQueue(
        VulkanContext->Device,
        VulkanContext->ComputeQueueFamilyIndex,
        ComputeQueueIndex,
        &VulkanContext->ComputeQueue);
    assert(VulkanContext->ComputeQueue);
}

void VulkanRHI::LoadDeviceExtensionFunctions()
{
    VulkanEXT.VKCmdDrawMeshTaskEXT = VKTool::LoadDeviceFunction<PFN_vkCmdDrawMeshTasksEXT>(VulkanContext, "vkCmdDrawMeshTasksEXT");
}

void VulkanRHI::CreateSwapchain()
{
    LOG_DEBUG_FUNCTION("Creating Swapchian");
    
    VkSurfaceCapabilitiesKHR SurfaceCapabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanContext->PhysicalDevice, VulkanContext->WindowSurface, &SurfaceCapabilities);
    int SwapchainFormatIndex = 0;
    std::vector<VkSurfaceFormatKHR>& DeviceFormats = VulkanContext->SurfaceFormats;
    for(size_t i = 0; i < DeviceFormats.size(); ++i)
    {
        if(DeviceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && DeviceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SwapchainFormatIndex = (int)i;
            break;
        }
    }

    VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& Present : VulkanContext->PresentMods) {
        if(Present == VK_PRESENT_MODE_MAILBOX_KHR) {
            PresentMode = Present;
            break;
        }
    }

    WindowSize WinSize = GetWindowProxy().GetWindowSize();
    VkExtent2D Extend = {(uint32_t)WinSize.Width, (uint32_t)WinSize.Height};

    if(SurfaceCapabilities.currentExtent.width == UINT32_MAX) {
        Extend.width = std::max(SurfaceCapabilities.minImageExtent.width, std::min(Extend.width, SurfaceCapabilities.maxImageExtent.width));
        Extend.width = std::max(SurfaceCapabilities.minImageExtent.height, std::min(Extend.height, SurfaceCapabilities.maxImageExtent.height));
    }

    uint32_t ImageCount = std::min(SurfaceCapabilities.maxImageCount, std::max(3u,SurfaceCapabilities.minImageCount+1));

    VkSwapchainCreateInfoKHR SwapchainInfo {};
    SwapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainInfo.surface = VulkanContext->WindowSurface;

    SwapchainInfo.minImageCount = ImageCount;
    SwapchainInfo.imageFormat = VulkanContext->SurfaceFormats.at(SwapchainFormatIndex).format;
    SwapchainInfo.imageColorSpace = VulkanContext->SurfaceFormats.at(SwapchainFormatIndex).colorSpace;
    SwapchainInfo.imageExtent = Extend;
    SwapchainInfo.imageArrayLayers = 1;
    SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t QueueFamilies[] = {(uint32_t)VulkanContext->PresentQueueFamilyIndex, (uint32_t)VulkanContext->GraphicsQueueFamilyIndex};
    if(VulkanContext->PresentQueueFamilyIndex != VulkanContext->GraphicsQueueFamilyIndex)
    {
        SwapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapchainInfo.queueFamilyIndexCount = 2;
        SwapchainInfo.pQueueFamilyIndices = QueueFamilies;
    }
    else
    {
        SwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    SwapchainInfo.preTransform = SurfaceCapabilities.currentTransform;
    SwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainInfo.presentMode = PresentMode;
    SwapchainInfo.clipped = VK_TRUE;
    SwapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult Result = vkCreateSwapchainKHR(VulkanContext->Device, &SwapchainInfo, nullptr, &VulkanContext->Swapchain);
    assert(Result == VK_SUCCESS);

    VulkanContext->Format = VulkanContext->SurfaceFormats.at(SwapchainFormatIndex).format;
    VulkanContext->Extend = Extend;
    VulkanContext->SwapchainImages = VKTool::GetPropertiesArray2P<VkImage>(VulkanContext->Device, VulkanContext->Swapchain, vkGetSwapchainImagesKHR);
    VulkanContext->DefaultScissor = {{0,0}, Extend};
    VulkanContext->DefaultViewport = {0.0,0.0,(float)Extend.width, -(float)Extend.height, 0.0f, 1.0f};
}

void VulkanRHI::CreateSwapchainImageView()
{
    LOG_DEBUG_FUNCTION("Creating image views");
    
    VulkanContext->ImageViews.resize(VulkanContext->SwapchainImages.size());
    for(size_t i=0; i<VulkanContext->SwapchainImages.size(); ++i)
    {
        VulkanContext->ImageViews[i] = VKTool::CreateImageView(
            VulkanContext,
            VulkanContext->SwapchainImages[i],
            VulkanContext->Format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            1,
            1);
        assert(VulkanContext->ImageViews[i]);
    }
}

void VulkanRHI::CreateDefaultAttachments()
{
    DefaultAttachments.resize(VulkanContext->SwapchainImages.size(), nullptr);
    for(size_t i=0; i<VulkanContext->SwapchainImages.size(); ++i)
    {
        DefaultAttachments[i] = new VulkanRHIRenderImageAttachment{};
        DefaultAttachments[i]->AttachmentType = RenderImageAttachmentType_SwapChain;
        DefaultAttachments[i]->AttachmentImage = new VulkanRHITexImage
        {
            {VulkanContext->Extend.width, VulkanContext->Extend.height, 1, 1},
            VulkanContext->SwapchainImages[i], nullptr
        };
        DefaultAttachments[i]->DefaultView = new VulkanRHITexImageView
        {
            {DefaultAttachments[i]->AttachmentImage},
            VulkanContext->ImageViews[i]
        };
    }
}

void VulkanRHI::CreateVmaAllocator()
{
    LOG_DEBUG_FUNCTION("Creating Vma allocator");
    VmaVulkanFunctions VulkanFunctions    = {};
    VulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    VulkanFunctions.vkGetDeviceProcAddr   = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo AllocatorCreateInfo = {};
    AllocatorCreateInfo.vulkanApiVersion       = COMPILE_VULKAN_VERSION;
    AllocatorCreateInfo.physicalDevice         = VulkanContext->PhysicalDevice;
    AllocatorCreateInfo.device                 = VulkanContext->Device;
    AllocatorCreateInfo.instance               = VulkanContext->Instance;
    AllocatorCreateInfo.pVulkanFunctions       = &VulkanFunctions;
    vmaCreateAllocator(&AllocatorCreateInfo, &VulkanContext->Allocator);
}

void VulkanRHI::CreateCommandPool()
{
    LOG_DEBUG_FUNCTION("Creating command pool");
    
    VkCommandPoolCreateInfo DrawCommandPoolCreateInfo {};
    DrawCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    DrawCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    DrawCommandPoolCreateInfo.queueFamilyIndex = VulkanContext->GraphicsQueueFamilyIndex;

    VkResult PoolCreateResult = vkCreateCommandPool(VulkanContext->Device, &DrawCommandPoolCreateInfo, nullptr, &VulkanContext->DrawCommandPool);
    assert(PoolCreateResult == VK_SUCCESS);


    VkCommandPoolCreateInfo ComputeCommandPoolCreateInfo {};
    ComputeCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ComputeCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ComputeCommandPoolCreateInfo.queueFamilyIndex = VulkanContext->ComputeQueueFamilyIndex;

    PoolCreateResult = vkCreateCommandPool(VulkanContext->Device, &ComputeCommandPoolCreateInfo, nullptr, &VulkanContext->ComputeCommandPool);
    assert(PoolCreateResult == VK_SUCCESS);

    VkCommandPoolCreateInfo SingleTimeCommandPoolCreateInfo {};
    SingleTimeCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    SingleTimeCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    SingleTimeCommandPoolCreateInfo.queueFamilyIndex = VulkanContext->GraphicsQueueFamilyIndex;
    PoolCreateResult = vkCreateCommandPool(VulkanContext->Device, &SingleTimeCommandPoolCreateInfo, nullptr, &VulkanContext->SingleTimeCommandPool);
    assert(PoolCreateResult == VK_SUCCESS);
    
}

void VulkanRHI::CreateCommandBuffers()
{
    LOG_DEBUG_FUNCTION("Creating command buffers");

    uint32_t DrawCmdBufferCount = 2u * 2u;
    VulkanContext->DrawCommandBuffers.resize(DrawCmdBufferCount);
    
    VkCommandBufferAllocateInfo CommandBufferInfo {};
    CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferInfo.commandPool = VulkanContext->DrawCommandPool;
    CommandBufferInfo.commandBufferCount = DrawCmdBufferCount;

    VkResult Result = vkAllocateCommandBuffers(VulkanContext->Device, &CommandBufferInfo, VulkanContext->DrawCommandBuffers.data());
    assert(Result == VK_SUCCESS);

    for(int i=0; i<(int)DrawCmdBufferCount; ++i)
    {
        DrawCommandsStatus.FreeQueue.push(i);
    }

    uint32_t ComputeCmdBufferCount = 2u * 2u;
    VulkanContext->ComputeCommandBuffers.resize(ComputeCmdBufferCount);
    
    VkCommandBufferAllocateInfo ComputeCommandBufferInfo {};
    ComputeCommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ComputeCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ComputeCommandBufferInfo.commandPool = VulkanContext->ComputeCommandPool;
    ComputeCommandBufferInfo.commandBufferCount = ComputeCmdBufferCount;
    
    Result = vkAllocateCommandBuffers(VulkanContext->Device, &ComputeCommandBufferInfo, VulkanContext->ComputeCommandBuffers.data());
    assert(Result == VK_SUCCESS);

    for(int i=0; i<(int)ComputeCmdBufferCount; ++i)
    {
        ComputeCommandsStatus.FreeQueue.push(i);
    }
}


void VulkanRHI::CreateSyncPrimitives()
{
    LOG_DEBUG_FUNCTION("Create sync primitives");
    
    VkSemaphoreCreateInfo SemaphoreInfo {};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkResult Result = vkCreateSemaphore(
        VulkanContext->Device,
        &SemaphoreInfo,
        nullptr,
        &VulkanContext->ImageRequireSemaphore);
    assert(Result == VK_SUCCESS);

    Result = vkCreateSemaphore(
        VulkanContext->Device,
        &SemaphoreInfo,
        nullptr,
        &VulkanContext->PresentSemaphore);
    assert(Result == VK_SUCCESS);
    
}

void VulkanRHI::CreateDescriptorPool()
{

    LOG_DEBUG_FUNCTION("Creating descriptor pool");
    
    //TODO: set max threshold by material mesh
    VkDescriptorPoolSize PoolSize[DESCRIPTOR_POOL_SIZE];
    PoolSize[0].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    PoolSize[0].descriptorCount = 32;
    PoolSize[1].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    PoolSize[1].descriptorCount = 3096;
    PoolSize[2].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSize[2].descriptorCount = 1024;
    PoolSize[3].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    PoolSize[3].descriptorCount = 4096; 
    PoolSize[4].type            = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    PoolSize[4].descriptorCount = 32;

    VkDescriptorPoolCreateInfo PoolInfo {};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = DESCRIPTOR_POOL_SIZE;
    PoolInfo.pPoolSizes = PoolSize;
    PoolInfo.maxSets = 2048 + 128;
    PoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VkResult Result = vkCreateDescriptorPool(VulkanContext->Device, &PoolInfo, nullptr, &VulkanContext->DescriptorPool);
    assert(Result == VK_SUCCESS);
}

void VulkanRHI::RecreateSwapChain()
{
    WindowSize Size = GetWindowProxy().GetWindowSize();
    while (Size.Width == 0 || Size.Height == 0)
    {
        Size = GetWindowProxy().GetWindowSize();
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(VulkanContext->Device);
    
    DestroySwapchainObjects();
    
    CreateSwapchain();
    CreateSwapchainImageView();
    CreateDefaultAttachments();
}

void VulkanRHI::DestroySwapchainObjects()
{
    for(auto Attachment : DefaultAttachments)
    {
        delete Attachment->DefaultView;
        delete Attachment->AttachmentImage;
        delete Attachment;
    }

    for(auto SwapchainImageView : VulkanContext->ImageViews)
    {
        vkDestroyImageView(VulkanContext->Device, SwapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(VulkanContext->Device, VulkanContext->Swapchain, nullptr);
}

void VulkanRHI::DestroyVulkanObjects()
{
    DestroySwapchainObjects();

    vkDestroySemaphore(VulkanContext->Device, VulkanContext->ImageRequireSemaphore, nullptr);
    vkDestroySemaphore(VulkanContext->Device, VulkanContext->PresentSemaphore, nullptr);
    
    vkDestroyDescriptorPool(VulkanContext->Device, VulkanContext->DescriptorPool, nullptr);
    vkDestroyCommandPool(VulkanContext->Device, VulkanContext->DrawCommandPool, nullptr);
    vkDestroyCommandPool(VulkanContext->Device, VulkanContext->ComputeCommandPool, nullptr);
    vkDestroyCommandPool(VulkanContext->Device, VulkanContext->SingleTimeCommandPool, nullptr);
    vmaDestroyAllocator(VulkanContext->Allocator);
    vkDestroyDevice(VulkanContext->Device, nullptr);
    vkDestroySurfaceKHR(VulkanContext->Instance, VulkanContext->WindowSurface, nullptr);
    if(VulkanContext->EnableDebug)
    {
        VulkanEXT.VKDstroyDebugUtilsMessengerEXT(VulkanContext->Instance, VulkanContext->DebugMessenger, nullptr);
    }
    vkDestroyInstance(VulkanContext->Instance, nullptr);
}


