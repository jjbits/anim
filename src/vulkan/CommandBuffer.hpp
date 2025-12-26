#pragma once

#include "CommandPool.hpp"

#include <vulkan/vulkan.h>

using namespace std;

namespace anim::vulkan {

class CommandBuffer {
public:
    CommandBuffer(CommandPool& pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    ~CommandBuffer();

    // Non-copyable
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    // Movable
    CommandBuffer(CommandBuffer&& other) noexcept;
    CommandBuffer& operator=(CommandBuffer&& other) noexcept;

    VkCommandBuffer handle() const { return buffer; }

    void begin(VkCommandBufferUsageFlags flags = 0);
    void end();
    void reset(VkCommandBufferResetFlags flags = 0);

    void beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer,
                         VkExtent2D extent, const VkClearValue* clearValues,
                         uint32_t clearValueCount,
                         VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
    void endRenderPass();

    void setViewport(float x, float y, float width, float height,
                     float minDepth = 0.0f, float maxDepth = 1.0f);
    void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

private:
    CommandPool* poolRef = nullptr;
    VkCommandBuffer buffer = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
