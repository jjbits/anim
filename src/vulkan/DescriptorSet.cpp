#include "DescriptorSet.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

// =============================================================================
// DescriptorSetLayout
// =============================================================================

DescriptorSetLayout::DescriptorSetLayout(Device& device,
                                         const vector<VkDescriptorSetLayoutBinding>& bindings)
    : deviceRef(&device) {
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(deviceRef->handle(), &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
        throw runtime_error("Failed to create descriptor set layout");
    }
}

DescriptorSetLayout::~DescriptorSetLayout() {
    if (deviceRef && layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(deviceRef->handle(), layout, nullptr);
    }
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
    : deviceRef(other.deviceRef)
    , layout(other.layout) {
    other.deviceRef = nullptr;
    other.layout = VK_NULL_HANDLE;
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept {
    if (this != &other) {
        if (deviceRef && layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(deviceRef->handle(), layout, nullptr);
        }

        deviceRef = other.deviceRef;
        layout = other.layout;

        other.deviceRef = nullptr;
        other.layout = VK_NULL_HANDLE;
    }
    return *this;
}

// =============================================================================
// DescriptorPool
// =============================================================================

DescriptorPool::DescriptorPool(Device& device, const vector<VkDescriptorPoolSize>& poolSizes,
                               uint32_t maxSets)
    : deviceRef(&device) {
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;

    if (vkCreateDescriptorPool(deviceRef->handle(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw runtime_error("Failed to create descriptor pool");
    }
}

DescriptorPool::~DescriptorPool() {
    if (deviceRef && pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(deviceRef->handle(), pool, nullptr);
    }
}

DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept
    : deviceRef(other.deviceRef)
    , pool(other.pool) {
    other.deviceRef = nullptr;
    other.pool = VK_NULL_HANDLE;
}

DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept {
    if (this != &other) {
        if (deviceRef && pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(deviceRef->handle(), pool, nullptr);
        }

        deviceRef = other.deviceRef;
        pool = other.pool;

        other.deviceRef = nullptr;
        other.pool = VK_NULL_HANDLE;
    }
    return *this;
}

VkDescriptorSet DescriptorPool::allocate(VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet set;
    if (vkAllocateDescriptorSets(deviceRef->handle(), &allocInfo, &set) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate descriptor set");
    }
    return set;
}

vector<VkDescriptorSet> DescriptorPool::allocate(const vector<VkDescriptorSetLayout>& layouts) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    allocInfo.pSetLayouts = layouts.data();

    vector<VkDescriptorSet> sets(layouts.size());
    if (vkAllocateDescriptorSets(deviceRef->handle(), &allocInfo, sets.data()) != VK_SUCCESS) {
        throw runtime_error("Failed to allocate descriptor sets");
    }
    return sets;
}

void DescriptorPool::reset() {
    vkResetDescriptorPool(deviceRef->handle(), pool, 0);
}

// =============================================================================
// DescriptorSet
// =============================================================================

DescriptorSet::DescriptorSet(DescriptorPool& pool, DescriptorSetLayout& layout)
    : deviceRef(&pool.device()) {
    set = pool.allocate(layout.handle());
}

DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
    : deviceRef(other.deviceRef)
    , set(other.set) {
    other.deviceRef = nullptr;
    other.set = VK_NULL_HANDLE;
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept {
    if (this != &other) {
        // Note: We don't free the set - pool owns the memory
        deviceRef = other.deviceRef;
        set = other.set;

        other.deviceRef = nullptr;
        other.set = VK_NULL_HANDLE;
    }
    return *this;
}

void DescriptorSet::updateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset,
                                  VkDeviceSize range, VkDescriptorType type) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(deviceRef->handle(), 1, &write, 0, nullptr);
}

void DescriptorSet::updateImage(uint32_t binding, VkImageView imageView, VkSampler sampler,
                                 VkImageLayout layout, VkDescriptorType type) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = layout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(deviceRef->handle(), 1, &write, 0, nullptr);
}

} // namespace anim::vulkan
