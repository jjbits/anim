#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

using namespace std;

namespace anim::vulkan {

class Buffer {
public:
    Buffer(Device& device, VkDeviceSize size,
           VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
    ~Buffer();

    // Non-copyable
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Movable
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    VkBuffer handle() const { return buffer; }
    VkDeviceSize size() const { return bufferSize; }

    void upload(const void* data, size_t size);
    void* map();
    void unmap();

private:
    Device* deviceRef = nullptr;
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkDeviceSize bufferSize = 0;
    void* mappedData = nullptr;
};

} // namespace anim::vulkan
