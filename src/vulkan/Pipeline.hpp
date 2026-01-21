#pragma once

#include "Device.hpp"

#include <vulkan/vulkan.h>

#include <vector>

using namespace std;

namespace anim::vulkan {

class Pipeline {
public:
    Pipeline(Device& device, VkRenderPass renderPass,
             const vector<uint32_t>& vertShaderCode,
             const vector<uint32_t>& fragShaderCode,
             const vector<VkVertexInputBindingDescription>& vertexBindings,
             const vector<VkVertexInputAttributeDescription>& vertexAttribs,
             const vector<VkDescriptorSetLayout>& descriptorLayouts = {},
             const vector<VkPushConstantRange>& pushConstantRanges = {},
             VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL);
    ~Pipeline();

    // Non-copyable
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    // Movable
    Pipeline(Pipeline&& other) noexcept;
    Pipeline& operator=(Pipeline&& other) noexcept;

    VkPipeline handle() const { return pipeline; }
    VkPipelineLayout layout() const { return pipelineLayout; }

private:
    VkShaderModule createShaderModule(const vector<uint32_t>& code);

    Device* deviceRef = nullptr;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
};

} // namespace anim::vulkan
