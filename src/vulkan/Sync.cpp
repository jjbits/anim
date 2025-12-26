#include "Sync.hpp"

#include <stdexcept>

using namespace std;

namespace anim::vulkan {

// =============================================================================
// Semaphore
// =============================================================================

Semaphore::Semaphore(Device& device) : deviceRef(&device) {
    VkSemaphoreCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(deviceRef->handle(), &createInfo, nullptr, &semaphore) != VK_SUCCESS) {
        throw runtime_error("Failed to create semaphore");
    }
}

Semaphore::~Semaphore() {
    if (deviceRef && semaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(deviceRef->handle(), semaphore, nullptr);
    }
}

Semaphore::Semaphore(Semaphore&& other) noexcept
    : deviceRef(other.deviceRef)
    , semaphore(other.semaphore) {
    other.deviceRef = nullptr;
    other.semaphore = VK_NULL_HANDLE;
}

Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
    if (this != &other) {
        if (deviceRef && semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(deviceRef->handle(), semaphore, nullptr);
        }

        deviceRef = other.deviceRef;
        semaphore = other.semaphore;

        other.deviceRef = nullptr;
        other.semaphore = VK_NULL_HANDLE;
    }
    return *this;
}

// =============================================================================
// Fence
// =============================================================================

Fence::Fence(Device& device, bool signaled) : deviceRef(&device) {
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (signaled) {
        createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    if (vkCreateFence(deviceRef->handle(), &createInfo, nullptr, &fence) != VK_SUCCESS) {
        throw runtime_error("Failed to create fence");
    }
}

Fence::~Fence() {
    if (deviceRef && fence != VK_NULL_HANDLE) {
        vkDestroyFence(deviceRef->handle(), fence, nullptr);
    }
}

Fence::Fence(Fence&& other) noexcept
    : deviceRef(other.deviceRef)
    , fence(other.fence) {
    other.deviceRef = nullptr;
    other.fence = VK_NULL_HANDLE;
}

Fence& Fence::operator=(Fence&& other) noexcept {
    if (this != &other) {
        if (deviceRef && fence != VK_NULL_HANDLE) {
            vkDestroyFence(deviceRef->handle(), fence, nullptr);
        }

        deviceRef = other.deviceRef;
        fence = other.fence;

        other.deviceRef = nullptr;
        other.fence = VK_NULL_HANDLE;
    }
    return *this;
}

void Fence::wait(uint64_t timeout) {
    vkWaitForFences(deviceRef->handle(), 1, &fence, VK_TRUE, timeout);
}

void Fence::reset() {
    vkResetFences(deviceRef->handle(), 1, &fence);
}

} // namespace anim::vulkan
