#pragma once

#include "Device.hpp"
#include "RenderPass.hpp"

#include <vulkan/vulkan.h>
#include <vector>

using namespace std;

namespace anim::vulkan {

class Framebuffer {
public:
    Framebuffer(Device& device, RenderPass& renderPass,
                const vector<VkImageView>& attachments, VkExtent2D extent);
    ~Framebuffer();

    // Non-copyable
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // Movable
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    VkFramebuffer handle() const { return framebuffer; }
    VkExtent2D extent() const { return fbExtent; }

private:
    Device* deviceRef = nullptr;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkExtent2D fbExtent;
};

} // namespace anim::vulkan
