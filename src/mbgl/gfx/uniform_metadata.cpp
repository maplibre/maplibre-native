#include <mbgl/gfx/uniform_metadata.hpp>

#if MLN_DRAWABLE_RENDERER
//#ifndef NDEBUG

#include <mbgl/shaders/background_layer_ubo.hpp>
#include <mbgl/shaders/circle_layer_ubo.hpp>
#include <mbgl/shaders/collision_layer_ubo.hpp>
#include <mbgl/shaders/debug_layer_ubo.hpp>
#include <mbgl/shaders/fill_extrusion_layer_ubo.hpp>
#include <mbgl/shaders/fill_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_layer_ubo.hpp>
#include <mbgl/shaders/heatmap_texture_layer_ubo.hpp>
#include <mbgl/shaders/hillshade_layer_ubo.hpp>
#include <mbgl/shaders/hillshade_prepare_layer_ubo.hpp>
#include <mbgl/shaders/layer_ubo.hpp>
#include <mbgl/shaders/line_layer_ubo.hpp>
#include <mbgl/shaders/raster_layer_ubo.hpp>
#include <mbgl/shaders/shader_manifest.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/shader_source.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>

#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace mbgl {
namespace uniform_metadata {

namespace {
using namespace shaders;

#define UBO_STRING(__X) #__X
#define FIELD(field_) {#field_, {offsetof(UBO_, field_), sizeof(UBO_::field_)}}

UniformsMetadata uniformsMetadata {
#define UBO_ BackgroundDrawableUBO
    {UBO_STRING(BackgroundDrawableUBO), {
        FIELD(matrix)
    }},
#undef UBO_

#define UBO_ BackgroundLayerUBO
    {UBO_STRING(BackgroundLayerUBO), {
        FIELD(color),
        FIELD(opacity),
    }},
#undef UBO_

#define UBO_ BackgroundPatternLayerUBO
    {UBO_STRING(BackgroundPatternLayerUBO), {
        FIELD(pattern_tl_a),
        FIELD(pattern_br_a),
        FIELD(pattern_tl_b),
        FIELD(pattern_br_b),
        FIELD(texsize),
        FIELD(pattern_size_a),
        FIELD(pattern_size_b),
        FIELD(pixel_coord_upper),
        FIELD(pixel_coord_lower),
        FIELD(tile_units_to_pixels),
        FIELD(scale_a),
        FIELD(scale_b),
        FIELD(mix),
        FIELD(opacity),
    }},
#undef UBO_

#define UBO_ CircleDrawableUBO
    {UBO_STRING(CircleDrawableUBO), {
        FIELD(matrix),
        FIELD(extrude_scale),
    }},
#undef UBO_

#define UBO_ CirclePaintParamsUBO
    {UBO_STRING(CirclePaintParamsUBO), {
        FIELD(camera_to_center_distance),
    }},
#undef UBO_

#define UBO_ CircleEvaluatedPropsUBO
    {UBO_STRING(CircleEvaluatedPropsUBO), {
        FIELD(color),
        FIELD(stroke_color),
        FIELD(radius),
        FIELD(blur),
        FIELD(opacity),
        FIELD(stroke_width),
        FIELD(stroke_opacity),
        FIELD(scale_with_map),
        FIELD(pitch_with_map),
    }},
#undef UBO_

#define UBO_ CircleInterpolateUBO
    {UBO_STRING(CircleInterpolateUBO), {
        FIELD(color_t),
        FIELD(radius_t),
        FIELD(blur_t),
        FIELD(opacity_t),
        FIELD(stroke_color_t),
        FIELD(stroke_width_t),
        FIELD(stroke_opacity_t),
    }},
#undef UBO_

#define UBO_ CollisionUBO
    {UBO_STRING(CollisionUBO), {
        FIELD(matrix),
        FIELD(extrude_scale),
        FIELD(overscale_factor),
    }},
#undef UBO_

#define UBO_ DebugUBO
    {UBO_STRING(DebugUBO), {
        FIELD(matrix),
        FIELD(color),
        FIELD(overlay_scale),
    }},
#undef UBO_

#define UBO_ FillExtrusionDrawableTilePropsUBO
    {UBO_STRING(FillExtrusionDrawableTilePropsUBO), {
        FIELD(pattern_from),
        FIELD(pattern_to),
    }},
#undef UBO_

#define UBO_ FillExtrusionInterpolateUBO
    {UBO_STRING(FillExtrusionInterpolateUBO), {
        FIELD(base_t),
        FIELD(height_t),
        FIELD(color_t),
        FIELD(pattern_from_t),
        FIELD(pattern_to_t),
    }},
#undef UBO_

#define UBO_ FillExtrusionDrawableUBO
    {UBO_STRING(FillExtrusionDrawableUBO), {
        FIELD(matrix),
        FIELD(scale),
        FIELD(texsize),
        FIELD(pixel_coord_upper),
        FIELD(pixel_coord_lower),
        FIELD(height_factor),
    }},
#undef UBO_

#define UBO_ FillExtrusionDrawablePropsUBO
    {UBO_STRING(FillExtrusionDrawablePropsUBO), {
        FIELD(color),
        FIELD(light_color),
        FIELD(light_position),
        FIELD(base),
        FIELD(height),
        FIELD(light_intensity),
        FIELD(vertical_gradient),
        FIELD(opacity),
        FIELD(fade),
    }},
#undef UBO_

#define UBO_ FillDrawableUBO
    {UBO_STRING(FillDrawableUBO), {
        FIELD(matrix),
    }},
#undef UBO_

#define UBO_ FillEvaluatedPropsUBO
    {UBO_STRING(FillEvaluatedPropsUBO), {
        FIELD(color),
        FIELD(opacity),
    }},
#undef UBO_

#define UBO_ FillInterpolateUBO
    {UBO_STRING(FillInterpolateUBO), {
        FIELD(color_t),
        FIELD(opacity_t),
    }},
#undef UBO_

#define UBO_ FillOutlineDrawableUBO
    {UBO_STRING(FillOutlineDrawableUBO), {
        FIELD(matrix),
        FIELD(world),
    }},
#undef UBO_

#define UBO_ FillOutlineEvaluatedPropsUBO
    {UBO_STRING(FillOutlineEvaluatedPropsUBO), {
        FIELD(outline_color),
        FIELD(opacity),
    }},
#undef UBO_

#define UBO_ FillOutlineInterpolateUBO
    {UBO_STRING(FillOutlineInterpolateUBO), {
        FIELD(outline_color_t),
        FIELD(opacity_t),
    }},
#undef UBO_

#define UBO_ FillPatternDrawableUBO
    {UBO_STRING(FillPatternDrawableUBO), {
        FIELD(matrix),
        FIELD(scale),
        FIELD(pixel_coord_upper),
        FIELD(texsize),
    }},
#undef UBO_

#define UBO_ FillPatternEvaluatedPropsUBO
    {UBO_STRING(FillPatternEvaluatedPropsUBO), {
        FIELD(opacity),
        FIELD(fade),
    }},
#undef UBO_

#define UBO_ FillPatternTilePropsUBO
    {UBO_STRING(FillPatternTilePropsUBO), {
        FIELD(pattern_from),
        FIELD(pattern_to),
    }},
#undef UBO_

#define UBO_ FillPatternInterpolateUBO
    {UBO_STRING(FillPatternInterpolateUBO), {
        FIELD(pattern_from_t),
        FIELD(pattern_to_t),
        FIELD(opacity_t),
    }},
#undef UBO_

#define UBO_ FillOutlinePatternDrawableUBO
    {UBO_STRING(FillOutlinePatternDrawableUBO), {
        FIELD(matrix),
        FIELD(scale),
        FIELD(world),
        FIELD(pixel_coord_upper),
        FIELD(pixel_coord_lower),
        FIELD(texsize),
    }},
#undef UBO_

#define UBO_ FillOutlinePatternEvaluatedPropsUBO
    {UBO_STRING(FillOutlinePatternEvaluatedPropsUBO), {
        FIELD(opacity),
        FIELD(fade),
    }},
#undef UBO_

#define UBO_ FillOutlinePatternTilePropsUBO
    {UBO_STRING(FillOutlinePatternTilePropsUBO), {
        FIELD(pattern_from),
        FIELD(pattern_to),
    }},
#undef UBO_

#define UBO_ FillOutlinePatternInterpolateUBO
    {UBO_STRING(FillOutlinePatternInterpolateUBO), {
        FIELD(pattern_from_t),
        FIELD(pattern_to_t),
        FIELD(opacity_t),
    }},
#undef UBO_

#define UBO_ HeatmapDrawableUBO
    {UBO_STRING(HeatmapDrawableUBO), {
        FIELD(matrix),
        FIELD(extrude_scale),
    }},
#undef UBO_

#define UBO_ HeatmapEvaluatedPropsUBO
    {UBO_STRING(HeatmapEvaluatedPropsUBO), {
        FIELD(weight),
        FIELD(radius),
        FIELD(intensity),
    }},
#undef UBO_

#define UBO_ HeatmapInterpolateUBO
    {UBO_STRING(HeatmapInterpolateUBO), {
        FIELD(weight_t),
        FIELD(radius_t),
    }},
#undef UBO_

#define UBO_ HeatmapTextureDrawableUBO
    {UBO_STRING(HeatmapTextureDrawableUBO), {
        FIELD(matrix),
        FIELD(world),
        FIELD(opacity),
    }},
#undef UBO_

#define UBO_ HillshadeDrawableUBO
    {UBO_STRING(HillshadeDrawableUBO), {
        FIELD(matrix),
        FIELD(latrange),
        FIELD(light),
    }},
#undef UBO_

#define UBO_ HillshadeEvaluatedPropsUBO
    {UBO_STRING(HillshadeEvaluatedPropsUBO), {
        FIELD(highlight),
        FIELD(shadow),
        FIELD(accent),
    }},
#undef UBO_

#define UBO_ HillshadePrepareDrawableUBO
    {UBO_STRING(HillshadePrepareDrawableUBO), {
        FIELD(matrix),
        FIELD(unpack),
        FIELD(dimension),
        FIELD(zoom),
        FIELD(maxzoom),
    }},
#undef UBO_

#define UBO_ LineUBO
    {UBO_STRING(LineUBO), {
        FIELD(matrix),
        FIELD(ratio),
    }},
#undef UBO_

#define UBO_ LineGradientUBO
    {UBO_STRING(LineGradientUBO), {
        FIELD(matrix),
        FIELD(ratio),
    }},
#undef UBO_

#define UBO_ LinePropertiesUBO
    {UBO_STRING(LinePropertiesUBO), {
        FIELD(color),
        FIELD(blur),
        FIELD(opacity),
        FIELD(gapwidth),
        FIELD(offset),
        FIELD(width),
    }},
#undef UBO_

#define UBO_ LineGradientPropertiesUBO
    {UBO_STRING(LineGradientPropertiesUBO), {
        FIELD(blur),
        FIELD(opacity),
        FIELD(gapwidth),
        FIELD(offset),
        FIELD(width),
    }},
#undef UBO_

#define UBO_ LinePatternUBO
    {UBO_STRING(LinePatternUBO), {
        FIELD(matrix),
        FIELD(scale),
        FIELD(texsize),
        FIELD(ratio),
        FIELD(fade),
    }},
#undef UBO_

#define UBO_ LinePatternPropertiesUBO
    {UBO_STRING(LinePatternPropertiesUBO), {
        FIELD(blur),
        FIELD(opacity),
        FIELD(offset),
        FIELD(gapwidth),
        FIELD(width),
    }},
#undef UBO_

#define UBO_ LineSDFUBO
    {UBO_STRING(LineSDFUBO), {
        FIELD(matrix),
        FIELD(patternscale_a),
        FIELD(patternscale_b),
        FIELD(ratio),
        FIELD(tex_y_a),
        FIELD(tex_y_b),
        FIELD(sdfgamma),
        FIELD(mix),
    }},
#undef UBO_

#define UBO_ LineSDFPropertiesUBO
    {UBO_STRING(LineSDFPropertiesUBO), {
        FIELD(color),
        FIELD(blur),
        FIELD(opacity),
        FIELD(gapwidth),
        FIELD(offset),
        FIELD(width),
        FIELD(floorwidth),
    }},
#undef UBO_

#define UBO_ LineBasicUBO
    {UBO_STRING(LineBasicUBO), {
        FIELD(matrix),
        FIELD(units_to_pixels),
        FIELD(ratio),
    }},
#undef UBO_

#define UBO_ LineBasicPropertiesUBO
    {UBO_STRING(LineBasicPropertiesUBO), {
        FIELD(color),
        FIELD(opacity),
        FIELD(width),
    }},
#undef UBO_

#define UBO_ LineInterpolationUBO
    {UBO_STRING(LineInterpolationUBO), {
        FIELD(color_t),
        FIELD(blur_t),
        FIELD(opacity_t),
        FIELD(gapwidth_t),
        FIELD(offset_t),
        FIELD(width_t),
    }},
#undef UBO_

#define UBO_ LineGradientInterpolationUBO
    {UBO_STRING(LineGradientInterpolationUBO), {
        FIELD(blur_t),
        FIELD(opacity_t),
        FIELD(gapwidth_t),
        FIELD(offset_t),
        FIELD(width_t),
    }},
#undef UBO_

#define UBO_ LinePatternInterpolationUBO
    {UBO_STRING(LinePatternInterpolationUBO), {
        FIELD(blur_t),
        FIELD(opacity_t),
        FIELD(offset_t),
        FIELD(gapwidth_t),
        FIELD(width_t),
        FIELD(pattern_from_t),
        FIELD(pattern_to_t),
    }},
#undef UBO_

#define UBO_ LineSDFInterpolationUBO
    {UBO_STRING(LineSDFInterpolationUBO), {
        FIELD(color_t),
        FIELD(blur_t),
        FIELD(opacity_t),
        FIELD(gapwidth_t),
        FIELD(offset_t),
        FIELD(width_t),
        FIELD(floorwidth_t),
    }},
#undef UBO_

#define UBO_ LinePatternTilePropertiesUBO
    {UBO_STRING(LinePatternTilePropertiesUBO), {
        FIELD(pattern_from),
        FIELD(pattern_to),
    }},
#undef UBO_

#define UBO_ RasterDrawableUBO
    {UBO_STRING(RasterDrawableUBO), {
        FIELD(matrix),
        FIELD(spin_weights),
        FIELD(tl_parent),
        FIELD(scale_parent),
        FIELD(buffer_scale),
        FIELD(fade_t),
        FIELD(opacity),
        FIELD(brightness_low),
        FIELD(brightness_high),
        FIELD(saturation_factor),
        FIELD(contrast_factor),
    }},
#undef UBO_

#define UBO_ SymbolDrawableTilePropsUBO
    {UBO_STRING(SymbolDrawableTilePropsUBO), {
        FIELD(is_text),
        FIELD(is_halo),
        FIELD(pitch_with_map),
        FIELD(is_size_zoom_constant),
        FIELD(is_size_feature_constant),
        FIELD(size_t),
        FIELD(size),
    }},
#undef UBO_

#define UBO_ SymbolDrawableInterpolateUBO
    {UBO_STRING(SymbolDrawableInterpolateUBO), {
        FIELD(fill_color_t),
        FIELD(halo_color_t),
        FIELD(opacity_t),
        FIELD(halo_width_t),
        FIELD(halo_blur_t),
    }},
#undef UBO_

#define UBO_ SymbolDrawableUBO
    {UBO_STRING(SymbolDrawableUBO), {
        FIELD(matrix),
        FIELD(label_plane_matrix),
        FIELD(coord_matrix),
        FIELD(texsize),
        FIELD(texsize_icon),
        FIELD(gamma_scale),
        FIELD(rotate_symbol),
    }},
#undef UBO_

#define UBO_ SymbolDynamicUBO
    {UBO_STRING(SymbolDynamicUBO), {
        FIELD(fade_change),
        FIELD(camera_to_center_distance),
        FIELD(aspect_ratio),
    }},
#undef UBO_

#define UBO_ SymbolDrawablePaintUBO
    {UBO_STRING(SymbolDrawablePaintUBO), {
        FIELD(fill_color),
        FIELD(halo_color),
        FIELD(opacity),
        FIELD(halo_width),
        FIELD(halo_blur),
    }},
#undef UBO_

};
#undef FIELD
#undef UBO_STRING
} // namespace

void getChangedUBOFields(const std::string& name, std::size_t size, const uint8_t* before, const uint8_t* after,
                                             /*out*/ std::vector<std::string>& fieldNames, /*out*/ std::size_t& updatedSize) {
    if(auto iUBO = uniformsMetadata.find(name); iUBO != uniformsMetadata.end()) {
        for(const auto& ifield : iUBO->second) {
            if(ifield.second.offset + ifield.second.size <= size) {
                // compare
                if(0 != std::memcmp(before + ifield.second.offset, after + ifield.second.offset, ifield.second.size)) {
                    fieldNames.emplace_back(ifield.first);
                    updatedSize += ifield.second.size;
                }
            }
            else {
                // the field is out of bounds
                Log::Error(Event::General, "UBO field out of bounds: " + name + "." + ifield.first);
                assert(false);
            }
        }
    }
    else {
        Log::Error(Event::General, "UBO not found: " + name);
    }
}

void getUBOFields(const std::string& name, std::size_t size,
                  /*out*/ std::vector<std::string>& fieldNames, /*out*/ std::size_t& dataSize) {
    if(auto iUBO = uniformsMetadata.find(name); iUBO != uniformsMetadata.end()) {
        for(const auto& ifield : iUBO->second) {
            if(ifield.second.offset + ifield.second.size <= size) {
                fieldNames.emplace_back(ifield.first);
                dataSize += ifield.second.size;
            }
            else {
                // the field is out of bounds
                Log::Error(Event::General, "UBO field out of bounds: " + name + "." + ifield.first);
                assert(false);
            }
        }
    }
    else {
        Log::Error(Event::General, "UBO not found: " + name);
    }
}

} // namespace uniform_metadata
} // namespace mbgl

//#endif // !NDEBUG
#endif // MLN_DRAWABLE_RENDERER
