#pragma once

#include <mbgl/programs/program.hpp>
#include <mbgl/programs/attributes.hpp>
#include <mbgl/programs/heatmap_texture_program.hpp>
#include <mbgl/programs/uniforms.hpp>
#include <mbgl/util/geometry.hpp>
#include <mbgl/style/layers/heatmap_layer_properties.hpp>

namespace mbgl {

namespace uniforms {
MBGL_DEFINE_UNIFORM_SCALAR(float, intensity);
} // namespace uniforms

class HeatmapProgram final
    : public Program<HeatmapProgram,
                     shaders::BuiltIn::HeatmapProgram,
                     gfx::PrimitiveType::Triangle,
                     TypeList<attributes::pos>,
                     TypeList<uniforms::intensity, uniforms::matrix, uniforms::heatmap::extrude_scale>,
                     TypeList<>,
                     style::HeatmapPaintProperties> {
public:
    static constexpr std::string_view Name{"HeatmapProgram"};
    const std::string_view typeName() const noexcept override { return Name; }

    using Program::Program;

    /*
     * @param {number} x vertex position
     * @param {number} y vertex position
     * @param {number} ex extrude normal
     * @param {number} ey extrude normal
     */
    static LayoutVertex vertex(Point<int16_t> p, float ex, float ey) {
        return LayoutVertex{
            {{static_cast<int16_t>((p.x * 2) + ((ex + 1) / 2)), static_cast<int16_t>((p.y * 2) + ((ey + 1) / 2))}}};
    }
};

using HeatmapLayoutVertex = HeatmapProgram::LayoutVertex;
using HeatmapAttributes = HeatmapProgram::AttributeList;

} // namespace mbgl
