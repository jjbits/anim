#pragma once

#include "Device.hpp"
#include "Pipeline.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <unordered_map>
#include <memory>

using namespace std;

namespace anim::vulkan {

struct PipelineConfig {
    vector<uint32_t> vertShaderCode;
    vector<uint32_t> fragShaderCode;
    vector<VkVertexInputBindingDescription> vertexBindings;
    vector<VkVertexInputAttributeDescription> vertexAttribs;
    vector<VkDescriptorSetLayout> descriptorLayouts;
    vector<VkPushConstantRange> pushConstantRanges;
    VkRenderPass renderPass;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;

    bool operator==(const PipelineConfig& other) const;
};

struct PipelineConfigHash {
    size_t operator()(const PipelineConfig& config) const;
};

class PipelineCache {
public:
    PipelineCache(Device& device);
    ~PipelineCache();

    // Non-copyable
    PipelineCache(const PipelineCache&) = delete;
    PipelineCache& operator=(const PipelineCache&) = delete;

    // Get or create pipeline
    Pipeline& getPipeline(const PipelineConfig& config);

    // Clear cache
    void clear();

private:
    Device* device_;
    unordered_map<PipelineConfig, unique_ptr<Pipeline>, PipelineConfigHash> pipelines_;
};

} // namespace anim::vulkan
