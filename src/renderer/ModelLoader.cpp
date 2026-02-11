#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "ModelLoader.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <stdexcept>
#include <iostream>
#include <functional>

using namespace std;
using namespace glm;

namespace anim::renderer {

// Helper to get image index from a texture info
static int getImageIndex(const tinygltf::Model& model, int textureIndex) {
    if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size())) {
        return -1;
    }
    return model.textures[textureIndex].source;
}

// Get local transform matrix from a glTF node
static mat4 getNodeTransform(const tinygltf::Node& node) {
    if (!node.matrix.empty()) {
        return make_mat4(node.matrix.data());
    }

    mat4 T{1.0f}, R{1.0f}, S{1.0f};

    if (!node.translation.empty()) {
        T = translate(mat4{1.0f}, vec3(node.translation[0], node.translation[1], node.translation[2]));
    }
    if (!node.rotation.empty()) {
        quat q(static_cast<float>(node.rotation[3]),
               static_cast<float>(node.rotation[0]),
               static_cast<float>(node.rotation[1]),
               static_cast<float>(node.rotation[2]));
        R = mat4_cast(q);
    }
    if (!node.scale.empty()) {
        vec3 s(node.scale[0], node.scale[1], node.scale[2]);
        S = scale(mat4{1.0f}, s);
    }

    return T * R * S;
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
            result.textures.push_back(nullptr);
            continue;
        }

        // tinygltf may provide RGB (3 components) or RGBA (4 components)
        // Our Texture class expects RGBA, so convert if needed
        if (image.component == 3) {
            // Convert RGB to RGBA
            vector<uint8_t> rgba(image.width * image.height * 4);
            const uint8_t* src = image.image.data();
            for (int i = 0; i < image.width * image.height; i++) {
                rgba[i * 4 + 0] = src[i * 3 + 0];  // R
                rgba[i * 4 + 1] = src[i * 3 + 1];  // G
                rgba[i * 4 + 2] = src[i * 3 + 2];  // B
                rgba[i * 4 + 3] = 255;             // A
            }
            result.textures.push_back(make_unique<Texture>(
                device, cmdPool,
                image.width, image.height,
                rgba.data()
            ));
        } else {
            // Already RGBA (or other 4-component format)
            result.textures.push_back(make_unique<Texture>(
                device, cmdPool,
                image.width, image.height,
                image.image.data()
            ));
        }
    }

    // Load materials
    for (size_t matIdx = 0; matIdx < model.materials.size(); matIdx++) {
        const auto& mat = model.materials[matIdx];
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

    // Helper lambda to load a primitive
    auto loadPrimitive = [&](const tinygltf::Primitive& primitive, mat4 transform) -> LoadedMesh {
        vector<Vertex> vertices;
        vector<uint32_t> indices;

        const uint8_t* positionData = nullptr;
        const uint8_t* normalData = nullptr;
        const uint8_t* texcoordData = nullptr;
        const uint8_t* tangentData = nullptr;
        size_t positionStride = 0;
        size_t normalStride = 0;
        size_t texcoordStride = 0;
        size_t tangentStride = 0;
        size_t vertexCount = 0;

        // Position (required)
        if (primitive.attributes.count("POSITION")) {
            const auto& accessor = model.accessors[primitive.attributes.at("POSITION")];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            positionData = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            positionStride = bufferView.byteStride ? bufferView.byteStride : sizeof(float) * 3;
            vertexCount = accessor.count;
        }

        // Normal (optional)
        if (primitive.attributes.count("NORMAL")) {
            const auto& accessor = model.accessors[primitive.attributes.at("NORMAL")];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            normalData = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            normalStride = bufferView.byteStride ? bufferView.byteStride : sizeof(float) * 3;
        }

        // Tangent (optional)
        if (primitive.attributes.count("TANGENT")) {
            const auto& accessor = model.accessors[primitive.attributes.at("TANGENT")];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            tangentData = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            tangentStride = bufferView.byteStride ? bufferView.byteStride : sizeof(float) * 4;
        }

        // Texcoord (optional)
        if (primitive.attributes.count("TEXCOORD_0")) {
            const auto& accessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
            const auto& bufferView = model.bufferViews[accessor.bufferView];
            const auto& buffer = model.buffers[bufferView.buffer];
            texcoordData = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
            texcoordStride = bufferView.byteStride ? bufferView.byteStride : sizeof(float) * 2;
        }

        // Build vertices
        vertices.resize(vertexCount);
        for (size_t i = 0; i < vertexCount; i++) {
            const float* pos = reinterpret_cast<const float*>(positionData + i * positionStride);
            vertices[i].position = vec3(pos[0], pos[1], pos[2]);

            if (normalData) {
                const float* norm = reinterpret_cast<const float*>(normalData + i * normalStride);
                vertices[i].normal = vec3(norm[0], norm[1], norm[2]);
            } else {
                vertices[i].normal = vec3(0.0f, 1.0f, 0.0f);
            }

            if (texcoordData) {
                const float* uv = reinterpret_cast<const float*>(texcoordData + i * texcoordStride);
                vertices[i].uv = vec2(uv[0], uv[1]);
            } else {
                vertices[i].uv = vec2(0.0f);
            }

            if (tangentData) {
                const float* tan = reinterpret_cast<const float*>(tangentData + i * tangentStride);
                vertices[i].tangent = vec4(tan[0], tan[1], tan[2], tan[3]);
            } else {
                vertices[i].tangent = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
        loadedMesh.transform = transform;
        return loadedMesh;
    };

    // Recursive function to process nodes
    function<void(int, mat4)> processNode = [&](int nodeIndex, mat4 parentTransform) {
        const auto& node = model.nodes[nodeIndex];
        mat4 localTransform = getNodeTransform(node);
        mat4 worldTransform = parentTransform * localTransform;

        // If node has a mesh, load all its primitives
        if (node.mesh >= 0) {
            const auto& mesh = model.meshes[node.mesh];
            for (const auto& primitive : mesh.primitives) {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                    continue;
                }
                if (!primitive.attributes.count("POSITION")) {
                    continue;
                }
                result.meshes.push_back(loadPrimitive(primitive, worldTransform));
            }
        }

        // Process children
        for (int childIndex : node.children) {
            processNode(childIndex, worldTransform);
        }
    };

    // Process all scenes
    for (const auto& scene : model.scenes) {
        for (int nodeIndex : scene.nodes) {
            processNode(nodeIndex, mat4{1.0f});
        }
    }

    cout << "Loaded " << result.meshes.size() << " mesh(es), "
         << result.textures.size() << " texture(s), "
         << result.materials.size() << " material(s) from " << path << endl;

    return result;
}

} // namespace anim::renderer
