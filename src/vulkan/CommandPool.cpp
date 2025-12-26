#include "CommandPool.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

CommandPool::CommandPool(Device& device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
    : deviceRef(&device) {
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.flags = flags;

    if (vkCreateCommandPool(deviceRef->handle(), &createInfo, nullptr, &pool) != VK_SUCCESS) {
        throw runtime_error("Failed to create command pool");
    }
}

CommandPool::~CommandPool() {
    if (deviceRef && pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(deviceRef->handle(), pool, nullptr);
    }
}

CommandPool::CommandPool(CommandPool&& other) noexcept
    : deviceRef(other.deviceRef)
    , pool(other.pool) {
    other.deviceRef = nullptr;
    other.pool = VK_NULL_HANDLE;
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept {
    if (this != &other) {
        if (deviceRef && pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(deviceRef->handle(), pool, nullptr);
        }

        deviceRef = other.deviceRef;
        pool = other.pool;

        other.deviceRef = nullptr;
        other.pool = VK_NULL_HANDLE;
    }
    return *this;
}

VkCommandBuffer CommandPool::allocateBuffer(VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = pool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer buffer;
    if (vkAllocateCommandBuffers(deviceRef->handle(), &allocInfo, &buffer) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate command buffer");
    }

    return buffer;
}

void CommandPool::freeBuffer(VkCommandBuffer buffer) {
    vkFreeCommandBuffers(deviceRef->handle(), pool, 1, &buffer);
}

void CommandPool::reset(VkCommandPoolResetFlags flags) {
    vkResetCommandPool(deviceRef->handle(), pool, flags);
}

} // namespace anim::vulkan
