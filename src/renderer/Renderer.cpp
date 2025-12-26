#include "Renderer.hpp"

#include <stdexcept>

using namespace std;

namespace anim::renderer {

Renderer::Renderer(vulkan::Device& device, vulkan::Swapchain& swapchain)
    : device_(&device)
    , swapchain_(&swapchain) {

    renderPass_ = make_unique<vulkan::RenderPass>(device, swapchain.imageFormat());
    createFramebuffers();

    commandPool_ = make_unique<vulkan::CommandPool>(device, device.graphicsQueueFamily());

    // Create command buffers per swapchain image (indexed by imageIndex_)
    uint32_t imageCount = swapchain.imageCount();
    for (uint32_t i = 0; i < imageCount; i++) {
        commandBuffers_.push_back(make_unique<vulkan::CommandBuffer>(*commandPool_));
    }

    // imageAvailable semaphores and fences: per frame-in-flight (indexed by currentFrame_)
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        imageAvailableSemaphores_.push_back(make_unique<vulkan::Semaphore>(device));
        inFlightFences_.push_back(make_unique<vulkan::Fence>(device, true));  // Start signaled
    }

    // renderFinished semaphores: per swapchain image (indexed by imageIndex_)
    // Safe to reuse when that image is re-acquired
    for (uint32_t i = 0; i < imageCount; i++) {
        renderFinishedSemaphores_.push_back(make_unique<vulkan::Semaphore>(device));
    }
}

Renderer::~Renderer() {
    if (device_) {
        device_->waitIdle();
    }
}

void Renderer::createFramebuffers() {
    framebuffers_.clear();
    for (auto view : swapchain_->imageViews()) {
        framebuffers_.push_back(
            make_unique<vulkan::Framebuffer>(*device_, *renderPass_, view, swapchain_->extent())
        );
    }
}

bool Renderer::beginFrame() {
    // Wait for this frame slot's previous work to complete
    inFlightFences_[currentFrame_]->wait();

    // Acquire next swapchain image using currentFrame_'s semaphore
    VkResult result = swapchain_->acquireNextImage(
        imageAvailableSemaphores_[currentFrame_]->handle(), &imageIndex_);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw runtime_error("Failed to acquire swapchain image");
    }

    // Reset fence only after we know we'll submit work
    inFlightFences_[currentFrame_]->reset();
    frameStarted_ = true;

    // Begin recording commands (command buffer indexed by imageIndex_)
    auto& cmd = *commandBuffers_[imageIndex_];
    cmd.reset();
    cmd.begin();
    cmd.beginRenderPass(
        renderPass_->handle(),
        framebuffers_[imageIndex_]->handle(),
        swapchain_->extent(),
        &clearColor_, 1);

    return true;
}

void Renderer::endFrame() {
    if (!frameStarted_) {
        return;
    }

    // Command buffer indexed by imageIndex_
    auto& cmd = *commandBuffers_[imageIndex_];
    cmd.endRenderPass();
    cmd.end();

    // imageAvailable indexed by currentFrame_, renderFinished indexed by imageIndex_
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores_[currentFrame_]->handle()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores_[imageIndex_]->handle()};
    VkCommandBuffer cmdBuf = cmd.handle();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(device_->graphicsQueue(), 1, &submitInfo, inFlightFences_[currentFrame_]->handle()) != VK_SUCCESS) {
        throw runtime_error("Failed to submit command buffer");
    }

    // Present the acquired image, waiting on imageIndex_'s renderFinished semaphore
    swapchain_->present(device_->presentQueue(), imageIndex_, renderFinishedSemaphores_[imageIndex_]->handle());

    frameStarted_ = false;
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::setClearColor(float r, float g, float b, float a) {
    clearColor_ = {{{r, g, b, a}}};
}

} // namespace anim::renderer
