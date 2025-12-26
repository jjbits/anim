#include "Framebuffer.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

Framebuffer::Framebuffer(Device& device, RenderPass& renderPass,
                         VkImageView imageView, VkExtent2D extent)
    : deviceRef(&device), fbExtent(extent) {

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass.handle();
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &imageView;
    createInfo.width = extent.width;
    createInfo.height = extent.height;
    createInfo.layers = 1;

    if (vkCreateFramebuffer(deviceRef->handle(), &createInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw runtime_error("Failed to create framebuffer");
    }
}

Framebuffer::~Framebuffer() {
    if (deviceRef && framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(deviceRef->handle(), framebuffer, nullptr);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : deviceRef(other.deviceRef)
    , framebuffer(other.framebuffer)
    , fbExtent(other.fbExtent) {
    other.deviceRef = nullptr;
    other.framebuffer = VK_NULL_HANDLE;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        if (deviceRef && framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(deviceRef->handle(), framebuffer, nullptr);
        }

        deviceRef = other.deviceRef;
        framebuffer = other.framebuffer;
        fbExtent = other.fbExtent;

        other.deviceRef = nullptr;
        other.framebuffer = VK_NULL_HANDLE;
    }
    return *this;
}

} // namespace anim::vulkan
