#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

using namespace std;

namespace anim::vulkan {

class RenderPass {
public:
    // Creates a render pass with color and optional depth attachment
    RenderPass(Device& device, VkFormat colorFormat, VkFormat depthFormat = VK_FORMAT_UNDEFINED,
               VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
               VkImageLayout finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    ~RenderPass();

    // Non-copyable
    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    // Movable
    RenderPass(RenderPass&& other) noexcept;
    RenderPass& operator=(RenderPass&& other) noexcept;

    VkRenderPass handle() const { return renderPass; }
    bool hasDepth() const { return hasDepthAttachment; }

private:
    Device* deviceRef = nullptr;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    bool hasDepthAttachment = false;
};

} // namespace anim::vulkan
