#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

using namespace std;

namespace anim::vulkan {

class RenderPass {
public:
    // Creates a simple render pass with one color attachment
    RenderPass(Device& device, VkFormat colorFormat,
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

private:
    Device* deviceRef = nullptr;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
