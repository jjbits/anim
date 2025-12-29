#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "ModelLoader.hpp"

#include <stdexcept>
#include <iostream>

using namespace std;

namespace anim::renderer {

// Helper to get image index from a texture info
static int getImageIndex(const tinygltf::Model& model, int textureIndex) {
    if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size())) {
        return -1;
    }
    return model.textures[textureIndex].source;
}

LoadedModel ModelLoader::load(vulkan::Device& device, vulkan::CommandPool& cmdPool, const string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    string err, warn;

    bool success = false;
    if (path.ends_with(".glb")) {
        success = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    } else {
        success = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }

    if (!warn.empty()) {
        cout << "glTF warning: " << warn << endl;
    }
    if (!err.empty()) {
        throw runtime_error("glTF error: " + err);
    }
    if (!success) {
        throw runtime_error("Failed to load glTF: " + path);
    }

    LoadedModel result;

    // Load all images as textures
    for (const auto& image : model.images) {
        if (image.image.empty()) {
            cout << "Skipping empty image" << endl;
            result.textures.push_back(nullptr);
            continue;
        }

        result.textures.push_back(make_unique<Texture>(
            device, cmdPool,
            image.width, image.height,
            image.image.data()
        ));
        cout << "Loaded texture: " << image.width << "x" << image.height << endl;
    }

    // Load materials
    for (const auto& mat : model.materials) {
        LoadedMaterial loadedMat;

        // Base color
        const auto& pbr = mat.pbrMetallicRoughness;
        loadedMat.baseColorTexture = getImageIndex(model, pbr.baseColorTexture.index);
        loadedMat.baseColorFactor = glm::vec4(
            pbr.baseColorFactor[0],
            pbr.baseColorFactor[1],
            pbr.baseColorFactor[2],
            pbr.baseColorFactor[3]
        );

        // Metallic-roughness
        loadedMat.metallicRoughnessTexture = getImageIndex(model, pbr.metallicRoughnessTexture.index);
        loadedMat.metallicFactor = static_cast<float>(pbr.metallicFactor);
        loadedMat.roughnessFactor = static_cast<float>(pbr.roughnessFactor);

        // Normal map
        loadedMat.normalTexture = getImageIndex(model, mat.normalTexture.index);

        // Occlusion
        loadedMat.occlusionTexture = getImageIndex(model, mat.occlusionTexture.index);

        // Emissive
        loadedMat.emissiveTexture = getImageIndex(model, mat.emissiveTexture.index);
        loadedMat.emissiveFactor = glm::vec3(
            mat.emissiveFactor[0],
            mat.emissiveFactor[1],
            mat.emissiveFactor[2]
        );

        result.materials.push_back(loadedMat);
    }

    // Load meshes
    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                continue;
            }

            vector<Vertex> vertices;
            vector<uint32_t> indices;

            const float* positions = nullptr;
            const float* normals = nullptr;
            const float* texcoords = nullptr;
            size_t vertexCount = 0;

            // Position (required)
            if (primitive.attributes.count("POSITION")) {
                const auto& accessor = model.accessors[primitive.attributes.at("POSITION")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                positions = reinterpret_cast<const float*>(
                    buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
                vertexCount = accessor.count;
            } else {
                continue;
            }

            // Normal (optional)
            if (primitive.attributes.count("NORMAL")) {
                const auto& accessor = model.accessors[primitive.attributes.at("NORMAL")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                normals = reinterpret_cast<const float*>(
                    buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
            }

            // Texcoord (optional)
            if (primitive.attributes.count("TEXCOORD_0")) {
                const auto& accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                texcoords = reinterpret_cast<const float*>(
                    buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
            }

            // Build vertices
            vertices.resize(vertexCount);
            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].position = glm::vec3(
                    positions[i * 3 + 0],
                    positions[i * 3 + 1],
                    positions[i * 3 + 2]
                );

                if (normals) {
                    vertices[i].normal = glm::vec3(
                        normals[i * 3 + 0],
                        normals[i * 3 + 1],
                        normals[i * 3 + 2]
                    );
                } else {
                    vertices[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                if (texcoords) {
                    vertices[i].uv = glm::vec2(
                        texcoords[i * 2 + 0],
                        texcoords[i * 2 + 1]
                    );
                } else {
                    vertices[i].uv = glm::vec2(0.0f);
                }
            }

            // Build indices
            if (primitive.indices >= 0) {
                const auto& accessor = model.accessors[primitive.indices];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const uint8_t* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                indices.resize(accessor.count);

                switch (accessor.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                        const uint16_t* ptr = reinterpret_cast<const uint16_t*>(data);
                        for (size_t i = 0; i < accessor.count; i++) {
                            indices[i] = ptr[i];
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                        const uint32_t* ptr = reinterpret_cast<const uint32_t*>(data);
                        for (size_t i = 0; i < accessor.count; i++) {
                            indices[i] = ptr[i];
                        }
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                        for (size_t i = 0; i < accessor.count; i++) {
                            indices[i] = data[i];
                        }
                        break;
                    }
                    default:
                        throw runtime_error("Unsupported index component type");
                }
            } else {
                indices.resize(vertexCount);
                for (size_t i = 0; i < vertexCount; i++) {
                    indices[i] = static_cast<uint32_t>(i);
                }
            }

            LoadedMesh loadedMesh;
            loadedMesh.mesh = make_unique<Mesh>(device, vertices, indices);
            loadedMesh.materialIndex = primitive.material;

            result.meshes.push_back(std::move(loadedMesh));
        }
    }

    cout << "Loaded " << result.meshes.size() << " mesh(es), "
         << result.textures.size() << " texture(s), "
         << result.materials.size() << " material(s) from " << path << endl;
    return result;
}

} // namespace anim::renderer
