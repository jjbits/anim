#include "core/Window.hpp"
#include "vkengine/Instance.hpp"
#include "vkengine/Device.hpp"
#include "vkengine/Swapchain.hpp"

#include <iostream>
#include <cstdlib>

using namespace std;
using namespace anim;

int main() {
    try {
        core::Window window("Anim Engine", 1280, 720);

        auto extensions = window.getRequiredVulkanExtensions();
        vkengine::Instance instance("Anim", extensions);

        VkSurfaceKHR surface = window.createSurface(instance.handle());

        vkengine::Device device(instance.handle(), surface);
        vkengine::Swapchain swapchain(device, surface, window.width(), window.height());

        cout << "Initialization complete. Press ESC to exit." << endl;

        while (!window.shouldClose()) {
            window.pollEvents();
        }

        device.waitIdle();

        vkDestroySurfaceKHR(instance.handle(), surface, nullptr);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Shutdown complete." << endl;
    return EXIT_SUCCESS;
}
