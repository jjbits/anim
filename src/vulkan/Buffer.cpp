#include "Buffer.hpp"

#include <stdexcept>
#include <cstring>

using namespace std;

namespace anim::vulkan {

Buffer::Buffer(Device& device, VkDeviceSize size,
               VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
    : deviceRef(&device)
    , bufferSize(size) {

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(deviceRef->allocator(), &bufferInfo, &allocInfo,
                        &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw runtime_error("Failed to create buffer");
    }
}

Buffer::~Buffer() {
    if (buffer != VK_NULL_HANDLE && deviceRef) {
        if (mappedData) {
            vmaUnmapMemory(deviceRef->allocator(), allocation);
        }
        vmaDestroyBuffer(deviceRef->allocator(), buffer, allocation);
    }
}

Buffer::Buffer(Buffer&& other) noexcept
    : deviceRef(other.deviceRef)
    , buffer(other.buffer)
    , allocation(other.allocation)
    , bufferSize(other.bufferSize)
    , mappedData(other.mappedData) {
    other.deviceRef = nullptr;
    other.buffer = VK_NULL_HANDLE;
    other.allocation = VK_NULL_HANDLE;
    other.bufferSize = 0;
    other.mappedData = nullptr;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        if (buffer != VK_NULL_HANDLE && deviceRef) {
            if (mappedData) {
                vmaUnmapMemory(deviceRef->allocator(), allocation);
            }
            vmaDestroyBuffer(deviceRef->allocator(), buffer, allocation);
        }

        deviceRef = other.deviceRef;
        buffer = other.buffer;
        allocation = other.allocation;
        bufferSize = other.bufferSize;
        mappedData = other.mappedData;

        other.deviceRef = nullptr;
        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.bufferSize = 0;
        other.mappedData = nullptr;
    }
    return *this;
}

void Buffer::upload(const void* data, size_t size) {
    void* mapped = map();
    memcpy(mapped, data, size);
    unmap();
}

void* Buffer::map() {
    if (!mappedData) {
        if (vmaMapMemory(deviceRef->allocator(), allocation, &mappedData) != VK_SUCCESS) {
            throw runtime_error("Failed to map buffer memory");
        }
    }
    return mappedData;
}

void Buffer::unmap() {
    if (mappedData) {
        vmaUnmapMemory(deviceRef->allocator(), allocation);
        mappedData = nullptr;
    }
}

} // namespace anim::vulkan
