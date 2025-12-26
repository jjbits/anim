#include "RenderPass.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

RenderPass::RenderPass(Device& device, VkFormat colorFormat,
                       VkImageLayout initialLayout, VkImageLayout finalLayout)
    : deviceRef(&device) {

    // Color attachment description
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = colorFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = initialLayout;
    colorAttachment.finalLayout = finalLayout;

    // Subpass uses color attachment
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependency for layout transitions
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Create render pass
    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &colorAttachment;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(deviceRef->handle(), &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw runtime_error("Failed to create render pass");
    }
}

RenderPass::~RenderPass() {
    if (deviceRef && renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(deviceRef->handle(), renderPass, nullptr);
    }
}

RenderPass::RenderPass(RenderPass&& other) noexcept
    : deviceRef(other.deviceRef)
    , renderPass(other.renderPass) {
    other.deviceRef = nullptr;
    other.renderPass = VK_NULL_HANDLE;
}

RenderPass& RenderPass::operator=(RenderPass&& other) noexcept {
    if (this != &other) {
        if (deviceRef && renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(deviceRef->handle(), renderPass, nullptr);
        }

        deviceRef = other.deviceRef;
        renderPass = other.renderPass;

        other.deviceRef = nullptr;
        other.renderPass = VK_NULL_HANDLE;
    }
    return *this;
}

} // namespace anim::vulkan
