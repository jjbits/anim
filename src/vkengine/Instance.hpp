#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

using namespace std;

namespace anim::vkengine {

class Instance {
public:
    Instance(const string& appName,
             const vector<const char*>& requiredExtensions,
             bool enableValidation = true);
    ~Instance();

    // Non-copyable
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    // Movable
    Instance(Instance&& other) noexcept;
    Instance& operator=(Instance&& other) noexcept;

    VkInstance handle() const { return instance; }
    bool validationEnabled() const { return validationLayers; }

private:
    void setupDebugMessenger();
    bool checkValidationLayerSupport();

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    bool validationLayers = false;
};

} // namespace anim::vkengine
