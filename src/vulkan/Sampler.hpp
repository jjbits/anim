#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

using namespace std;

namespace anim::vulkan {

struct SamplerConfig {
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    float maxAnisotropy = 16.0f;
    bool enableAnisotropy = true;
    float minLod = 0.0f;
    float maxLod = VK_LOD_CLAMP_NONE;
};

class Sampler {
public:
    Sampler(Device& device, const SamplerConfig& config = {});
    ~Sampler();

    // Non-copyable
    Sampler(const Sampler&) = delete;
    Sampler& operator=(const Sampler&) = delete;

    // Movable
    Sampler(Sampler&& other) noexcept;
    Sampler& operator=(Sampler&& other) noexcept;

    VkSampler handle() const { return sampler; }

private:
    Device* deviceRef = nullptr;
    VkSampler sampler = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
