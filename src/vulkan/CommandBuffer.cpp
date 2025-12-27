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

} // namespace anim::vulkan
