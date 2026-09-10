#include <mln/renderer/layers/render_plugin_style_layer.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/color_mode.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/plugin/plugin_drawable_data.hpp>
#include <mln/gfx/shader_group.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/gfx/vertex_attribute.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/plugin/plugin_shader.hpp>
#include <mln/renderer/change_request.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layers/plugin_layer_tweaker.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_source.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/style/plugin_property.hpp>

#include <set>

namespace mln {
namespace {

const style::PluginStyleLayer::Impl& pluginImpl(const Immutable<style::Layer::Impl>& impl) {
    return static_cast<const style::PluginStyleLayer::Impl&>(*impl);
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
    const auto& registration = pluginImpl(baseImpl).registration;
    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    const auto& layerImpl = pluginImpl(baseImpl);
    for (const auto& definition : definitions) {
        const auto property = layerImpl.pluginProperties.find(definition.name);
        auto value = property == layerImpl.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                                  : property->second;
        transitioningPaintProperties.emplace(definition.name, style::PluginTransitioningPropertyValue{value});
        evaluatedPluginProperties.emplace(definition.name, std::move(value));
    }
    passes = RenderPass::Translucent;
}

void RenderPluginStyleLayer::transition(const TransitionParameters& parameters) {
    const auto& impl = pluginImpl(baseImpl);
    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(impl.registration.type);
    for (const auto& definition : definitions) {
        const auto property = impl.pluginProperties.find(definition.name);
        auto value = property == impl.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                             : property->second;
        auto prior = transitioningPaintProperties.find(definition.name);
        auto priorValue = prior == transitioningPaintProperties.end()
                              ? style::PluginTransitioningPropertyValue{style::defaultPluginPropertyValue(definition)}
                              : std::move(prior->second);
        const auto options = impl.pluginPropertyTransitions.find(definition.name);
        const auto transition = options == impl.pluginPropertyTransitions.end()
                                    ? parameters.transition
                                    : options->second.reverseMerge(parameters.transition);
        transitioningPaintProperties.insert_or_assign(
            definition.name,
            style::PluginTransitioningPropertyValue{
                std::move(value), std::move(priorValue), transition, parameters.now});
    }
}

void RenderPluginStyleLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto& registration = pluginImpl(baseImpl).registration;
    passes = RenderPass::Translucent;

    evaluatedPluginProperties.clear();
    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    const auto& impl = pluginImpl(baseImpl);
    for (const auto& definition : definitions) {
        const auto transition = transitioningPaintProperties.find(definition.name);
        if (transition != transitioningPaintProperties.end()) {
            evaluatedPluginProperties.emplace(definition.name,
                                              transition->second.evaluate(parameters.z, definition, parameters.now));
        } else {
            const auto property = impl.pluginProperties.find(definition.name);
            evaluatedPluginProperties.emplace(definition.name,
                                              property == impl.pluginProperties.end()
                                                  ? style::defaultPluginPropertyValue(definition)
                                                  : property->second);
        }
    }
    auto properties = makeMutable<style::PluginStyleLayerProperties>(
        staticImmutableCast<style::PluginStyleLayer::Impl>(baseImpl), evaluatedPluginProperties);
    properties->renderPasses = underlying_type(passes);
    evaluatedProperties = std::move(properties);
    if (layerTweaker) layerTweaker->updateProperties(evaluatedProperties);
}

bool RenderPluginStyleLayer::hasTransition() const {
    return std::any_of(transitioningPaintProperties.begin(),
                       transitioningPaintProperties.end(),
                       [](const auto& property) { return property.second.hasTransition(); });
}

void RenderPluginStyleLayer::layerChanged(const TransitionParameters&,
                                          const Immutable<style::Layer::Impl>&,
                                          UniqueChangeRequestVec&) {
    // Plugin tweakers are registration-specific, not bucket-specific. Keeping
    // the existing tweaker lets retained drawables receive the newly evaluated
    // property snapshot. Bucket/layout revisions independently replace any
    // drawable whose geometry actually changed.
}

void RenderPluginStyleLayer::update(gfx::ShaderRegistry& shaders,
                                    gfx::Context& context,
                                    const TransformState& state,
                                    const std::shared_ptr<UpdateParameters>&,
                                    const PaintParameters&,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    const auto& registration = pluginImpl(baseImpl).registration;
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
    tileLayerGroup->setStencilTiles(renderTiles);
    if (!layerTweaker) {
        layerTweaker = std::make_shared<PluginLayerTweaker>(getID(), evaluatedProperties, registration);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    const auto renderPass = RenderPass::Translucent;
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
        if (bucket.synchronizePaint(getID(), evaluatedPluginProperties, static_cast<float>(state.getZoom()))) {
            removeTile(renderPass, tileID);
        }
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
            StringIDSetsPair propertiesAsUniforms;
            auto* paintBinders = bucket.paintBinders(getID(), definition.key);
            auto attributes = context.createVertexAttributeArray();
            if (paintBinders) paintBinders->populateVertexAttributes(*attributes, propertiesAsUniforms);
            const auto shader = shaderGroup ? shaderGroup->getOrCreateShader(context, propertiesAsUniforms)
                                            : gfx::ShaderPtr{};
            if (!shader || definition.segments.empty()) {
                continue;
            }

            std::size_t vertexCount = 0;
            gfx::AttributeDataType firstType = gfx::AttributeDataType::Invalid;
            for (const auto& binding : definition.attributes) {
                const auto stream = bucket.vertexStreams.find(binding.streamID);
                if (stream == bucket.vertexStreams.end()) continue;
                const auto type = attributeType(binding.type);
                if (const auto& attr = attributes->set(binding.attributeID)) {
                    attr->setSharedRawData(stream->second, binding.byteOffset, 0, stream->second->getRawSize(), type);
                }
                vertexCount = std::max(vertexCount, stream->second->getRawCount());
                if (firstType == gfx::AttributeDataType::Invalid) firstType = type;
            }
            if (!vertexCount || firstType == gfx::AttributeDataType::Invalid) continue;

            auto builder = context.createDrawableBuilder("plugin/" + registration.type);
            builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
            builder->setRenderPass(renderPass);
            builder->setEnableDepth(true);
            builder->setDepthType(gfx::DepthMaskType::ReadOnly);
            builder->setIs3D(false);
            builder->setColorMode(gfx::ColorMode::alphaBlended());
            builder->setCullFaceMode(gfx::CullFaceMode::disabled());
            builder->setEnableStencil(false);
            builder->setVertexAttributes(std::move(attributes));
            builder->setRawVertices({}, vertexCount, firstType);
            builder->setSegments(
                gfx::Triangles(), bucket.indices, definition.segments.data(), definition.segments.size());
            builder->flush(context);
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                if (paintBinders) drawable->setBinders(renderData->bucket, paintBinders);
                drawable->setData(std::make_unique<plugin::DrawableData>(definition.shaderID));
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
                                                    const TransformState& transformState,
                                                    float pixelsToTileUnits,
                                                    const mat4& tileMatrix,
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
    pluginFeature.points = points.data();
    pluginFeature.point_count = points.size();
    pluginFeature.path_offsets = offsets.data();
    pluginFeature.path_count = geometry.size();
    const auto definitions = plugin::PluginRegistry::get().propertiesForLayer(registration.type);
    const auto& impl = pluginImpl(baseImpl);
    std::vector<mln_plugin_property_value_v1> properties;
    std::vector<style::PluginPropertyValue::EvaluationStorage> storage(definitions.size());
    properties.reserve(definitions.size());
    for (size_t i = 0; i < definitions.size(); ++i) {
        const auto& definition = definitions[i];
        const auto propertyIt = evaluatedPluginProperties.find(definition.name);
        const auto value = propertyIt == evaluatedPluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                                         : propertyIt->second;
        mln_plugin_property_value_v1 property{};
        property.struct_size = sizeof(property);
        property.name = {definition.name.data(), definition.name.size()};
        property.value = value.evaluate(zoom, feature, featureState, definition, storage[i]);
        property.explicitly_set = impl.pluginProperties.find(definition.name) != impl.pluginProperties.end();
        properties.push_back(property);
    }
    mln_plugin_query_context_v1 queryContext{};
    queryContext.struct_size = sizeof(queryContext);
    queryContext.pixels_to_tile_units = pixelsToTileUnits;
    queryContext.camera_to_center_distance = transformState.getCameraToCenterDistance();
    queryContext.bearing = transformState.getBearing();
    std::copy(tileMatrix.begin(), tileMatrix.end(), queryContext.tile_matrix);
    queryContext.viewport_width = transformState.getSize().width;
    queryContext.viewport_height = transformState.getSize().height;
    return registration.queryFeature(
               &pluginFeature, query.data(), query.size(), &queryContext, properties.data(), properties.size()) != 0;
}

} // namespace mln
