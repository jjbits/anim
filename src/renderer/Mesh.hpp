#pragma once

#include "../vulkan/Buffer.hpp"
#include "../vulkan/Device.hpp"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>

using namespace std;

namespace anim::renderer {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
        vector<VkVertexInputAttributeDescription> attribs(3);

        attribs[0].binding = 0;
        attribs[0].location = 0;
        attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[0].offset = offsetof(Vertex, position);

        attribs[1].binding = 0;
        attribs[1].location = 1;
        attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribs[1].offset = offsetof(Vertex, normal);

        attribs[2].binding = 0;
        attribs[2].location = 2;
        attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribs[2].offset = offsetof(Vertex, uv);

        return attribs;
    }
};

class Mesh {
public:
    Mesh(vulkan::Device& device, const vector<Vertex>& vertices, const vector<uint32_t>& indices);
    ~Mesh() = default;

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept = default;
    Mesh& operator=(Mesh&& other) noexcept = default;

    VkBuffer vertexBuffer() const { return vertexBuf.handle(); }
    VkBuffer indexBuffer() const { return indexBuf.handle(); }
    uint32_t indexCount() const { return indexCnt; }

    void draw(VkCommandBuffer cmd) const;

private:
    vulkan::Buffer vertexBuf;
    vulkan::Buffer indexBuf;
    uint32_t indexCnt = 0;
};

} // namespace anim::renderer
