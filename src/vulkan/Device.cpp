#define VMA_IMPLEMENTATION
#include "Device.hpp"

#include <iostream>
#include <stdexcept>
#include <set>

using namespace std;

namespace anim::vulkan {

const vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
    "VK_KHR_portability_subset",
#endif
};

Device::Device(VkInstance instance, VkSurfaceKHR surface) {
    pickPhysicalDevice(instance, surface);
    createLogicalDevice(surface);
    createAllocator(instance);
    cout << "Vulkan device created" << endl;
}

Device::~Device() {
    if (vmaAllocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(vmaAllocator);
    }
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
    }
}

Device::Device(Device&& other) noexcept
    : physical(other.physical)
    , device(other.device)
    , graphicsQ(other.graphicsQ)
    , presentQ(other.presentQ)
    , vmaAllocator(other.vmaAllocator)
    , queueFamilies(other.queueFamilies) {
    other.physical = VK_NULL_HANDLE;
    other.device = VK_NULL_HANDLE;
    other.graphicsQ = VK_NULL_HANDLE;
    other.presentQ = VK_NULL_HANDLE;
    other.vmaAllocator = VK_NULL_HANDLE;
}

Device& Device::operator=(Device&& other) noexcept {
    if (this != &other) {
        if (vmaAllocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(vmaAllocator);
        }
        if (device != VK_NULL_HANDLE) {
            vkDestroyDevice(device, nullptr);
        }

        physical = other.physical;
        device = other.device;
        graphicsQ = other.graphicsQ;
        presentQ = other.presentQ;
        vmaAllocator = other.vmaAllocator;
        queueFamilies = other.queueFamilies;

        other.physical = VK_NULL_HANDLE;
        other.device = VK_NULL_HANDLE;
        other.graphicsQ = VK_NULL_HANDLE;
        other.presentQ = VK_NULL_HANDLE;
        other.vmaAllocator = VK_NULL_HANDLE;
    }
    return *this;
}

void Device::waitIdle() const {
    vkDeviceWaitIdle(device);
}

void Device::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw runtime_error("Failed to find GPUs with Vulkan support");
    }

    vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& dev : devices) {
        if (isDeviceSuitable(dev, surface)) {
            physical = dev;
            break;
        }
    }

    if (physical == VK_NULL_HANDLE) {
        throw runtime_error("Failed to find a suitable GPU");
    }

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physical, &props);
    cout << "Selected GPU: " << props.deviceName << endl;
}

void Device::createLogicalDevice(VkSurfaceKHR surface) {
    queueFamilies = findQueueFamilies(physical, surface);

    set<uint32_t> uniqueQueueFamilies = {
        queueFamilies.graphics.value(),
        queueFamilies.present.value()
    };

    vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physical, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(device, queueFamilies.graphics.value(), 0, &graphicsQ);
    vkGetDeviceQueue(device, queueFamilies.present.value(), 0, &presentQ);
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice dev, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);

    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &presentSupport);
        if (presentSupport) {
            indices.present = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}

bool Device::isDeviceSuitable(VkPhysicalDevice dev, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(dev, surface);

    bool extensionsSupported = checkDeviceExtensionSupport(dev);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr);

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentModeCount, nullptr);

        swapChainAdequate = formatCount > 0 && presentModeCount > 0;
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice dev) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);

    vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, availableExtensions.data());

    set<string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Device::createAllocator(VkInstance instance) {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = physical;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

    if (vmaCreateAllocator(&allocatorInfo, &vmaAllocator) != VK_SUCCESS) {
        throw runtime_error("Failed to create VMA allocator");
    }
}

} // namespace anim::vulkan
