#include "Texture.hpp"

#include "../vulkan/Buffer.hpp"
#include "../vulkan/CommandBuffer.hpp"

#include <stb_image.h>

#include <stdexcept>

using namespace std;

namespace anim::renderer {

Texture::Texture(vulkan::Device& device, vulkan::CommandPool& cmdPool, const string& path)
    : image(device, 1, 1, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT)
    , texSampler(device) {
    int width, height, channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        throw runtime_error("Failed to load texture: " + path);
    }

    // Recreate image with actual dimensions
    image = vulkan::Image(device, width, height, VK_FORMAT_R8G8B8A8_SRGB,
                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                          VK_IMAGE_ASPECT_COLOR_BIT);

    upload(device, cmdPool, width, height, pixels);

    stbi_image_free(pixels);
}

Texture::Texture(vulkan::Device& device, vulkan::CommandPool& cmdPool,
                 uint32_t width, uint32_t height, const void* pixels)
    : image(device, width, height, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT)
    , texSampler(device) {
    upload(device, cmdPool, width, height, pixels);
}

void Texture::upload(vulkan::Device& device, vulkan::CommandPool& cmdPool,
                     uint32_t width, uint32_t height, const void* pixels) {
    VkDeviceSize imageSize = width * height * 4;

    // Create staging buffer
    vulkan::Buffer stagingBuffer(device, imageSize,
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VMA_MEMORY_USAGE_CPU_ONLY);
    stagingBuffer.upload(pixels, imageSize);

    // Record and submit transfer commands
    vulkan::CommandBuffer cmd(cmdPool);
    cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    cmd.transitionImageLayout(image.handle(), image.format(),
                              VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    cmd.copyBufferToImage(stagingBuffer.handle(), image.handle(), width, height);

    cmd.transitionImageLayout(image.handle(), image.format(),
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    cmd.end();

    // Submit and wait
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    VkCommandBuffer cmdHandle = cmd.handle();
    submitInfo.pCommandBuffers = &cmdHandle;

    vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.graphicsQueue());
}

} // namespace anim::renderer
