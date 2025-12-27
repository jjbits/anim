#pragma once

#include "Mesh.hpp"
#include "ModelLoader.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/Pipeline.hpp"
#include "../vulkan/Buffer.hpp"
#include "../vulkan/DescriptorSet.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <string>

using namespace std;

namespace anim::renderer {

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

    void update(float time, float aspect);
    void render(VkCommandBuffer cmd);

private:
    void loadShaders();
    void createDescriptors();
    void createPipeline(VkRenderPass renderPass);

    vulkan::Device* deviceRef;
    VkRenderPass renderPassRef;

    unique_ptr<vulkan::Pipeline> pipeline;
    unique_ptr<vulkan::Buffer> uniformBuffer;
    unique_ptr<vulkan::DescriptorSetLayout> descriptorLayout;
    unique_ptr<vulkan::DescriptorPool> descriptorPool;
    unique_ptr<vulkan::DescriptorSet> descriptorSet;

    vector<uint32_t> vertShaderCode;
    vector<uint32_t> fragShaderCode;

    vector<unique_ptr<Mesh>> meshes;
};

} // namespace anim::renderer
