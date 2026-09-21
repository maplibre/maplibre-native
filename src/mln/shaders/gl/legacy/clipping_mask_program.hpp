#pragma once

#include <mln/shaders/gl/legacy/program.hpp>
#include <mln/shaders/attributes.hpp>
#include <mln/shaders/uniforms.hpp>
#include <mln/style/properties.hpp>

namespace mln {

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

} // namespace mln
