// Generated code, do not modify this file!
// NOLINTBEGIN
#pragma once
#include <mbgl/shaders/shader_source.hpp>

#if MLN_RENDER_BACKEND_OPENGL
#include <mbgl/shaders/gl/drawable_background.hpp>
#include <mbgl/shaders/gl/drawable_background_pattern.hpp>
#include <mbgl/shaders/gl/drawable_circle.hpp>
#include <mbgl/shaders/gl/drawable_collision_box.hpp>
#include <mbgl/shaders/gl/drawable_collision_circle.hpp>
#include <mbgl/shaders/gl/drawable_debug.hpp>
#include <mbgl/shaders/gl/drawable_fill.hpp>
#include <mbgl/shaders/gl/drawable_fill_outline.hpp>
#include <mbgl/shaders/gl/drawable_line_gradient.hpp>
#include <mbgl/shaders/gl/drawable_line_pattern.hpp>
#include <mbgl/shaders/gl/drawable_line_sdf.hpp>
#include <mbgl/shaders/gl/drawable_line.hpp>
#include <mbgl/shaders/gl/drawable_fill_pattern.hpp>
#include <mbgl/shaders/gl/drawable_fill_outline_pattern.hpp>
#include <mbgl/shaders/gl/drawable_fill_extrusion.hpp>
#include <mbgl/shaders/gl/drawable_fill_extrusion_pattern.hpp>
#include <mbgl/shaders/gl/drawable_heatmap.hpp>
#include <mbgl/shaders/gl/drawable_heatmap_texture.hpp>
#include <mbgl/shaders/gl/drawable_hillshade_prepare.hpp>
#include <mbgl/shaders/gl/drawable_hillshade.hpp>
#include <mbgl/shaders/gl/drawable_raster.hpp>
#include <mbgl/shaders/gl/drawable_symbol_icon.hpp>
#include <mbgl/shaders/gl/drawable_symbol_sdf.hpp>
#include <mbgl/shaders/gl/drawable_symbol_text_and_icon.hpp>
#include <mbgl/shaders/gl/prelude.hpp>
#include <mbgl/shaders/gl/background.hpp>
#include <mbgl/shaders/gl/background_pattern.hpp>
#include <mbgl/shaders/gl/circle.hpp>
#include <mbgl/shaders/gl/clipping_mask.hpp>
#include <mbgl/shaders/gl/collision_box.hpp>
#include <mbgl/shaders/gl/collision_circle.hpp>
#include <mbgl/shaders/gl/debug.hpp>
#include <mbgl/shaders/gl/fill_extrusion_pattern.hpp>
#include <mbgl/shaders/gl/fill_extrusion.hpp>
#include <mbgl/shaders/gl/fill_outline_pattern.hpp>
#include <mbgl/shaders/gl/fill_outline.hpp>
#include <mbgl/shaders/gl/fill_pattern.hpp>
#include <mbgl/shaders/gl/fill.hpp>
#include <mbgl/shaders/gl/heatmap_texture.hpp>
#include <mbgl/shaders/gl/heatmap.hpp>
#include <mbgl/shaders/gl/hillshade_prepare.hpp>
#include <mbgl/shaders/gl/hillshade.hpp>
#include <mbgl/shaders/gl/line_gradient.hpp>
#include <mbgl/shaders/gl/line_pattern.hpp>
#include <mbgl/shaders/gl/line_sdf.hpp>
#include <mbgl/shaders/gl/line.hpp>
#include <mbgl/shaders/gl/raster.hpp>
#include <mbgl/shaders/gl/symbol_icon.hpp>
#include <mbgl/shaders/gl/symbol_sdf_text.hpp>
#include <mbgl/shaders/gl/symbol_sdf_icon.hpp>
#include <mbgl/shaders/gl/symbol_text_and_icon.hpp>
#endif
#if MLN_RENDER_BACKEND_METAL
#include <mbgl/shaders/mtl/prelude.hpp>
#include <mbgl/shaders/mtl/background.hpp>
#include <mbgl/shaders/mtl/background_pattern.hpp>
#include <mbgl/shaders/mtl/circle.hpp>
#include <mbgl/shaders/mtl/clipping_mask.hpp>
#include <mbgl/shaders/mtl/collision_box.hpp>
#include <mbgl/shaders/mtl/collision_circle.hpp>
#include <mbgl/shaders/mtl/fill_extrusion_pattern.hpp>
#include <mbgl/shaders/mtl/fill_extrusion.hpp>
#include <mbgl/shaders/mtl/fill.hpp>
#include <mbgl/shaders/mtl/fill_outline.hpp>
#include <mbgl/shaders/mtl/fill_pattern.hpp>
#include <mbgl/shaders/mtl/fill_outline_pattern.hpp>
#include <mbgl/shaders/mtl/heatmap_texture.hpp>
#include <mbgl/shaders/mtl/heatmap.hpp>
#include <mbgl/shaders/mtl/hillshade_prepare.hpp>
#include <mbgl/shaders/mtl/hillshade.hpp>
#include <mbgl/shaders/mtl/line.hpp>
#include <mbgl/shaders/mtl/line_pattern.hpp>
#include <mbgl/shaders/mtl/line_sdf.hpp>
#include <mbgl/shaders/mtl/line_gradient.hpp>
#include <mbgl/shaders/mtl/raster.hpp>
#include <mbgl/shaders/mtl/symbol_icon.hpp>
#include <mbgl/shaders/mtl/symbol_sdf.hpp>
#include <mbgl/shaders/mtl/symbol_text_and_icon.hpp>
#endif

namespace mbgl {
namespace shaders {

struct ReflectionData;

/// @brief Get the name of the given shader ID as a string
std::string getProgramName(BuiltIn programID);

template <gfx::Backend::Type>
std::pair<std::string, std::string> getShaderSource(shaders::BuiltIn programID);

template <gfx::Backend::Type>
const ReflectionData& getReflectionData(BuiltIn programID);

} // namespace shaders
} // namespace mbgl

// NOLINTEND
