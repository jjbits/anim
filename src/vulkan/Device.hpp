#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>
#include <optional>

using namespace std;

namespace anim::vulkan {

struct QueueFamilyIndices {
    optional<uint32_t> graphics;
    optional<uint32_t> present;

    bool isComplete() const { return graphics.has_value() && present.has_value(); }
};

class Device {
public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();

    // Non-copyable
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Movable
    Device(Device&& other) noexcept;
    Device& operator=(Device&& other) noexcept;

    VkDevice handle() const { return device; }
    VkPhysicalDevice physicalDevice() const { return physical; }
    VkQueue graphicsQueue() const { return graphicsQ; }
    VkQueue presentQueue() const { return presentQ; }
    uint32_t graphicsQueueFamily() const { return queueFamilies.graphics.value(); }
    uint32_t presentQueueFamily() const { return queueFamilies.present.value(); }
    VmaAllocator allocator() const { return vmaAllocator; }

    void waitIdle() const;

private:
    void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
    void createLogicalDevice(VkSurfaceKHR surface);
    void createAllocator(VkInstance instance);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQ = VK_NULL_HANDLE;
    VkQueue presentQ = VK_NULL_HANDLE;
    VmaAllocator vmaAllocator = VK_NULL_HANDLE;
    QueueFamilyIndices queueFamilies;
};

} // namespace anim::vulkan
