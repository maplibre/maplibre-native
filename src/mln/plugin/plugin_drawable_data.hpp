#pragma once
#include <mln/gfx/drawable_data.hpp>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <cstdint>

namespace mln::plugin {
struct DrawableData final : gfx::DrawableData {
    explicit DrawableData(std::string shaderID_)
        : shaderID(std::move(shaderID_)) {}
    std::string shaderID;
    struct UniformData {
        std::vector<uint8_t> scratch;
        std::vector<uint8_t> uploaded;
    };
    std::map<uint32_t, UniformData> uniforms;
};
} // namespace mln::plugin
