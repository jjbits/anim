#include "PipelineCache.hpp"

#include <functional>

using namespace std;

namespace anim::vulkan {

// Helper to combine hash values
template<typename T>
static void hashCombine(size_t& seed, const T& val) {
    seed ^= hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Hash shader code
static size_t hashShaderCode(const vector<uint32_t>& code) {
    size_t seed = code.size();
    for (const auto& val : code) {
        hashCombine(seed, val);
    }
    return seed;
}

bool PipelineConfig::operator==(const PipelineConfig& other) const {
    return vertShaderCode == other.vertShaderCode &&
           fragShaderCode == other.fragShaderCode &&
           vertexBindings.size() == other.vertexBindings.size() &&
           vertexAttribs.size() == other.vertexAttribs.size() &&
           descriptorLayouts == other.descriptorLayouts &&
           pushConstantRanges.size() == other.pushConstantRanges.size() &&
           renderPass == other.renderPass &&
           polygonMode == other.polygonMode;
}

size_t PipelineConfigHash::operator()(const PipelineConfig& config) const {
    size_t seed = 0;
    hashCombine(seed, hashShaderCode(config.vertShaderCode));
    hashCombine(seed, hashShaderCode(config.fragShaderCode));
    hashCombine(seed, config.vertexBindings.size());
    hashCombine(seed, config.vertexAttribs.size());
    hashCombine(seed, config.descriptorLayouts.size());
    hashCombine(seed, config.pushConstantRanges.size());
    hashCombine(seed, reinterpret_cast<size_t>(config.renderPass));
    hashCombine(seed, static_cast<size_t>(config.polygonMode));
    return seed;
}

PipelineCache::PipelineCache(Device& device)
    : device_(&device) {
}

PipelineCache::~PipelineCache() {
    clear();
}

Pipeline& PipelineCache::getPipeline(const PipelineConfig& config) {
    auto it = pipelines_.find(config);
    if (it != pipelines_.end()) {
        return *it->second;
    }

    // Create new pipeline
    auto pipeline = make_unique<Pipeline>(
        *device_,
        config.renderPass,
        config.vertShaderCode,
        config.fragShaderCode,
        config.vertexBindings,
        config.vertexAttribs,
        config.descriptorLayouts,
        config.pushConstantRanges,
        config.polygonMode
    );

    auto& ref = *pipeline;
    pipelines_.emplace(config, std::move(pipeline));
    return ref;
}

void PipelineCache::clear() {
    pipelines_.clear();
}

} // namespace anim::vulkan
