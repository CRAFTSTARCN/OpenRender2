#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Util/Logger.h"

#include <cassert>

#include "VulkanRHITexture.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITextureView.h"

#define COMPILE_VULKAN_VERSION VK_API_VERSION_1_3
#define DESCRIPTOR_POOL_SIZE 7

void VulkanRHI::InitializeVulkan()
{
    CreateVulkanInstance();
    LoadExtensionFunctions();

    if(Context->EnableDebug)
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
    
    if(Context->EnableDebug)
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
    VkResult CreateInstanceResult = vkCreateInstance(&VulkanInstanceInfo, nullptr, &Context->Instance);
    assert(CreateInstanceResult == VK_SUCCESS);
}

void VulkanRHI::LoadExtensionFunctions()
{
    VulkanEXT.VKCreateDebugUtilsMessengerEXT =
        VKTool::LoadFunction<PFN_vkCreateDebugUtilsMessengerEXT>(
            Context,
            "vkCreateDebugUtilsMessengerEXT");
    VulkanEXT.VKDstroyDebugUtilsMessengerEXT =
        VKTool::LoadFunction<PFN_vkDestroyDebugUtilsMessengerEXT>(
            Context,
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
        Context->Instance,
        &DebugCreateInfo,
        nullptr,
        &Context->DebugMessenger);

    assert(CreateResult == VK_SUCCESS);
}

void VulkanRHI::CreateWindowSurface()
{
    LOG_DEBUG_FUNCTION("Creating window surface");
    VkResult CreateResult = glfwCreateWindowSurface(Context->Instance, WindowPtr, nullptr, &Context->WindowSurface);
    assert(CreateResult == VK_SUCCESS);
}

void VulkanRHI::PickPhysicalDevice()
{
    LOG_DEBUG_FUNCTION("Start picking physical device");
    std::vector<std::pair<size_t, VkPhysicalDevice>> RankedDevices = VKTool::GetRankedGPU(Context);

    auto TestGPU = [&](VkPhysicalDevice PhysicalDevice) -> bool
    {
        int GraphicsQueueIndex = VKTool::FindQueueFamilyByBit(PhysicalDevice, VK_QUEUE_GRAPHICS_BIT);
        int ComputeQueueIndex = VKTool::FindQueueFamilyByBit(PhysicalDevice, VK_QUEUE_COMPUTE_BIT);
        int PresentQueueIndex = VKTool::FindPresentQueueFamily(PhysicalDevice, Context);

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
            Context->WindowSurface,
            vkGetPhysicalDeviceSurfaceFormatsKHR);
        std::vector<VkPresentModeKHR> PresentModes = VKTool::GetPropertiesArray2P<VkPresentModeKHR>(
            PhysicalDevice,
            Context->WindowSurface,
            vkGetPhysicalDeviceSurfacePresentModesKHR);

        if(Formats.empty() || PresentModes.empty())
        {
            return false;
        }

        Context->SurfaceFormats = std::move(Formats);
        Context->PresentMods = std::move(PresentModes);
        Context->GraphicsQueueFamilyIndex = GraphicsQueueIndex;
        Context->PresentQueueFamilyIndex = PresentQueueIndex;
        Context->ComputeQueueFamilyIndex = ComputeQueueIndex;

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
            Context->PhysicalDevice = GPUPair.second;
        }
    }

    
    assert(Context->PhysicalDevice);
    
}

void VulkanRHI::CreateLogicalDevice()
{

    LOG_DEBUG_FUNCTION("Creating logical device");
    
    VkDeviceCreateInfo DeviceInfo{};
    DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    int Total = 3;
    
    std::unordered_map<int, int> QueueIndexMap;
    int GraphicsQueueIndexStart = QueueIndexMap[Context->GraphicsQueueFamilyIndex];
    QueueIndexMap[Context->GraphicsQueueFamilyIndex] += 1;
    
    int PresentQueueIndex = QueueIndexMap[Context->PresentQueueFamilyIndex];
    QueueIndexMap[Context->PresentQueueFamilyIndex] += 1;

    int ComputeQueueIndex = QueueIndexMap[Context->ComputeQueueFamilyIndex];
    QueueIndexMap[Context->ComputeQueueFamilyIndex] += 1;

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
    Features12.descriptorIndexing = VK_TRUE;
    Features12.scalarBlockLayout = VK_TRUE;
    MeshShaderFeatureEnable.pNext = &Features12;
    
    VkResult CreateResult = vkCreateDevice(Context->PhysicalDevice, &DeviceInfo, nullptr, &Context->Device);
    assert(CreateResult == VK_SUCCESS);
    

    vkGetDeviceQueue(
        Context->Device,
        Context->GraphicsQueueFamilyIndex,
        GraphicsQueueIndexStart,
        &Context->GraphicsQueue);
    assert(Context->GraphicsQueue);
    
    vkGetDeviceQueue(
        Context->Device,
        Context->PresentQueueFamilyIndex,
        PresentQueueIndex,
        &Context->PresentQueue);
    assert(Context->PresentQueue);

    vkGetDeviceQueue(
        Context->Device,
        Context->ComputeQueueFamilyIndex,
        ComputeQueueIndex,
        &Context->ComputeQueue);
    assert(Context->ComputeQueue);
}

void VulkanRHI::LoadDeviceExtensionFunctions()
{
    VulkanEXT.VKCmdDrawMeshTaskEXT = VKTool::LoadDeviceFunction<PFN_vkCmdDrawMeshTasksEXT>(Context, "vkCmdDrawMeshTasksEXT");
}

void VulkanRHI::CreateSwapchain()
{
    LOG_DEBUG_FUNCTION("Creating Swapchian");
    
    VkSurfaceCapabilitiesKHR SurfaceCapabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Context->PhysicalDevice, Context->WindowSurface, &SurfaceCapabilities);
    int SwapchainFormatIndex = 0;
    std::vector<VkSurfaceFormatKHR>& DeviceFormats = Context->SurfaceFormats;
    for(size_t i = 0; i < DeviceFormats.size(); ++i)
    {
        if(DeviceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && DeviceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SwapchainFormatIndex = (int)i;
            break;
        }
    }

    VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& Present : Context->PresentMods) {
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
    SwapchainInfo.surface = Context->WindowSurface;

    SwapchainInfo.minImageCount = ImageCount;
    SwapchainInfo.imageFormat = Context->SurfaceFormats.at(SwapchainFormatIndex).format;
    SwapchainInfo.imageColorSpace = Context->SurfaceFormats.at(SwapchainFormatIndex).colorSpace;
    SwapchainInfo.imageExtent = Extend;
    SwapchainInfo.imageArrayLayers = 1;
    SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t QueueFamilies[] = {(uint32_t)Context->PresentQueueFamilyIndex, (uint32_t)Context->GraphicsQueueFamilyIndex};
    if(Context->PresentQueueFamilyIndex != Context->GraphicsQueueFamilyIndex)
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

    VkResult Result = vkCreateSwapchainKHR(Context->Device, &SwapchainInfo, nullptr, &Context->Swapchain);
    assert(Result == VK_SUCCESS);

    Context->Format = Context->SurfaceFormats.at(SwapchainFormatIndex).format;
    Context->Extend = Extend;
    Context->SwapchainImages = VKTool::GetPropertiesArray2P<VkImage>(Context->Device, Context->Swapchain, vkGetSwapchainImagesKHR);
    Context->DefaultScissor = {{0,0}, Extend};
    Context->DefaultViewport = {0.0,0.0,(float)Extend.width, -(float)Extend.height, 0.0f, 1.0f};
}

void VulkanRHI::CreateSwapchainImageView()
{
    LOG_DEBUG_FUNCTION("Creating image views");
    
    Context->ImageViews.resize(Context->SwapchainImages.size());
    for(size_t i=0; i<Context->SwapchainImages.size(); ++i)
    {
        Context->ImageViews[i] = VKTool::CreateImageView(
            Context,
            Context->SwapchainImages[i],
            Context->Format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_VIEW_TYPE_2D,
            1,
            1);
        assert(Context->ImageViews[i]);
    }
}

void VulkanRHI::CreateDefaultAttachments()
{
    DefaultAttachments.resize(Context->SwapchainImages.size(), nullptr);
    for(size_t i=0; i<Context->SwapchainImages.size(); ++i)
    {
        DefaultAttachments[i] = new VulkanRHITexture{
            {
                {
                    Context->Extend.width,
                    Context->Extend.height,
                    GetSwapchainTextureFormat(),
                    TextureType_2D,
                    TextureUsageBit_ColorAttachment,
                    1,
                    1
                }},
            Context->SwapchainImages[i], nullptr
        };
        DefaultAttachments[i]->DefaultTextureView = new VulkanRHITextureView
        {
            {DefaultAttachments[i], GetSwapchainTextureFormat()},
            Context->ImageViews[i]
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
    AllocatorCreateInfo.physicalDevice         = Context->PhysicalDevice;
    AllocatorCreateInfo.device                 = Context->Device;
    AllocatorCreateInfo.instance               = Context->Instance;
    AllocatorCreateInfo.pVulkanFunctions       = &VulkanFunctions;
    vmaCreateAllocator(&AllocatorCreateInfo, &Context->Allocator);
}

void VulkanRHI::CreateCommandPool()
{
    LOG_DEBUG_FUNCTION("Creating command pool");

    Context->DrawCommandPool.resize(10);
    Context->ComputeCommandPool.resize(10);
    
    VkCommandPoolCreateInfo DrawCommandPoolCreateInfo {};
    DrawCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    DrawCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    DrawCommandPoolCreateInfo.queueFamilyIndex = Context->GraphicsQueueFamilyIndex;
    
    VkResult PoolCreateResult;
    for(int i = 0; i < 10; ++i)
    {
        PoolCreateResult = vkCreateCommandPool(Context->Device, &DrawCommandPoolCreateInfo, nullptr, &Context->DrawCommandPool[i]);
        assert(PoolCreateResult == VK_SUCCESS);
    }


    VkCommandPoolCreateInfo ComputeCommandPoolCreateInfo {};
    ComputeCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ComputeCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ComputeCommandPoolCreateInfo.queueFamilyIndex = Context->ComputeQueueFamilyIndex;

    for(int i = 0; i < 10; ++i)
    {
        PoolCreateResult = vkCreateCommandPool(Context->Device, &ComputeCommandPoolCreateInfo, nullptr, &Context->ComputeCommandPool[i]);
        assert(PoolCreateResult == VK_SUCCESS);
    }

    VkCommandPoolCreateInfo SingleTimeCommandPoolCreateInfo {};
    SingleTimeCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    SingleTimeCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    SingleTimeCommandPoolCreateInfo.queueFamilyIndex = Context->GraphicsQueueFamilyIndex;
    PoolCreateResult = vkCreateCommandPool(Context->Device, &SingleTimeCommandPoolCreateInfo, nullptr, &Context->SingleTimeCommandPool);
    assert(PoolCreateResult == VK_SUCCESS);
    
}

void VulkanRHI::CreateCommandBuffers()
{
    LOG_DEBUG_FUNCTION("Creating command buffers");

    uint32_t DrawCmdBufferCount = 10;
    Context->DrawCommandBuffers.resize(DrawCmdBufferCount);
    
    VkCommandBufferAllocateInfo CommandBufferInfo {};
    CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferInfo.commandBufferCount = 1;

    VkResult Result;
    for(int i = 0; i < 10; ++i)
    {
        CommandBufferInfo.commandPool = Context->DrawCommandPool[i]; 
        Result = vkAllocateCommandBuffers(Context->Device, &CommandBufferInfo, &Context->DrawCommandBuffers[i]);
        assert(Result == VK_SUCCESS);
    }

    for(int i=0; i<(int)DrawCmdBufferCount; ++i)
    {
        DrawCommandsStatus.FreeQueue.push(i);
    }

    uint32_t ComputeCmdBufferCount = 10;
    Context->ComputeCommandBuffers.resize(ComputeCmdBufferCount);
    
    VkCommandBufferAllocateInfo ComputeCommandBufferInfo {};
    ComputeCommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ComputeCommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ComputeCommandBufferInfo.commandBufferCount = 1;

    for(int i = 0; i < 10; ++i)
    {
        ComputeCommandBufferInfo.commandPool = Context->ComputeCommandPool[i]; 
        Result = vkAllocateCommandBuffers(Context->Device, &ComputeCommandBufferInfo, &Context->ComputeCommandBuffers[i]);
        assert(Result == VK_SUCCESS);
    }
    
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
        Context->Device,
        &SemaphoreInfo,
        nullptr,
        &Context->ImageRequireSemaphore);
    assert(Result == VK_SUCCESS);

    Result = vkCreateSemaphore(
        Context->Device,
        &SemaphoreInfo,
        nullptr,
        &Context->PresentSemaphore);
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
    
    vkDeviceWaitIdle(Context->Device);
    
    DestroySwapchainObjects();
    
    CreateSwapchain();
    CreateSwapchainImageView();
    CreateDefaultAttachments();
}

void VulkanRHI::DestroySwapchainObjects()
{
    for(auto Attachment : DefaultAttachments)
    {
        delete Attachment->DefaultTextureView;
        delete Attachment;
    }

    for(auto SwapchainImageView : Context->ImageViews)
    {
        vkDestroyImageView(Context->Device, SwapchainImageView, nullptr);
    }

    vkDestroySwapchainKHR(Context->Device, Context->Swapchain, nullptr);
}

void VulkanRHI::DestroyVulkanObjects()
{
    DestroySwapchainObjects();

    vkDestroySemaphore(Context->Device, Context->ImageRequireSemaphore, nullptr);
    vkDestroySemaphore(Context->Device, Context->PresentSemaphore, nullptr);
    
    for(int i = 0; i < 10; ++i)
    {
        vkDestroyCommandPool(Context->Device, Context->DrawCommandPool[i], nullptr);
        vkDestroyCommandPool(Context->Device, Context->ComputeCommandPool[i], nullptr);
    }
    
    vkDestroyCommandPool(Context->Device, Context->SingleTimeCommandPool, nullptr);
    vmaDestroyAllocator(Context->Allocator);
    vkDestroyDevice(Context->Device, nullptr);
    vkDestroySurfaceKHR(Context->Instance, Context->WindowSurface, nullptr);
    if(Context->EnableDebug)
    {
        VulkanEXT.VKDstroyDebugUtilsMessengerEXT(Context->Instance, Context->DebugMessenger, nullptr);
    }
    vkDestroyInstance(Context->Instance, nullptr);
}


