#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "ModelLoader.hpp"

#include <stdexcept>
#include <iostream>

using namespace std;

namespace anim::renderer {

vector<unique_ptr<Mesh>> ModelLoader::load(vulkan::Device& device, const string& path) {
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

    vector<unique_ptr<Mesh>> meshes;

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
                continue;
            }

            vector<Vertex> vertices;
            vector<uint32_t> indices;

            // Get accessors
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
                // No indices - generate sequential
                indices.resize(vertexCount);
                for (size_t i = 0; i < vertexCount; i++) {
                    indices[i] = static_cast<uint32_t>(i);
                }
            }

            meshes.push_back(make_unique<Mesh>(device, vertices, indices));
        }
    }

    cout << "Loaded " << meshes.size() << " mesh(es) from " << path << endl;
    return meshes;
}

} // namespace anim::renderer
