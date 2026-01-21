#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"
#include "../vulkan/Device.hpp"
#include "../vulkan/CommandPool.hpp"

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <memory>

using namespace std;

namespace anim::renderer {

struct LoadedMaterial {
    // Texture indices into LoadedModel::textures, -1 if not present
    int baseColorTexture = -1;
    int normalTexture = -1;
    int metallicRoughnessTexture = -1;
    int occlusionTexture = -1;
    int emissiveTexture = -1;

    // Fallback factors when textures are not present
    glm::vec4 baseColorFactor{1.0f};
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec3 emissiveFactor{0.0f};
};

struct LoadedMesh {
    unique_ptr<Mesh> mesh;
    int materialIndex = -1;  // Index into LoadedModel::materials, -1 if no material
    glm::mat4 transform{1.0f};  // World transform from node hierarchy
};

struct LoadedModel {
    vector<LoadedMesh> meshes;
    vector<unique_ptr<Texture>> textures;
    vector<LoadedMaterial> materials;
};

class ModelLoader {
public:
    static LoadedModel load(vulkan::Device& device, vulkan::CommandPool& cmdPool, const string& path);
};

} // namespace anim::renderer
