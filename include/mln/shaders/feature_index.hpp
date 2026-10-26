#pragma once

#include <mln/shaders/shader_defines.hpp>
#include <mln/shaders/shader_source.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace mln {
namespace shaders {

/// One paint attribute that may be read from the feature table
struct FeatureAttribute {
    std::size_t id = 0;
    // attribute suffix (`width`, `color`, `pattern_from`), matching
    // `AttributeNames` and the generated `MLN_FEATURE_<name>` constant
    const char* name = "";
    // `floatCount` is 2 for a zoom-interpolated scalar and 4 for a color or one pattern rect.
    std::uint32_t floatCount = 0;
};

/// A data-driven field in one feature record
struct FeatureField {
    std::size_t id = 0;
    const char* name = "";
    std::uint32_t floatCount = 0;
    std::uint32_t offset = 0; // in floats, not bytes
};

/// Tightly packed record layout.
struct FeatureLayout {
    std::vector<FeatureField> fields;
    std::uint32_t stride = 0;
};

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

/// Paint attributes this shader can read from a feature table, in sample order.
/// An empty catalog means the shader does not use a feature table.
/// Another layer adds a specialization; the record format stays the same.
template <BuiltIn ShaderID>
constexpr auto featureAttributes() {
    constexpr std::uint32_t scalar = 2;
    constexpr std::uint32_t wide = 4;
    if constexpr (ShaderID == BuiltIn::LineShader) {
        return std::array{
            FeatureAttribute{.id = idLineColorVertexAttribute, .name = "color", .floatCount = wide},
            FeatureAttribute{.id = idLineBlurVertexAttribute, .name = "blur", .floatCount = scalar},
            FeatureAttribute{.id = idLineOpacityVertexAttribute, .name = "opacity", .floatCount = scalar},
            FeatureAttribute{.id = idLineGapWidthVertexAttribute, .name = "gapwidth", .floatCount = scalar},
            FeatureAttribute{.id = idLineOffsetVertexAttribute, .name = "offset", .floatCount = scalar},
            FeatureAttribute{.id = idLineWidthVertexAttribute, .name = "width", .floatCount = scalar},
        };
    } else if constexpr (ShaderID == BuiltIn::LineGradientShader) {
        // Color comes from the gradient ramp.
        return std::array{
            FeatureAttribute{.id = idLineBlurVertexAttribute, .name = "blur", .floatCount = scalar},
            FeatureAttribute{.id = idLineOpacityVertexAttribute, .name = "opacity", .floatCount = scalar},
            FeatureAttribute{.id = idLineGapWidthVertexAttribute, .name = "gapwidth", .floatCount = scalar},
            FeatureAttribute{.id = idLineOffsetVertexAttribute, .name = "offset", .floatCount = scalar},
            FeatureAttribute{.id = idLineWidthVertexAttribute, .name = "width", .floatCount = scalar},
        };
    } else if constexpr (ShaderID == BuiltIn::LinePatternShader) {
        return std::array{
            FeatureAttribute{.id = idLineBlurVertexAttribute, .name = "blur", .floatCount = scalar},
            FeatureAttribute{.id = idLineOpacityVertexAttribute, .name = "opacity", .floatCount = scalar},
            FeatureAttribute{.id = idLineGapWidthVertexAttribute, .name = "gapwidth", .floatCount = scalar},
            FeatureAttribute{.id = idLineOffsetVertexAttribute, .name = "offset", .floatCount = scalar},
            FeatureAttribute{.id = idLineWidthVertexAttribute, .name = "width", .floatCount = scalar},
            FeatureAttribute{.id = idLineFloorWidthVertexAttribute, .name = "floorwidth", .floatCount = scalar},
            FeatureAttribute{.id = idLinePatternFromVertexAttribute, .name = "pattern_from", .floatCount = wide},
            FeatureAttribute{.id = idLinePatternToVertexAttribute, .name = "pattern_to", .floatCount = wide},
        };
    } else if constexpr (ShaderID == BuiltIn::LineSDFShader) {
        return std::array{
            FeatureAttribute{.id = idLineColorVertexAttribute, .name = "color", .floatCount = wide},
            FeatureAttribute{.id = idLineBlurVertexAttribute, .name = "blur", .floatCount = scalar},
            FeatureAttribute{.id = idLineOpacityVertexAttribute, .name = "opacity", .floatCount = scalar},
            FeatureAttribute{.id = idLineGapWidthVertexAttribute, .name = "gapwidth", .floatCount = scalar},
            FeatureAttribute{.id = idLineOffsetVertexAttribute, .name = "offset", .floatCount = scalar},
            FeatureAttribute{.id = idLineWidthVertexAttribute, .name = "width", .floatCount = scalar},
            FeatureAttribute{.id = idLineFloorWidthVertexAttribute, .name = "floorwidth", .floatCount = scalar},
        };
    } else if constexpr (ShaderID == BuiltIn::LineGradientSDFShader) {
        // Color comes from the gradient ramp.
        return std::array{
            FeatureAttribute{.id = idLineBlurVertexAttribute, .name = "blur", .floatCount = scalar},
            FeatureAttribute{.id = idLineOpacityVertexAttribute, .name = "opacity", .floatCount = scalar},
            FeatureAttribute{.id = idLineGapWidthVertexAttribute, .name = "gapwidth", .floatCount = scalar},
            FeatureAttribute{.id = idLineOffsetVertexAttribute, .name = "offset", .floatCount = scalar},
            FeatureAttribute{.id = idLineWidthVertexAttribute, .name = "width", .floatCount = scalar},
            FeatureAttribute{.id = idLineFloorWidthVertexAttribute, .name = "floorwidth", .floatCount = scalar},
        };
    } else {
        return std::array<FeatureAttribute, 0>{};
    }
}

/// Construct a feature layout from non-uniform attributes
template <typename Catalog, typename UniformIds>
FeatureLayout featureLayout(const Catalog& catalog, const UniformIds& uniformIds) {
    FeatureLayout layout;
    for (const auto& attribute : catalog) {
        if (uniformIds.contains(attribute.id)) {
            continue;
        }
        layout.fields.push_back(FeatureField{
            .id = attribute.id, .name = attribute.name, .floatCount = attribute.floatCount, .offset = layout.stride});
        layout.stride += attribute.floatCount;
    }
    return layout;
}

/// WGSL constants for one layout. Metal and Vulkan receive the same names as
/// preprocessor definitions, because those preludes are compiled before any
/// appended source and their preprocessors substitute values. WebGPU's
/// preprocessor only keeps or drops `ifdef` blocks, so the constants are source.
inline std::string featureLayoutSource(const FeatureLayout& layout) {
    std::ostringstream source;
    source << "const MLN_FEATURE_STRIDE: u32 = " << layout.stride << "u;\n";
    for (const auto& field : layout.fields) {
        source << "const MLN_FEATURE_" << field.name << ": u32 = " << field.offset << "u;\n";
    }
    return source.str();
}

/// WGSL loaders for a feature table. Append these after the shader, which declares
/// `var<storage, read> features`. A storage pointer is not a legal function parameter,
/// and the shared prelude is concatenated after preprocessing, so it cannot contain
/// `#ifdef` or these functions.
inline std::string featureLoadSource() {
    return R"(
fn featureFloat2(base: u32, index: u32, offset: u32) -> vec2<f32> {
    let i = (base + index) * MLN_FEATURE_STRIDE + offset;
    return vec2<f32>(features[i], features[i + 1u]);
}
fn featureFloat4(base: u32, index: u32, offset: u32) -> vec4<f32> {
    let i = (base + index) * MLN_FEATURE_STRIDE + offset;
    return vec4<f32>(features[i], features[i + 1u], features[i + 2u], features[i + 3u]);
}
)";
}

template <typename Defines>
void defineFeatureConstants(const FeatureLayout& layout, Defines& defines) {
    defines.insert(std::make_pair("MLN_FEATURE_STRIDE", std::to_string(layout.stride)));
    for (const auto& field : layout.fields) {
        defines.insert(std::make_pair(std::string("MLN_FEATURE_") + field.name, std::to_string(field.offset)));
    }
}

/// Pack `featureCount` records. `lookup` returns the float4 table for an attribute id,
/// or null when that field has no values. Missing entries stay zero. Each field copies
/// its `floatCount` leading components.
template <typename Lookup>
std::vector<float> packFeatureTable(const FeatureLayout& layout, std::size_t featureCount, Lookup&& lookup) {
    std::vector<float> packed(featureCount * layout.stride, 0.f);
    if (layout.stride == 0 || featureCount == 0) {
        return packed;
    }
    for (std::size_t featureIndex = 0; featureIndex < featureCount; ++featureIndex) {
        for (const auto& field : layout.fields) {
            const auto* values = lookup(field.id);
            if (!values || featureIndex >= values->size()) {
                continue;
            }
            const auto& value = (*values)[featureIndex];
            auto* destination = packed.data() + featureIndex * layout.stride + field.offset;
            std::copy_n(value.begin(), field.floatCount, destination);
        }
    }
    return packed;
}

} // namespace shaders
} // namespace mln
