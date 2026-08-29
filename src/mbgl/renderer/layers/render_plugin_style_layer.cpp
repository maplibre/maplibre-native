#include <mbgl/renderer/layers/render_plugin_style_layer.hpp>

#include <mbgl/gfx/context.hpp>
#include <mbgl/gfx/color_mode.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/gfx/vertex_attribute.hpp>
#include <mbgl/plugin/plugin_shader.hpp>
#include <mbgl/renderer/buckets/plugin_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/plugin_layer_tweaker.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/shaders/shader_program_base.hpp>

namespace mbgl {
namespace {

const style::PluginStyleLayer::Impl& pluginImpl(const Immutable<style::Layer::Impl>& impl) {
    return static_cast<const style::PluginStyleLayer::Impl&>(*impl);
}

RenderPass renderPassFor(mln_plugin_render_stage stage) {
    switch (stage) {
        case MLN_PLUGIN_RENDER_STAGE_PASS_3D:
            return RenderPass::Pass3D;
        case MLN_PLUGIN_RENDER_STAGE_OPAQUE:
            return RenderPass::Opaque;
        case MLN_PLUGIN_RENDER_STAGE_TRANSLUCENT:
            return RenderPass::Translucent;
        default:
            return RenderPass::None;
    }
}

gfx::AttributeDataType attributeType(mln_plugin_vertex_attribute_type type) {
    switch (type) {
        case MLN_PLUGIN_VERTEX_INT16:
            return gfx::AttributeDataType::Short;
        case MLN_PLUGIN_VERTEX_INT16_X2:
            return gfx::AttributeDataType::Short2;
        case MLN_PLUGIN_VERTEX_UINT16:
            return gfx::AttributeDataType::UShort;
        case MLN_PLUGIN_VERTEX_UINT16_X2:
            return gfx::AttributeDataType::UShort2;
        case MLN_PLUGIN_VERTEX_FLOAT:
            return gfx::AttributeDataType::Float;
        case MLN_PLUGIN_VERTEX_FLOAT_X2:
            return gfx::AttributeDataType::Float2;
        case MLN_PLUGIN_VERTEX_FLOAT_X3:
            return gfx::AttributeDataType::Float3;
        case MLN_PLUGIN_VERTEX_FLOAT_X4:
            return gfx::AttributeDataType::Float4;
        case MLN_PLUGIN_VERTEX_UINT8_X4_NORMALIZED:
            return gfx::AttributeDataType::UByte4;
    }
    return gfx::AttributeDataType::Invalid;
}

gfx::ColorMode colorMode(mln_plugin_blend_mode mode) {
    switch (mode) {
        case MLN_PLUGIN_BLEND_REPLACE:
            return gfx::ColorMode::unblended();
        case MLN_PLUGIN_BLEND_MULTIPLY:
            return {.blendFunction = gfx::ColorMode::Add{gfx::ColorBlendFactorType::DstColor,
                                                         gfx::ColorBlendFactorType::Zero},
                    .blendColor = {},
                    .mask = {.r = true, .g = true, .b = true, .a = true}};
        case MLN_PLUGIN_BLEND_ALPHA:
        case MLN_PLUGIN_BLEND_PREMULTIPLIED_ALPHA:
            return gfx::ColorMode::alphaBlended();
    }
    return gfx::ColorMode::alphaBlended();
}

mln_plugin_geometry_type geometryType(FeatureType type) {
    switch (type) {
        case FeatureType::Point:
            return MLN_PLUGIN_GEOMETRY_POINT;
        case FeatureType::LineString:
            return MLN_PLUGIN_GEOMETRY_LINESTRING;
        case FeatureType::Polygon:
            return MLN_PLUGIN_GEOMETRY_POLYGON;
        case FeatureType::Unknown:
            return static_cast<mln_plugin_geometry_type>(0);
    }
    return static_cast<mln_plugin_geometry_type>(0);
}

} // namespace

RenderPluginStyleLayer::RenderPluginStyleLayer(Immutable<style::PluginStyleLayer::Impl> impl)
    : RenderLayer(makeMutable<style::PluginStyleLayerProperties>(std::move(impl))) {
    // Unlike generated layers, a plugin layer has no generated evaluated
    // property object that initializes its pass. It must be renderable from
    // its first frame, including styles installed before the initial zoom
    // evaluation.
    passes = renderPassFor(pluginImpl(baseImpl).registration.renderStage);
}

void RenderPluginStyleLayer::evaluate(const PropertyEvaluationParameters&) {
    passes = renderPassFor(pluginImpl(baseImpl).registration.renderStage);
}

bool RenderPluginStyleLayer::is3D() const {
    return pluginImpl(baseImpl).registration.requires3D;
}

void RenderPluginStyleLayer::update(gfx::ShaderRegistry& shaders,
                                    gfx::Context& context,
                                    const TransformState&,
                                    const std::shared_ptr<UpdateParameters>&,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    const auto& registration = pluginImpl(baseImpl).registration;
    if (!registration.usesHostDrawables()) {
        if (!layerGroup) {
            if (auto group = context.createLayerGroup(layerIndex, 0, getID())) {
                setLayerGroup(std::move(group), changes);
            }
        }
        return;
    }

    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }
    if (!layerGroup) {
        if (auto group = context.createTileLayerGroup(layerIndex, 64, getID())) {
            setLayerGroup(std::move(group), changes);
        } else {
            return;
        }
    }
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    if (!layerTweaker) {
        layerTweaker = std::make_shared<PluginLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    const auto renderPass = renderPassFor(registration.renderStage);
    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();
        const auto* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData || !renderData->bucket || !renderData->bucket->hasData()) {
            removeTile(renderPass, tileID);
            continue;
        }
        auto& bucket = static_cast<PluginBucket&>(*renderData->bucket);
        const auto previousBucket = getRenderTileBucketID(tileID);
        if (previousBucket != util::SimpleIdentity::Empty && previousBucket != bucket.getID()) {
            removeTile(renderPass, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());
        if (updateTile(renderPass, tileID, [&](gfx::Drawable& drawable) {
                return drawable.getLayerTweaker() == layerTweaker;
            })) {
            continue;
        }

        for (const auto& definition : bucket.drawables) {
            const auto groupName = plugin::shaderGroupName(registration.pluginID, definition.shaderID);
            const auto shaderGroup = shaders.getShaderGroup(groupName);
            const auto shader = shaderGroup ? shaderGroup->getOrCreateShader(context, {}) : gfx::ShaderPtr{};
            if (!shader || definition.segments.empty()) {
                continue;
            }

            auto attributes = context.createVertexAttributeArray();
            std::size_t vertexCount = 0;
            gfx::AttributeDataType firstType = gfx::AttributeDataType::Invalid;
            for (const auto& binding : definition.attributes) {
                const auto stream = bucket.vertexStreams.find(binding.streamID);
                if (stream == bucket.vertexStreams.end()) continue;
                const auto type = attributeType(binding.type);
                if (const auto& attr = attributes->set(binding.attributeID)) {
                    attr->setSharedRawData(stream->second,
                                           binding.byteOffset,
                                           0,
                                           stream->second->getRawSize(),
                                           type);
                }
                vertexCount = std::max(vertexCount, stream->second->getRawCount());
                if (firstType == gfx::AttributeDataType::Invalid) firstType = type;
            }
            if (!vertexCount || firstType == gfx::AttributeDataType::Invalid) continue;

            auto builder = context.createDrawableBuilder("plugin/" + registration.type);
            builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
            builder->setRenderPass(renderPass);
            builder->setEnableDepth(definition.depthMode != MLN_PLUGIN_DEPTH_DISABLED);
            builder->setDepthType(definition.depthMode == MLN_PLUGIN_DEPTH_READ_WRITE
                                      ? gfx::DepthMaskType::ReadWrite
                                      : gfx::DepthMaskType::ReadOnly);
            builder->setColorMode(colorMode(definition.blendMode));
            builder->setCullFaceMode(definition.enableCullFace ? gfx::CullFaceMode::backCCW()
                                                               : gfx::CullFaceMode::disabled());
            builder->setEnableStencil(definition.enableStencil);
            builder->setVertexAttributes(std::move(attributes));
            builder->setRawVertices({}, vertexCount, firstType);
            switch (definition.drawMode) {
                case MLN_PLUGIN_DRAW_MODE_TRIANGLES:
                    builder->setSegments(
                        gfx::Triangles(), bucket.indices, definition.segments.data(), definition.segments.size());
                    break;
                case MLN_PLUGIN_DRAW_MODE_LINES:
                    builder->setSegments(
                        gfx::Lines(1.0f), bucket.indices, definition.segments.data(), definition.segments.size());
                    break;
                case MLN_PLUGIN_DRAW_MODE_POINTS:
                    builder->setSegments(
                        gfx::Points(1.0f), bucket.indices, definition.segments.data(), definition.segments.size());
                    break;
            }
            builder->flush(context);
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                drawable->setRenderTile(renderTilesOwner, &tile);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        }
    }
}

bool RenderPluginStyleLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                                    const GeometryTileFeature& feature,
                                                    float zoom,
                                                    const TransformState&,
                                                    float pixelsToTileUnits,
                                                    const mat4&,
                                                    const FeatureState& featureState) const {
    const auto& registration = pluginImpl(baseImpl).registration;
    if (!registration.queryFeature) return false;
    const auto& geometry = feature.getGeometries();
    std::vector<mln_plugin_tile_point_v1> points;
    std::vector<uint32_t> offsets;
    offsets.push_back(0);
    for (const auto& path : geometry) {
        for (const auto& point : path) points.push_back({point.x, point.y});
        offsets.push_back(static_cast<uint32_t>(points.size()));
    }
    std::vector<mln_plugin_tile_point_v1> query;
    query.reserve(queryGeometry.size());
    for (const auto& point : queryGeometry) query.push_back({point.x, point.y});
    const auto id = featureIDtoString(feature.getID()).value_or(std::string{});
    mln_plugin_feature_v1 pluginFeature{};
    pluginFeature.struct_size = sizeof(pluginFeature);
    pluginFeature.geometry_type = geometryType(feature.getType());
    pluginFeature.feature_id = {id.data(), id.size()};
    pluginFeature.points = points.data();
    pluginFeature.point_count = points.size();
    pluginFeature.path_offsets = offsets.data();
    pluginFeature.path_count = geometry.size();
    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    const auto& impl = pluginImpl(baseImpl);
    std::vector<mln_plugin_property_value_v1> properties;
    std::vector<std::string> strings(definitions.size());
    properties.reserve(definitions.size());
    for (size_t i = 0; i < definitions.size(); ++i) {
        const auto& definition = definitions[i];
        const auto propertyIt = impl.pluginProperties.find(definition.name);
        const auto value = propertyIt == impl.pluginProperties.end()
                               ? style::defaultPluginPropertyValue(definition)
                               : propertyIt->second;
        mln_plugin_property_value_v1 property{};
        property.struct_size = sizeof(property);
        property.name = {definition.name.data(), definition.name.size()};
        property.value = value.evaluate(zoom, feature, featureState, definition, strings[i]);
        property.explicitly_set = propertyIt != impl.pluginProperties.end();
        properties.push_back(property);
    }
    pluginFeature.evaluated_properties = properties.data();
    pluginFeature.evaluated_property_count = properties.size();
    return registration.queryFeature(
               &pluginFeature, query.data(), query.size(), pixelsToTileUnits, properties.data(), properties.size()) != 0;
}

} // namespace mbgl
