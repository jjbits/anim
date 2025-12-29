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
    glm::vec3 camPos;
    float padding; // Align to 16 bytes
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
    commandPool = make_unique<vulkan::CommandPool>(device, device.graphicsQueueFamily());
    loadShaders();
    createDefaultTexture();
    createDescriptors();
    createPipeline(renderPass);
}

void Scene::loadShaders() {
    vertShaderCode = readShaderFile("shaders/model.vert.spv");
    fragShaderCode = readShaderFile("shaders/model.frag.spv");
}

void Scene::createDefaultTexture() {
    // Create a 1x1 white texture as fallback
    uint32_t white = 0xFFFFFFFF;
    defaultTexture = make_unique<Texture>(*deviceRef, *commandPool, 1, 1, &white);
}

void Scene::createDescriptors() {
    // Uniform buffer
    uniformBuffer = make_unique<vulkan::Buffer>(
        *deviceRef,
        sizeof(UniformBufferObject),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU
    );

    // Descriptor layout with uniform buffer and 5 PBR texture samplers
    vector<VkDescriptorSetLayoutBinding> bindings = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 1,  // Base color
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 2,  // Normal
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 3,  // Metallic-Roughness
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 4,  // Occlusion
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        },
        {
            .binding = 5,  // Emissive
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr
        }
    };
    descriptorLayout = make_unique<vulkan::DescriptorSetLayout>(*deviceRef, bindings);

    // Descriptor pool - reserve space for multiple materials + 1 default
    constexpr uint32_t MAX_MATERIALS = 16;
    vector<VkDescriptorPoolSize> poolSizes = {
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = MAX_MATERIALS + 1},
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 5 * (MAX_MATERIALS + 1)}
    };
    descriptorPool = make_unique<vulkan::DescriptorPool>(*deviceRef, poolSizes, MAX_MATERIALS + 1);

    // Default descriptor set (for meshes without materials)
    defaultDescriptorSet = make_unique<vulkan::DescriptorSet>(*descriptorPool, *descriptorLayout);
    defaultDescriptorSet->updateBuffer(0, uniformBuffer->handle(), 0, sizeof(UniformBufferObject));
    defaultDescriptorSet->updateImage(1, defaultTexture->view(), defaultTexture->sampler());
    defaultDescriptorSet->updateImage(2, defaultTexture->view(), defaultTexture->sampler());
    defaultDescriptorSet->updateImage(3, defaultTexture->view(), defaultTexture->sampler());
    defaultDescriptorSet->updateImage(4, defaultTexture->view(), defaultTexture->sampler());
    defaultDescriptorSet->updateImage(5, defaultTexture->view(), defaultTexture->sampler());
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
    auto loaded = ModelLoader::load(*deviceRef, *commandPool, path);

    // Move textures
    for (auto& tex : loaded.textures) {
        textures.push_back(std::move(tex));
    }

    // Move materials
    for (auto& mat : loaded.materials) {
        materials.push_back(mat);
    }

    // Move meshes
    for (auto& mesh : loaded.meshes) {
        loadedMeshes.push_back(std::move(mesh));
    }

    // Create descriptor set for each material
    for (const auto& mat : materials) {
        auto ds = make_unique<vulkan::DescriptorSet>(*descriptorPool, *descriptorLayout);
        ds->updateBuffer(0, uniformBuffer->handle(), 0, sizeof(UniformBufferObject));

        // Helper to get texture or default
        auto getTexture = [&](int index) -> Texture& {
            if (index >= 0 && index < static_cast<int>(textures.size()) && textures[index]) {
                return *textures[index];
            }
            return *defaultTexture;
        };

        Texture& baseColor = getTexture(mat.baseColorTexture);
        Texture& normal = getTexture(mat.normalTexture);
        Texture& metallicRoughness = getTexture(mat.metallicRoughnessTexture);
        Texture& occlusion = getTexture(mat.occlusionTexture);
        Texture& emissive = getTexture(mat.emissiveTexture);

        ds->updateImage(1, baseColor.view(), baseColor.sampler());
        ds->updateImage(2, normal.view(), normal.sampler());
        ds->updateImage(3, metallicRoughness.view(), metallicRoughness.sampler());
        ds->updateImage(4, occlusion.view(), occlusion.sampler());
        ds->updateImage(5, emissive.view(), emissive.sampler());

        materialDescriptorSets.push_back(std::move(ds));
    }
}

void Scene::addTriangle() {
    vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    vector<uint32_t> indices = {0, 1, 2};

    LoadedMesh loadedMesh;
    loadedMesh.mesh = make_unique<Mesh>(*deviceRef, vertices, indices);
    loadedMeshes.push_back(std::move(loadedMesh));
}

void Scene::update(float time, float aspect) {
    glm::vec3 camPos = glm::vec3(2.0f, 2.0f, 2.0f);

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;  // Flip Y for Vulkan
    ubo.camPos = camPos;

    uniformBuffer->upload(&ubo, sizeof(ubo));
}

void Scene::render(VkCommandBuffer cmd) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());

    for (const auto& loadedMesh : loadedMeshes) {
        // Select descriptor set based on material index
        VkDescriptorSet ds;
        int matIdx = loadedMesh.materialIndex;
        if (matIdx >= 0 && matIdx < static_cast<int>(materialDescriptorSets.size())) {
            ds = materialDescriptorSets[matIdx]->handle();
        } else {
            ds = defaultDescriptorSet->handle();
        }

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout(), 0, 1, &ds, 0, nullptr);
        loadedMesh.mesh->draw(cmd);
    }
}

} // namespace anim::renderer
