#include "core/Window.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "renderer/Renderer.hpp"

#include <iostream>
#include <cstdlib>

using namespace std;
using namespace anim;

int main() {
    try {
        core::Window window("Anim Engine", 1280, 720);

        auto extensions = window.getRequiredVulkanExtensions();
        vulkan::Instance instance("Anim", extensions);

        VkSurfaceKHR surface = window.createSurface(instance.handle());

        {
            vulkan::Device device(instance.handle(), surface);
            vulkan::Swapchain swapchain(device, surface, window.width(), window.height());
            renderer::Renderer renderer(device, swapchain);

            cout << "Initialization complete. Press ESC to exit." << endl;

            while (!window.shouldClose()) {
                window.pollEvents();

                if (renderer.beginFrame()) {
                    // Future: draw commands go here
                    renderer.endFrame();
                }
            }

            device.waitIdle();
            // renderer, swapchain, device destroyed here (RAII)
        }

        vkDestroySurfaceKHR(instance.handle(), surface, nullptr);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Shutdown complete." << endl;
    return EXIT_SUCCESS;
}
