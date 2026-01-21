#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"
#include "ModelLoader.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/Pipeline.hpp"
#include "../vulkan/PipelineCache.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/DescriptorSet.hpp"
#include "../vulkan/CommandPool.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <string>

using namespace std;

namespace anim::renderer {

struct CameraData {
    glm::mat4 view;
    glm::vec3 position;
};

class Scene {
public:
    Scene(vulkan::Device& device, VkRenderPass renderPass);
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    void loadModel(const string& path);
    void addTriangle();

    void update(float time, float aspect, const CameraData& camera);
    void render(VkCommandBuffer cmd);

    void toggleWireframe();
    bool isWireframe() const { return wireframeMode; }

private:
    void loadShaders();
    void createDescriptors();
    void createPipeline(VkRenderPass renderPass);
    void createDefaultTexture();

    vulkan::Device* deviceRef;
    VkRenderPass renderPassRef;

    unique_ptr<vulkan::CommandPool> commandPool;
    unique_ptr<vulkan::PipelineCache> pipelineCache;
    vulkan::Pipeline* currentPipeline = nullptr;
    unique_ptr<vulkan::Buffer> uniformBuffer;
    unique_ptr<vulkan::DescriptorSetLayout> descriptorLayout;
    unique_ptr<vulkan::DescriptorPool> descriptorPool;
    vector<unique_ptr<vulkan::DescriptorSet>> materialDescriptorSets;
    unique_ptr<vulkan::DescriptorSet> defaultDescriptorSet;

    vector<uint32_t> vertShaderCode;
    vector<uint32_t> fragShaderCode;

    // Loaded model data
    vector<LoadedMesh> loadedMeshes;
    vector<unique_ptr<Texture>> textures;
    vector<LoadedMaterial> materials;

    // Default texture for materials without specific textures
    unique_ptr<Texture> defaultTexture;

    // Wireframe mode
    bool wireframeMode = false;
    vulkan::PipelineConfig pipelineConfig;
};

} // namespace anim::renderer
