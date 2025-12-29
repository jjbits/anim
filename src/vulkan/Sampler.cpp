#include "Sampler.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

Sampler::Sampler(Device& device, const SamplerConfig& config)
    : deviceRef(&device) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.physicalDevice(), &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = config.magFilter;
    samplerInfo.minFilter = config.minFilter;
    samplerInfo.addressModeU = config.addressModeU;
    samplerInfo.addressModeV = config.addressModeV;
    samplerInfo.addressModeW = config.addressModeW;
    samplerInfo.anisotropyEnable = config.enableAnisotropy ? VK_TRUE : VK_FALSE;
    samplerInfo.maxAnisotropy = min(config.maxAnisotropy, properties.limits.maxSamplerAnisotropy);
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = config.mipmapMode;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = config.minLod;
    samplerInfo.maxLod = config.maxLod;

    if (vkCreateSampler(device.handle(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw runtime_error("Failed to create sampler");
    }
}

Sampler::~Sampler() {
    if (deviceRef && sampler != VK_NULL_HANDLE) {
        vkDestroySampler(deviceRef->handle(), sampler, nullptr);
    }
}

Sampler::Sampler(Sampler&& other) noexcept
    : deviceRef(other.deviceRef)
    , sampler(other.sampler) {
    other.deviceRef = nullptr;
    other.sampler = VK_NULL_HANDLE;
}

Sampler& Sampler::operator=(Sampler&& other) noexcept {
    if (this != &other) {
        if (deviceRef && sampler != VK_NULL_HANDLE) {
            vkDestroySampler(deviceRef->handle(), sampler, nullptr);
        }

        deviceRef = other.deviceRef;
        sampler = other.sampler;

        other.deviceRef = nullptr;
        other.sampler = VK_NULL_HANDLE;
    }
    return *this;
}

} // namespace anim::vulkan
