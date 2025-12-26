#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

using namespace std;

namespace anim::vulkan {

class Semaphore {
public:
    Semaphore(Device& device);
    ~Semaphore();

    // Non-copyable
    Semaphore(const Semaphore&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

    // Movable
    Semaphore(Semaphore&& other) noexcept;
    Semaphore& operator=(Semaphore&& other) noexcept;

    VkSemaphore handle() const { return semaphore; }

private:
    Device* deviceRef = nullptr;
    VkSemaphore semaphore = VK_NULL_HANDLE;
};

class Fence {
public:
    Fence(Device& device, bool signaled = false);
    ~Fence();

    // Non-copyable
    Fence(const Fence&) = delete;
    Fence& operator=(const Fence&) = delete;

    // Movable
    Fence(Fence&& other) noexcept;
    Fence& operator=(Fence&& other) noexcept;

    VkFence handle() const { return fence; }

    void wait(uint64_t timeout = UINT64_MAX);
    void reset();

private:
    Device* deviceRef = nullptr;
    VkFence fence = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
