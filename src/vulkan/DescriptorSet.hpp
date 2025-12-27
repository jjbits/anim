#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

#include <vector>

using namespace std;

namespace anim::vulkan {

// Describes the structure of a descriptor set
class DescriptorSetLayout {
public:
    DescriptorSetLayout(Device& device, const vector<VkDescriptorSetLayoutBinding>& bindings);
    ~DescriptorSetLayout();

    // Non-copyable
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

    // Movable
    DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

    VkDescriptorSetLayout handle() const { return layout; }

private:
    Device* deviceRef = nullptr;
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
};

// Allocates descriptor sets
class DescriptorPool {
public:
    DescriptorPool(Device& device, const vector<VkDescriptorPoolSize>& poolSizes, uint32_t maxSets);
    ~DescriptorPool();

    // Non-copyable
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    // Movable
    DescriptorPool(DescriptorPool&& other) noexcept;
    DescriptorPool& operator=(DescriptorPool&& other) noexcept;

    VkDescriptorPool handle() const { return pool; }
    Device& device() const { return *deviceRef; }

    VkDescriptorSet allocate(VkDescriptorSetLayout layout);
    vector<VkDescriptorSet> allocate(const vector<VkDescriptorSetLayout>& layouts);
    void reset();

private:
    Device* deviceRef = nullptr;
    VkDescriptorPool pool = VK_NULL_HANDLE;
};

// Wrapper for descriptor set with update helpers
class DescriptorSet {
public:
    DescriptorSet(DescriptorPool& pool, DescriptorSetLayout& layout);
    ~DescriptorSet() = default;  // Pool owns the memory, freed on pool reset/destroy

    // Non-copyable
    DescriptorSet(const DescriptorSet&) = delete;
    DescriptorSet& operator=(const DescriptorSet&) = delete;

    // Movable
    DescriptorSet(DescriptorSet&& other) noexcept;
    DescriptorSet& operator=(DescriptorSet&& other) noexcept;

    VkDescriptorSet handle() const { return set; }

    void updateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset,
                      VkDeviceSize range, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    void updateImage(uint32_t binding, VkImageView imageView, VkSampler sampler,
                     VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

private:
    Device* deviceRef = nullptr;
    VkDescriptorSet set = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
