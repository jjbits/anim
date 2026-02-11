#include "Swapchain.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace std;

namespace anim::vulkan {

Swapchain::Swapchain(Device& device, VkSurfaceKHR surface, uint32_t width, uint32_t height)
    : deviceRef(&device), surfaceRef(surface) {
    createSwapchain(surface, width, height);
    createImageViews();
    cout << "Swapchain created (" << swapExtent.width << "x" << swapExtent.height << ")" << endl;
}

Swapchain::~Swapchain() {
    if (deviceRef) {
        for (auto view : views) {
            vkDestroyImageView(deviceRef->handle(), view, nullptr);
        }
        if (swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(deviceRef->handle(), swapchain, nullptr);
        }
    }
}

Swapchain::Swapchain(Swapchain&& other) noexcept
    : deviceRef(other.deviceRef)
    , surfaceRef(other.surfaceRef)
    , swapchain(other.swapchain)
    , images(move(other.images))
    , views(move(other.views))
    , format(other.format)
    , swapExtent(other.swapExtent) {
    other.deviceRef = nullptr;
    other.swapchain = VK_NULL_HANDLE;
}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
    if (this != &other) {
        if (deviceRef) {
            for (auto view : views) {
                vkDestroyImageView(deviceRef->handle(), view, nullptr);
            }
            if (swapchain != VK_NULL_HANDLE) {
                vkDestroySwapchainKHR(deviceRef->handle(), swapchain, nullptr);
            }
        }

        deviceRef = other.deviceRef;
        surfaceRef = other.surfaceRef;
        swapchain = other.swapchain;
        images = move(other.images);
        views = move(other.views);
        format = other.format;
        swapExtent = other.swapExtent;

        other.deviceRef = nullptr;
        other.swapchain = VK_NULL_HANDLE;
    }
    return *this;
}

void Swapchain::recreate(uint32_t width, uint32_t height) {
    deviceRef->waitIdle();

    for (auto view : views) {
        vkDestroyImageView(deviceRef->handle(), view, nullptr);
    }
    views.clear();

    VkSwapchainKHR oldSwapchain = swapchain;
    swapchain = VK_NULL_HANDLE;

    createSwapchain(surfaceRef, width, height, oldSwapchain);
    createImageViews();

    vkDestroySwapchainKHR(deviceRef->handle(), oldSwapchain, nullptr);

    cout << "Swapchain recreated (" << swapExtent.width << "x" << swapExtent.height << ")" << endl;
}

VkResult Swapchain::acquireNextImage(VkSemaphore signalSemaphore, uint32_t* imageIndex) {
    return vkAcquireNextImageKHR(
        deviceRef->handle(),
        swapchain,
        UINT64_MAX,
        signalSemaphore,
        VK_NULL_HANDLE,
        imageIndex
    );
}

VkResult Swapchain::present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    return vkQueuePresentKHR(queue, &presentInfo);
}

void Swapchain::createSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain) {
    VkPhysicalDevice physicalDevice = deviceRef->physicalDevice();

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
    VkPresentModeKHR presentMode = choosePresentMode(presentModes);
    swapExtent = chooseExtent(capabilities, width, height);
    format = surfaceFormat.format;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = swapExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {
        deviceRef->graphicsQueueFamily(),
        deviceRef->presentQueueFamily()
    };

    if (deviceRef->graphicsQueueFamily() != deviceRef->presentQueueFamily()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    if (vkCreateSwapchainKHR(deviceRef->handle(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw runtime_error("Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(deviceRef->handle(), swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(deviceRef->handle(), swapchain, &imageCount, images.data());
}

void Swapchain::createImageViews() {
    views.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(deviceRef->handle(), &createInfo, nullptr, &views[i]) != VK_SUCCESS) {
            throw runtime_error("Failed to create image view");
        }
    }
}

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

VkPresentModeKHR Swapchain::choosePresentMode(const vector<VkPresentModeKHR>& modes) {
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;  // Triple buffering, low latency
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;  // VSync, always available
}

VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    VkExtent2D extent = {width, height};
    extent.width = clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

} // namespace anim::vulkan
