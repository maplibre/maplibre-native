#pragma once

#include <mbgl/programs/program.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/style/properties.hpp>

namespace mbgl {

class ClippingMaskProgram final : public Program<ClippingMaskProgram,
                                                 shaders::BuiltIn::ClippingMaskProgram,
                                                 gfx::PrimitiveType::Triangle,
                                                 PositionOnlyLayoutAttributes,
                                                 TypeList<uniforms::matrix>,
                                                 TypeList<>,
                                                 style::Properties<>> {
public:
    static constexpr std::string_view Name{"ClippingMaskProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;
};

using ClippingMaskLayoutVertex = ClippingMaskProgram::LayoutVertex;
using ClippingMaskAttributes = ClippingMaskProgram::AttributeList;

} // namespace mbgl
