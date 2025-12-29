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

    void bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    void bindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet descriptorSet,
                           VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
    void bindVertexBuffer(VkBuffer buffer, VkDeviceSize offset = 0);
    void bindIndexBuffer(VkBuffer buffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32, VkDeviceSize offset = 0);

    void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0);
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0);

    void copyBufferToImage(VkBuffer buffer, VkImage image,
                           uint32_t width, uint32_t height);
    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels = 1);

private:
    CommandPool* poolRef = nullptr;
    VkCommandBuffer buffer = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
