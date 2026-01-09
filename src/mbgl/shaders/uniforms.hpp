#pragma once

#include <mbgl/gfx/uniform.hpp>
#include <mbgl/util/size.hpp>
#include <mbgl/util/color.hpp>

namespace mbgl {
namespace uniforms {

// Legacy matrix uniform used in legacy shader ClippingMaskProgram
MBGL_DEFINE_UNIFORM_MATRIX(double, 4, matrix);

// Legacy uniforms needed by layer properties templates, not used in shaders.
// Shaders are using UBOs in drawable architecture.
MBGL_DEFINE_UNIFORM_SCALAR(float, opacity);
MBGL_DEFINE_UNIFORM_SCALAR(Color, color);
MBGL_DEFINE_UNIFORM_SCALAR(float, blur);
MBGL_DEFINE_UNIFORM_SCALAR(float, radius);
MBGL_DEFINE_UNIFORM_SCALAR(float, stroke_width);
MBGL_DEFINE_UNIFORM_SCALAR(Color, stroke_color);
MBGL_DEFINE_UNIFORM_SCALAR(float, stroke_opacity);
MBGL_DEFINE_UNIFORM_SCALAR(Color, fill_color);
MBGL_DEFINE_UNIFORM_SCALAR(Color, halo_color);
MBGL_DEFINE_UNIFORM_SCALAR(float, halo_width);
MBGL_DEFINE_UNIFORM_SCALAR(float, halo_blur);
MBGL_DEFINE_UNIFORM_SCALAR(Color, outline_color);
MBGL_DEFINE_UNIFORM_SCALAR(float, height);
MBGL_DEFINE_UNIFORM_SCALAR(float, base);
MBGL_DEFINE_UNIFORM_SCALAR(float, width);
MBGL_DEFINE_UNIFORM_SCALAR(float, floorwidth);
MBGL_DEFINE_UNIFORM_SCALAR(float, gapwidth);
MBGL_DEFINE_UNIFORM_SCALAR(float, offset);
MBGL_DEFINE_UNIFORM_SCALAR(float, weight);
MBGL_DEFINE_UNIFORM_VECTOR(uint16_t, 4, pattern_from);
MBGL_DEFINE_UNIFORM_VECTOR(uint16_t, 4, pattern_to);

} // namespace uniforms
} // namespace mbgl
