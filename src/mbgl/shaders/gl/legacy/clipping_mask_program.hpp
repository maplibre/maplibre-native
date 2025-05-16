#pragma once

#include <mbgl/shaders/gl/legacy/program.hpp>
#include <mbgl/shaders/attributes.hpp>
#include <mbgl/shaders/uniforms.hpp>
#include <mbgl/style/properties.hpp>

namespace mbgl {

class ClippingMaskProgram final : public Program<ClippingMaskProgram,
                                                 shaders::BuiltIn::ClippingMaskProgram,
                                                 gfx::PrimitiveType::Triangle,
                                                 PositionOnlyLayoutAttributes,
                                                 TypeList<uniforms::matrix>,
                                                 style::Properties<>> {
public:
    static constexpr std::string_view Name{"ClippingMaskProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;
};

using ClippingMaskLayoutVertex = ClippingMaskProgram::LayoutVertex;
using ClippingMaskAttributes = ClippingMaskProgram::AttributeList;

} // namespace mbgl
