#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

using namespace std;

namespace anim::vulkan {

class Image {
public:
    Image(Device& device, uint32_t width, uint32_t height, VkFormat format,
          VkImageUsageFlags usage, VkImageAspectFlags aspectFlags,
          uint32_t mipLevels = 1, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    ~Image();

    // Non-copyable
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    // Movable
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    VkImage handle() const { return image; }
    VkImageView view() const { return imageView; }
    VkFormat format() const { return imageFormat; }
    uint32_t width() const { return extent.width; }
    uint32_t height() const { return extent.height; }
    uint32_t mipLevels() const { return mipLevelCount; }

private:
    void createImage(VkImageUsageFlags usage, uint32_t mipLevels, VkSampleCountFlagBits samples);
    void createImageView(VkImageAspectFlags aspectFlags);

    Device* deviceRef = nullptr;
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent = {0, 0};
    uint32_t mipLevelCount = 1;
};

} // namespace anim::vulkan
