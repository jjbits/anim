#include "core/Window.hpp"
#include "vulkan/Instance.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/Swapchain.hpp"
#include "vulkan/Buffer.hpp"
#include "vulkan/Pipeline.hpp"
#include "renderer/Renderer.hpp"

#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;
using namespace anim;

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        vector<VkVertexInputAttributeDescription> attribs(2);

        attribs[0].binding = 0;
        attribs[0].location = 0;
        attribs[0].format = VK_FORMAT_R32G32_SFLOAT;
        attribs[0].offset = offsetof(Vertex, position);

        attribs[1].binding = 0;
        attribs[1].location = 1;
        attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[1].offset = offsetof(Vertex, color);

        return attribs;
    }
};

vector<uint32_t> readShaderFile(const string& path) {
    ifstream file(path, ios::ate | ios::binary);
    if (!file.is_open()) {
        throw runtime_error("Failed to open shader file: " + path);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    return buffer;
}

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

            // Triangle vertices (position + color)
            vector<Vertex> vertices = {
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // Top - Red
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},   // Bottom right - Green
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}   // Bottom left - Blue
            };

            // Create vertex buffer
            vulkan::Buffer vertexBuffer(
                device,
                sizeof(Vertex) * vertices.size(),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU
            );
            vertexBuffer.upload(vertices.data(), sizeof(Vertex) * vertices.size());

            // Load shaders and create pipeline
            auto vertShader = readShaderFile("shaders/triangle.vert.spv");
            auto fragShader = readShaderFile("shaders/triangle.frag.spv");

            auto bindingDesc = Vertex::getBindingDescription();
            auto attribDescs = Vertex::getAttributeDescriptions();

            vulkan::Pipeline pipeline(
                device,
                renderer.renderPass().handle(),
                vertShader,
                fragShader,
                {bindingDesc},
                attribDescs
            );

            cout << "Initialization complete. Press ESC to exit." << endl;

            while (!window.shouldClose()) {
                window.pollEvents();

                if (renderer.beginFrame()) {
                    auto& cmd = renderer.commandBuffer();
                    VkExtent2D extent = swapchain.extent();

                    // Set dynamic viewport and scissor
                    cmd.setViewport(0, 0, static_cast<float>(extent.width), static_cast<float>(extent.height));
                    cmd.setScissor(0, 0, extent.width, extent.height);

                    // Bind pipeline and vertex buffer, then draw
                    cmd.bindPipeline(pipeline.handle());
                    cmd.bindVertexBuffer(vertexBuffer.handle());
                    cmd.draw(static_cast<uint32_t>(vertices.size()));

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
