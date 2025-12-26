#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

#include <vector>

using namespace std;

namespace anim::vulkan {

class Swapchain {
public:
    Swapchain(Device& device, VkSurfaceKHR surface, uint32_t width, uint32_t height);
    ~Swapchain();

    // Non-copyable
    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    // Movable
    Swapchain(Swapchain&& other) noexcept;
    Swapchain& operator=(Swapchain&& other) noexcept;

    VkSwapchainKHR handle() const { return swapchain; }
    VkFormat imageFormat() const { return format; }
    VkExtent2D extent() const { return swapExtent; }
    const vector<VkImageView>& imageViews() const { return views; }
    uint32_t imageCount() const { return static_cast<uint32_t>(views.size()); }

    VkResult acquireNextImage(VkSemaphore signalSemaphore, uint32_t* imageIndex);
    VkResult present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

private:
    void createSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height);
    void createImageViews();

    VkSurfaceFormatKHR chooseSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR choosePresentMode(const vector<VkPresentModeKHR>& modes);
    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

    Device* deviceRef = nullptr;
    VkSurfaceKHR surfaceRef = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    vector<VkImage> images;
    vector<VkImageView> views;
    VkFormat format;
    VkExtent2D swapExtent;
};

} // namespace anim::vulkan
