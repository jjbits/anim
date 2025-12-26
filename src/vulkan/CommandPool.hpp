#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

using namespace std;

namespace anim::vulkan {

class CommandPool {
public:
    CommandPool(Device& device, uint32_t queueFamilyIndex,
                VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    ~CommandPool();

    // Non-copyable
    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;

    // Movable
    CommandPool(CommandPool&& other) noexcept;
    CommandPool& operator=(CommandPool&& other) noexcept;

    VkCommandPool handle() const { return pool; }
    Device& device() const { return *deviceRef; }

    VkCommandBuffer allocateBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void freeBuffer(VkCommandBuffer buffer);
    void reset(VkCommandPoolResetFlags flags = 0);

private:
    Device* deviceRef = nullptr;
    VkCommandPool pool = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
