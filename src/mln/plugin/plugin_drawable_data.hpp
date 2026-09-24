#pragma once
#include <mln/gfx/drawable_data.hpp>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <cstdint>

namespace mln::plugin {
struct DrawableData final : gfx::DrawableData {
    explicit DrawableData(std::string shaderID_, uint32_t passIndex_ = 0)
        : shaderID(std::move(shaderID_)), passIndex(passIndex_) {}
    std::string shaderID;
    uint32_t passIndex;
    bool uniformFailed = false;
    struct UniformData {
        std::vector<uint8_t> scratch;
        std::vector<uint8_t> uploaded;
    };
    std::map<uint32_t, UniformData> uniforms;
};
} // namespace mln::plugin
