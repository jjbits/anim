#pragma once

#include "Mesh.hpp"
#include "../vulkan/Device.hpp"

#include <string>
#include <vector>
#include <memory>

using namespace std;

namespace anim::renderer {

class ModelLoader {
public:
    static vector<unique_ptr<Mesh>> load(vulkan::Device& device, const string& path);
};

} // namespace anim::renderer
