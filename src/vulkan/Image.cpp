#include "Image.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format,
             VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
             uint32_t mipLevels, VkSampleCountFlagBits samples)
    : deviceRef(&device)
    , imageFormat(format)
    , extent{width, height}
    , mipLevelCount(mipLevels) {
    createImage(usage, mipLevels, samples);
    createImageView(aspectFlags);
}

Image::~Image() {
    if (deviceRef) {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(deviceRef->handle(), imageView, nullptr);
        }
        if (image != VK_NULL_HANDLE) {
            vmaDestroyImage(deviceRef->allocator(), image, allocation);
        }
    }
}

Image::Image(Image&& other) noexcept
    : deviceRef(other.deviceRef)
    , image(other.image)
    , imageView(other.imageView)
    , allocation(other.allocation)
    , imageFormat(other.imageFormat)
    , extent(other.extent)
    , mipLevelCount(other.mipLevelCount) {
    other.deviceRef = nullptr;
    other.image = VK_NULL_HANDLE;
    other.imageView = VK_NULL_HANDLE;
    other.allocation = VK_NULL_HANDLE;
    other.imageFormat = VK_FORMAT_UNDEFINED;
    other.extent = {0, 0};
    other.mipLevelCount = 1;
}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        if (deviceRef) {
            if (imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(deviceRef->handle(), imageView, nullptr);
            }
            if (image != VK_NULL_HANDLE) {
                vmaDestroyImage(deviceRef->allocator(), image, allocation);
            }
        }

        deviceRef = other.deviceRef;
        image = other.image;
        imageView = other.imageView;
        allocation = other.allocation;
        imageFormat = other.imageFormat;
        extent = other.extent;
        mipLevelCount = other.mipLevelCount;

        other.deviceRef = nullptr;
        other.image = VK_NULL_HANDLE;
        other.imageView = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.imageFormat = VK_FORMAT_UNDEFINED;
        other.extent = {0, 0};
        other.mipLevelCount = 1;
    }
    return *this;
}

void Image::createImage(VkImageUsageFlags usage, uint32_t mipLevels, VkSampleCountFlagBits samples) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = imageFormat;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = samples;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(deviceRef->allocator(), &imageInfo, &allocInfo,
                       &image, &allocation, nullptr) != VK_SUCCESS) {
        throw runtime_error("Failed to create image");
    }
}

void Image::createImageView(VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageFormat;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevelCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(deviceRef->handle(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw runtime_error("Failed to create image view");
    }
}

} // namespace anim::vulkan
