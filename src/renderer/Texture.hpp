#pragma once

#include "../vulkan/Device.hpp"
#include "../vulkan/Image.hpp"
#include "../vulkan/Sampler.hpp"
#include "../vulkan/CommandPool.hpp"

#include <vulkan/vulkan.h>

#include <string>

using namespace std;

namespace anim::renderer {

class Texture {
public:
    // Load texture from file
    Texture(vulkan::Device& device, vulkan::CommandPool& cmdPool, const string& path);

    // Create texture from raw pixel data (RGBA)
    Texture(vulkan::Device& device, vulkan::CommandPool& cmdPool,
            uint32_t width, uint32_t height, const void* pixels);

    ~Texture() = default;

    // Non-copyable
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Movable
    Texture(Texture&& other) noexcept = default;
    Texture& operator=(Texture&& other) noexcept = default;

    VkImageView view() const { return image.view(); }
    VkSampler sampler() const { return texSampler.handle(); }
    uint32_t width() const { return image.width(); }
    uint32_t height() const { return image.height(); }

private:
    void upload(vulkan::Device& device, vulkan::CommandPool& cmdPool,
                uint32_t width, uint32_t height, const void* pixels);

    vulkan::Image image;
    vulkan::Sampler texSampler;
};

} // namespace anim::renderer
