#pragma once

#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Sync.hpp"
#include "vulkan/CommandPool.hpp"
#include "vulkan/CommandBuffer.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/Framebuffer.hpp"

#include <vector>
#include <memory>

using namespace std;

namespace anim::renderer {


class Renderer {
public:
    Renderer(vulkan::Device& device, vulkan::Swapchain& swapchain);
    ~Renderer();

    // Non-copyable
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // Frame management
    bool beginFrame();
    void endFrame();

    // Accessors
    vulkan::CommandBuffer& commandBuffer() { return *commandBuffers_[imageIndex_]; }
    vulkan::RenderPass& renderPass() { return *renderPass_; }
    uint32_t currentImageIndex() const { return imageIndex_; }

    void setClearColor(float r, float g, float b, float a = 1.0f);

private:
    void createFramebuffers();

    vulkan::Device* device_;
    vulkan::Swapchain* swapchain_;

    unique_ptr<vulkan::RenderPass> renderPass_;
    vector<unique_ptr<vulkan::Framebuffer>> framebuffers_;
    unique_ptr<vulkan::CommandPool> commandPool_;
    vector<unique_ptr<vulkan::CommandBuffer>> commandBuffers_;

    // Per-frame-in-flight sync objects (indexed by currentFrame_)
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    vector<unique_ptr<vulkan::Semaphore>> imageAvailableSemaphores_;
    vector<unique_ptr<vulkan::Semaphore>> renderFinishedSemaphores_;
    vector<unique_ptr<vulkan::Fence>> inFlightFences_;

    uint32_t currentFrame_ = 0;  // Cycles 0 to MAX_FRAMES_IN_FLIGHT-1, indexes sync objects
    uint32_t imageIndex_ = 0;    // Set by acquireNextImage, indexes framebuffers
    bool frameStarted_ = false;
    VkClearValue clearColor_ = {{{0.39f, 0.58f, 0.93f, 1.0f}}};  // Cornflower blue
};

} // namespace anim::renderer
