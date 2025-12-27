#include "Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <stdexcept>

using namespace std;

namespace anim::renderer {

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

static vector<uint32_t> readShaderFile(const string& path) {
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

Scene::Scene(vulkan::Device& device, VkRenderPass renderPass)
    : deviceRef(&device)
    , renderPassRef(renderPass) {
    loadShaders();
    createDescriptors();
    createPipeline(renderPass);
}

void Scene::loadShaders() {
    vertShaderCode = readShaderFile("shaders/model.vert.spv");
    fragShaderCode = readShaderFile("shaders/model.frag.spv");
}

void Scene::createDescriptors() {
    // Uniform buffer
    uniformBuffer = make_unique<vulkan::Buffer>(
        *deviceRef,
        sizeof(UniformBufferObject),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    // Descriptor layout
    vector<VkDescriptorSetLayoutBinding> bindings = {{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    }};
    descriptorLayout = make_unique<vulkan::DescriptorSetLayout>(*deviceRef, bindings);

    // Descriptor pool
    vector<VkDescriptorPoolSize> poolSizes = {{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1
    }};
    descriptorPool = make_unique<vulkan::DescriptorPool>(*deviceRef, poolSizes, 1);

    // Descriptor set
    descriptorSet = make_unique<vulkan::DescriptorSet>(*descriptorPool, *descriptorLayout);
    descriptorSet->updateBuffer(0, uniformBuffer->handle(), 0, sizeof(UniformBufferObject));
}

void Scene::createPipeline(VkRenderPass renderPass) {
    auto bindingDesc = Vertex::getBindingDescription();
    auto attribDescs = Vertex::getAttributeDescriptions();

    pipeline = make_unique<vulkan::Pipeline>(
        *deviceRef,
        renderPass,
        vertShaderCode,
        fragShaderCode,
        vector<VkVertexInputBindingDescription>{bindingDesc},
        attribDescs,
        vector<VkDescriptorSetLayout>{descriptorLayout->handle()}
    );
}

void Scene::loadModel(const string& path) {
    auto loaded = ModelLoader::load(*deviceRef, path);
    for (auto& mesh : loaded) {
        meshes.push_back(std::move(mesh));
    }
}

void Scene::addTriangle() {
    vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    vector<uint32_t> indices = {0, 1, 2};

    meshes.push_back(make_unique<Mesh>(*deviceRef, vertices, indices));
}

void Scene::update(float time, float aspect) {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;  // Flip Y for Vulkan

    uniformBuffer->upload(&ubo, sizeof(ubo));
}

void Scene::render(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());

    VkDescriptorSet ds = descriptorSet->handle();
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout(), 0, 1, &ds, 0, nullptr);

    for (const auto& mesh : meshes) {
        mesh->draw(cmd);
    }
}

} // namespace anim::renderer
