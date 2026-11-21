#include <Core/Core.h>
#include "VulkanContext.h"
#include <cstring>
#include <Core/Logger.h>
#include <set>
#include <cstdlib>
#include <cstring>
#if defined(BUN_PLATFORM_WINDOWS)
    #include <Windows.h>
    #include <vulkan/vulkan_win32.h>
#elif defined(BUN_PLATFORM_LINUX)
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
#endif

namespace Bun::RHI
{

    VulkanContext::VulkanContext() = default;
    
    void VulkanContext::Init(void* windowHandle)
    {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface(windowHandle);
        PickPhysicalDevice();
        CreateLogicalDevice();

        BUN_INFO("Vulkan context initialized.");
    }

    void VulkanContext::Shutdown()
    {
        vkDestroyDevice(m_Device, nullptr);

        if (m_ValidationActive)
            DestroyDebugMessenger();

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);

        BUN_INFO("Vulkan context destroyed.");
    }

    void VulkanContext::CreateInstance()
    {
        m_ValidationActive = false;
        if (k_EnableValidation)
        {
            if (!m_LayerPath.empty())
            {
        #if defined(BUN_PLATFORM_WINDOWS)
                SetEnvironmentVariableA("VK_LAYER_PATH", m_LayerPath.c_str());
        #else
                setenv("VK_LAYER_PATH", m_LayerPath.c_str(), 1);
        #endif
            }

            u32 layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> layers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

            for (const auto& layer : layers)
                if (strcmp(layer.layerName, k_ValidationLayer) == 0) { m_ValidationActive = true; break; }

            if (!m_ValidationActive)
                BUN_WARN("Validation layer not found - running without it. Run Setup to install it.");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Bun Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.pEngineName = "Bun";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
    #if defined(BUN_PLATFORM_WINDOWS)
            "VK_KHR_win32_surface",
    #elif defined(BUN_PLATFORM_LINUX)
            "VK_KHR_xlib_surface",
    #elif defined(BUN_PLATFORM_MACOS)
            "VK_MVK_macos_surface",
    #endif
        };

        if (m_ValidationActive)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (u32)extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
            BUN_FATAL("Failed to create Vulkan instance.");
        else
            BUN_INFO("Vulkan instance created successfully.");
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* userData)
    {
        BUN_UNUSED(type);
        BUN_UNUSED(userData);

        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            BUN_ERROR(std::string("[VULKAN] ") + data->pMessage);
        else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            BUN_WARN(std::string("[VULKAN] ") + data->pMessage);
        else
            BUN_INFO(std::string("[VULKAN] ") + data->pMessage);

        return VK_FALSE;
    }

    void VulkanContext::SetupDebugMessenger()
    {
        if (!m_ValidationActive) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = DebugCallback;

            auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr (m_Instance, "vkCreateDebugUtilsMessengerEXT");

            if (func && func(m_Instance, &createInfo, nullptr, &m_DebugMessenger) == VK_SUCCESS)
                BUN_INFO("Vulkan validation layers enabled.");
            else
                BUN_WARN("Failed to setup Vulkan debug messenger.");
    }     
    
    void VulkanContext::DestroyDebugMessenger()
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr (m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func)
            func(m_Instance, m_DebugMessenger, nullptr);
    }

    void VulkanContext::CreateSurface(void* windowHandle)
    {
    #if defined(BUN_PLATFORM_WINDOWS)
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = (HWND)windowHandle;
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
            BUN_FATAL("Failed to create Win32 Vulkan surface!");
    #elif defined(BUN_PLATFORM_LINUX)
        struct LinuxWindowHandle { void* display; unsigned long window; };
        auto* handle = (LinuxWindowHandle*)windowHandle;

        VkXlibSurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        createInfo.dpy = (Display*)handle->display;
        createInfo.window = handle->window;

        if (vkCreateXlibSurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
            BUN_FATAL("Failed to create linux Vulkan Surface!");
    #endif

        BUN_INFO("Vulkan surface created.");
    }

    void VulkanContext::PickPhysicalDevice()
    {
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            BUN_FATAL("No Vulkan-capable GPU found!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            u32 queueCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
            std::vector<VkQueueFamilyProperties> queues(queueCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());

            bool hasGraphics = false;
            bool hasPresent = false;

            for (u32 i = 0; i < queueCount; i++)
            {
                if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    m_GraphicsQueueFamily = i;
                    hasGraphics = true;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
                if (presentSupport)
                {
                    m_PresentQueueFamily = i;
                    hasPresent = true;
                }
            }

            if (hasGraphics && hasPresent)
            {
                m_PhysicalDevice = device;

                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(device, &props);
                BUN_INFO(std::string("GPU selected: ") + props.deviceName);
                return;
            }
        }

        BUN_FATAL("No suitable GPU found!");
    }

    void VulkanContext::CreateLogicalDevice()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<u32> uniqueQueueFamilies = {
            m_GraphicsQueueFamily,
            m_PresentQueueFamily
        };

        f32 queuePriority = 1.0f;
        for (u32 queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = (u32)ArrayCount(k_DeviceExtensions);
        createInfo.ppEnabledExtensionNames = k_DeviceExtensions;

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS)
            BUN_FATAL("Failed to create Vulkan logical device!");

        vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);

        BUN_INFO("Vulkan logical device created.");
    }

    void VulkanContext::ImmediateSubmit(const std::function<void(VkCommandBuffer)>& record)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_GraphicsQueueFamily;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        VkCommandPool pool = VK_NULL_HANDLE;
        vkCreateCommandPool(m_Device, &poolInfo, nullptr, &pool);

        VkCommandBufferAllocateInfo alloc{};
        alloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc.commandPool = pool;
        alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc.commandBufferCount = 1;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        vkAllocateCommandBuffers(m_Device, &alloc, &cmd);

        VkCommandBufferBeginInfo begin{};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &begin);
        record(cmd);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        vkQueueSubmit(m_GraphicsQueue, 1, &submit, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        vkDestroyCommandPool(m_Device, pool, nullptr);
    }
}