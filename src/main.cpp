#include "core/Window.hpp"
#include "core/Camera.hpp"
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

            // Create camera at a good viewing position
            core::Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), -90.0f, 0.0f);

            cout << "Controls: WASD to move, Click+Drag to look, Space/Shift for up/down, ESC to exit" << endl;

            auto lastTime = chrono::high_resolution_clock::now();

            while (!window.shouldClose()) {
                window.pollEvents();

                // Calculate delta time
                auto currentTime = chrono::high_resolution_clock::now();
                float deltaTime = chrono::duration<float>(currentTime - lastTime).count();
                lastTime = currentTime;

                // Get input state
                const auto& input = window.input();

                // Update camera from input
                if (input.forward) camera.moveForward(deltaTime);
                if (input.backward) camera.moveForward(-deltaTime);
                if (input.right) camera.moveRight(deltaTime);
                if (input.left) camera.moveRight(-deltaTime);
                if (input.up) camera.moveUp(deltaTime);
                if (input.down) camera.moveUp(-deltaTime);

                if (input.leftMouseDown) {
                    camera.rotate(input.mouseDeltaX, -input.mouseDeltaY);
                }

                VkExtent2D extent = swapchain.extent();
                float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

                renderer::CameraData camData;
                camData.view = camera.viewMatrix();
                camData.position = camera.position();

                float time = chrono::duration<float>(currentTime - lastTime).count();
                scene.update(time, aspect, camData);

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
