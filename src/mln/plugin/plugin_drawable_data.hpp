#pragma once
#include <mln/gfx/drawable_data.hpp>
#include <string>
#include <utility>

namespace mln::plugin {
struct DrawableData final : gfx::DrawableData {
    explicit DrawableData(std::string shaderID_)
        : shaderID(std::move(shaderID_)) {}
    std::string shaderID;
};
} // namespace mln::plugin
