#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vector>

struct VulkanExtFunctionSet
{
    PFN_vkCreateDebugUtilsMessengerEXT VKCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT VKDstroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkCmdDrawMeshTasksEXT VKCmdDrawMeshTaskEXT = nullptr;
};

class VulkanContext
{
    
public:
    
    VkInstance Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR WindowSurface = VK_NULL_HANDLE;

    std::vector<VkSurfaceFormatKHR> SurfaceFormats;
    std::vector<VkPresentModeKHR> PresentMods;

    int GraphicsQueueFamilyIndex = -1;
    int PresentQueueFamilyIndex = -1;
    int ComputeQueueFamilyIndex = -1;
    
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;

    VkQueue GraphicsQueue = VK_NULL_HANDLE;
    VkQueue PresentQueue = VK_NULL_HANDLE;
    VkQueue ComputeQueue = VK_NULL_HANDLE;

    VkSwapchainKHR Swapchain = VK_NULL_HANDLE;

    VkFormat Format {};
    VkExtent2D Extend {};
    
    //Both in RHI and render resource
    std::vector<VkImage> SwapchainImages;
    std::vector<VkImageView> ImageViews;

    VmaAllocator Allocator = VK_NULL_HANDLE;

    VulkanCommandPool* GraphicsPool;
    VulkanCommandPool* ComputePool;
    VulkanCommandPool* ImmediatePool;
    
    VkSemaphore ImageRequireSemaphore = VK_NULL_HANDLE;
    VkSemaphore PresentSemaphore = VK_NULL_HANDLE;

    VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

    uint32_t RHIThreadFrameCount = 0;
    
    bool EnableDebug = true;
    
};