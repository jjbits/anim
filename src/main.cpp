#include "core/Window.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Scene.hpp"

#include <iostream>
#include <chrono>
#include <cstdlib>

using namespace std;
using namespace anim;

int main(int argc, char* argv[]) {
    string modelPath = "";
    if (argc > 1) {
        modelPath = argv[1];
    }

    try {
        core::Window window("Anim Engine", 1280, 720);

        auto extensions = window.getRequiredVulkanExtensions();
        vulkan::Instance instance("Anim", extensions);

        VkSurfaceKHR surface = window.createSurface(instance.handle());

        {
            vulkan::Device device(instance.handle(), surface);
            vulkan::Swapchain swapchain(device, surface, window.width(), window.height());
            renderer::Renderer renderer(device, swapchain);

            renderer::Scene scene(device, renderer.renderPass().handle());

            if (modelPath.empty()) {
                scene.addTriangle();
                cout << "No model specified. Rendering triangle." << endl;
            } else {
                scene.loadModel(modelPath);
                cout << "Loaded model: " << modelPath << endl;
            }

            cout << "Press ESC to exit." << endl;

            auto startTime = chrono::high_resolution_clock::now();

            while (!window.shouldClose()) {
                window.pollEvents();

                auto currentTime = chrono::high_resolution_clock::now();
                float time = chrono::duration<float>(currentTime - startTime).count();

                VkExtent2D extent = swapchain.extent();
                float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

                scene.update(time, aspect);

                if (renderer.beginFrame()) {
                    auto& cmd = renderer.commandBuffer();

                    cmd.setViewport(0, 0, static_cast<float>(extent.width), static_cast<float>(extent.height));
                    cmd.setScissor(0, 0, extent.width, extent.height);

                    scene.render(cmd.handle());

                    renderer.endFrame();
                }
            }

            device.waitIdle();
        }

        vkDestroySurfaceKHR(instance.handle(), surface, nullptr);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Shutdown complete." << endl;
    return EXIT_SUCCESS;
}
