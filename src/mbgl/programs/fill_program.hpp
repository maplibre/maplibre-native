#pragma once

#include <mbgl/programs/program.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/programs/textures.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/style/layers/fill_layer_properties.hpp>

#include <string>

namespace mbgl {

class ImagePosition;
class UnwrappedTileID;
class TransformState;
template <class>
class Faded;

using FillLayoutAttributes = PositionOnlyLayoutAttributes;

using FillUniforms = TypeList<uniforms::matrix, uniforms::world>;

using FillPatternUniforms = TypeList<uniforms::matrix,
                                     uniforms::world,
                                     uniforms::texsize,
                                     uniforms::scale,
                                     uniforms::fade,
                                     uniforms::pixel_coord_upper,
                                     uniforms::pixel_coord_lower>;

class FillProgram final : public Program<FillProgram,
                                         shaders::BuiltIn::FillProgram,
                                         gfx::PrimitiveType::Triangle,
                                         FillLayoutAttributes,
                                         FillUniforms,
                                         TypeList<>,
                                         style::FillPaintProperties> {
public:
    static constexpr std::string_view Name{"FillProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;

    static LayoutVertex layoutVertex(Point<int16_t> p) { return LayoutVertex{{{p.x, p.y}}}; }
};

class FillPatternProgram final : public Program<FillPatternProgram,
                                                shaders::BuiltIn::FillPatternProgram,
                                                gfx::PrimitiveType::Triangle,
                                                FillLayoutAttributes,
                                                FillPatternUniforms,
                                                TypeList<textures::image>,
                                                style::FillPaintProperties> {
public:
    static constexpr std::string_view Name{"FillPatternProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;

    static LayoutUniformValues layoutUniformValues(mat4 matrix,
                                                   Size framebufferSize,
                                                   Size atlasSize,
                                                   const CrossfadeParameters& crossfade,
                                                   const UnwrappedTileID&,
                                                   const TransformState&,
                                                   float pixelRatio);
};

class FillOutlineProgram final : public Program<FillOutlineProgram,
                                                shaders::BuiltIn::FillOutlineProgram,
                                                gfx::PrimitiveType::Line,
                                                FillLayoutAttributes,
                                                FillUniforms,
                                                TypeList<>,
                                                style::FillPaintProperties> {
public:
    static constexpr std::string_view Name{"FillOutlineProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;
};

class FillOutlinePatternProgram final : public Program<FillOutlinePatternProgram,
                                                       shaders::BuiltIn::FillOutlinePatternProgram,
                                                       gfx::PrimitiveType::Line,
                                                       FillLayoutAttributes,
                                                       FillPatternUniforms,
                                                       TypeList<textures::image>,
                                                       style::FillPaintProperties> {
public:
    static constexpr std::string_view Name{"FillOutlinePatternProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;
};

using FillLayoutVertex = FillProgram::LayoutVertex;
using FillAttributes = FillProgram::AttributeList;

} // namespace mbgl
