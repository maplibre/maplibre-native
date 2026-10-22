#pragma once

#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/shader_source.hpp>

#include <array>
#include <cstddef>
#include <optional>

namespace mln {
namespace shaders {

/// Attribute id where this shader's per-vertex feature index is bound.
/// That is the pack's first data-driven attribute id, which for lines is line-color.
/// Another layer returns its own first attribute id. Empty when the shader does not
/// sample a feature table.
template <BuiltIn ShaderID>
constexpr std::optional<std::size_t> featureIndexAttributeId() {
    if constexpr (ShaderID == BuiltIn::LineShader || ShaderID == BuiltIn::LineGradientShader ||
                  ShaderID == BuiltIn::LinePatternShader || ShaderID == BuiltIn::LineSDFShader ||
                  ShaderID == BuiltIn::LineGradientSDFShader) {
        return idLineColorVertexAttribute;
    } else {
        return std::nullopt;
    }
}

/// The paint attribute IDs that these shaders read from the feature table.
/// `MLN_FEATURE_INDEX` is defined only when one of these is data-driven.
/// This allows the feature table binding to be dropped when the shader doesn't
/// read the table, preventing it from being optimized away unexpectedly and
/// shifting the binding indexes.
template <BuiltIn ShaderID>
constexpr auto featureAttributeIds() {
    if constexpr (ShaderID == BuiltIn::LineShader) {
        return std::array{idLineColorVertexAttribute,
                          idLineBlurVertexAttribute,
                          idLineOpacityVertexAttribute,
                          idLineGapWidthVertexAttribute,
                          idLineOffsetVertexAttribute,
                          idLineWidthVertexAttribute};
    } else if constexpr (ShaderID == BuiltIn::LineGradientShader) {
        // Color comes from the gradient ramp.
        return std::array{idLineBlurVertexAttribute,
                          idLineOpacityVertexAttribute,
                          idLineGapWidthVertexAttribute,
                          idLineOffsetVertexAttribute,
                          idLineWidthVertexAttribute};
    } else if constexpr (ShaderID == BuiltIn::LinePatternShader) {
        return std::array{idLineBlurVertexAttribute,
                          idLineOpacityVertexAttribute,
                          idLineGapWidthVertexAttribute,
                          idLineOffsetVertexAttribute,
                          idLineWidthVertexAttribute,
                          idLineFloorWidthVertexAttribute,
                          idLinePatternFromVertexAttribute,
                          idLinePatternToVertexAttribute};
    } else if constexpr (ShaderID == BuiltIn::LineSDFShader) {
        return std::array{idLineColorVertexAttribute,
                          idLineBlurVertexAttribute,
                          idLineOpacityVertexAttribute,
                          idLineGapWidthVertexAttribute,
                          idLineOffsetVertexAttribute,
                          idLineWidthVertexAttribute,
                          idLineFloorWidthVertexAttribute};
    } else if constexpr (ShaderID == BuiltIn::LineGradientSDFShader) {
        // Color comes from the gradient ramp.
        return std::array{idLineBlurVertexAttribute,
                          idLineOpacityVertexAttribute,
                          idLineGapWidthVertexAttribute,
                          idLineOffsetVertexAttribute,
                          idLineWidthVertexAttribute,
                          idLineFloorWidthVertexAttribute};
    } else {
        return std::array<std::size_t, 0>{};
    }
}

} // namespace shaders
} // namespace mln
