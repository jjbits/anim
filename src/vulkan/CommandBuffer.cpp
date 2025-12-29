#include "CommandBuffer.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

CommandBuffer::CommandBuffer(CommandPool& pool, VkCommandBufferLevel level)
    : poolRef(&pool) {
    buffer = pool.allocateBuffer(level);
}

CommandBuffer::~CommandBuffer() {
    if (poolRef && buffer != VK_NULL_HANDLE) {
        poolRef->freeBuffer(buffer);
    }
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    : poolRef(other.poolRef)
    , buffer(other.buffer) {
    other.poolRef = nullptr;
    other.buffer = VK_NULL_HANDLE;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
    if (this != &other) {
        if (poolRef && buffer != VK_NULL_HANDLE) {
            poolRef->freeBuffer(buffer);
        }

        poolRef = other.poolRef;
        buffer = other.buffer;

        other.poolRef = nullptr;
        other.buffer = VK_NULL_HANDLE;
    }
    return *this;
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flags;

    if (vkBeginCommandBuffer(buffer, &beginInfo) != VK_SUCCESS) {
        throw runtime_error("Failed to begin command buffer");
    }
}

void CommandBuffer::end() {
    if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
        throw runtime_error("Failed to end command buffer");
    }
}

void CommandBuffer::reset(VkCommandBufferResetFlags flags) {
    vkResetCommandBuffer(buffer, flags);
}

void CommandBuffer::beginRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer,
                                     VkExtent2D extent, const VkClearValue* clearValues,
                                     uint32_t clearValueCount, VkSubpassContents contents) {
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = renderPass;
    beginInfo.framebuffer = framebuffer;
    beginInfo.renderArea.offset = {0, 0};
    beginInfo.renderArea.extent = extent;
    beginInfo.clearValueCount = clearValueCount;
    beginInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(buffer, &beginInfo, contents);
}

void CommandBuffer::endRenderPass() {
    vkCmdEndRenderPass(buffer);
}

void CommandBuffer::setViewport(float x, float y, float width, float height,
                                 float minDepth, float maxDepth) {
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;

    vkCmdSetViewport(buffer, 0, 1, &viewport);
}

void CommandBuffer::setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    VkRect2D scissor{};
    scissor.offset = {x, y};
    scissor.extent = {width, height};

    vkCmdSetScissor(buffer, 0, 1, &scissor);
}

void CommandBuffer::bindPipeline(VkPipeline pipeline, VkPipelineBindPoint bindPoint) {
    vkCmdBindPipeline(buffer, bindPoint, pipeline);
}

void CommandBuffer::bindDescriptorSet(VkPipelineLayout layout, VkDescriptorSet descriptorSet,
                                       VkPipelineBindPoint bindPoint) {
    vkCmdBindDescriptorSets(buffer, bindPoint, layout, 0, 1, &descriptorSet, 0, nullptr);
}

void CommandBuffer::bindVertexBuffer(VkBuffer vertexBuffer, VkDeviceSize offset) {
    VkBuffer buffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {offset};
    vkCmdBindVertexBuffers(buffer, 0, 1, buffers, offsets);
}

void CommandBuffer::bindIndexBuffer(VkBuffer indexBuffer, VkIndexType indexType, VkDeviceSize offset) {
    vkCmdBindIndexBuffer(buffer, indexBuffer, offset, indexType);
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount,
                          uint32_t firstVertex, uint32_t firstInstance) {
    vkCmdDraw(buffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                  uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    vkCmdDrawIndexed(buffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage image,
                                       uint32_t width, uint32_t height) {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(buffer, srcBuffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CommandBuffer::transitionImageLayout(VkImage image, VkFormat format,
                                           VkImageLayout oldLayout, VkImageLayout newLayout,
                                           uint32_t mipLevels) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw runtime_error("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(buffer, srcStage, dstStage, 0,
                         0, nullptr, 0, nullptr, 1, &barrier);
}

} // namespace anim::vulkan
