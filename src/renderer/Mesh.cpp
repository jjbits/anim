#include "Mesh.hpp"

using namespace std;

namespace anim::renderer {

Mesh::Mesh(vulkan::Device& device, const vector<Vertex>& vertices, const vector<uint32_t>& indices)
    : vertexBuf(device, sizeof(Vertex) * vertices.size(),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , indexBuf(device, sizeof(uint32_t) * indices.size(),
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , indexCnt(static_cast<uint32_t>(indices.size())) {
    vertexBuf.upload(vertices.data(), sizeof(Vertex) * vertices.size());
    indexBuf.upload(indices.data(), sizeof(uint32_t) * indices.size());
}

void Mesh::draw(VkCommandBuffer cmd) const {
    VkBuffer buffers[] = {vertexBuf.handle()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(cmd, indexBuf.handle(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmd, indexCnt, 1, 0, 0, 0);
}

} // namespace anim::renderer
