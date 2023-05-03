#pragma once

#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/program.hpp>
#include <mbgl/programs/textures.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/style/properties.hpp>

namespace mbgl {

namespace uniforms {
MBGL_DEFINE_UNIFORM_SCALAR(float, overlay_scale);
}
class DebugProgram final : public Program<DebugProgram,
                                          shaders::BuiltIn::DebugProgram,
                                          gfx::PrimitiveType::Line,
                                          TypeList<attributes::pos>,
                                          TypeList<uniforms::matrix, uniforms::color, uniforms::overlay_scale>,
                                          TypeList<textures::overlay>,
                                          style::Properties<>> {
public:
    static constexpr std::string_view Name{"DebugProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;
};

using DebugLayoutVertex = DebugProgram::LayoutVertex;
using DebugAttributes = DebugProgram::AttributeList;

} // namespace mbgl
