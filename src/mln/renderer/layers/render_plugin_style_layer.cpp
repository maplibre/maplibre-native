#include <mln/renderer/layers/render_plugin_style_layer.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/color_mode.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/plugin/plugin_drawable_data.hpp>
#include <mln/plugin/plugin_conversion.hpp>
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
#include <mln/renderer/paint_parameters.hpp>
#include <mln/shaders/program_parameters.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/style/plugin_property.hpp>

#include <set>

namespace mln {
namespace {

const style::PluginStyleLayer::Impl& pluginImpl(const Immutable<style::Layer::Impl>& impl) {
    return static_cast<const style::PluginStyleLayer::Impl&>(*impl);
}

} // namespace

RenderPluginStyleLayer::RenderPluginStyleLayer(Immutable<style::PluginStyleLayer::Impl> impl)
    : RenderLayer(makeMutable<style::PluginStyleLayerProperties>(std::move(impl))) {
    // Unlike generated layers, a plugin layer has no generated evaluated
    // property object that initializes its pass. It must be renderable from
    // its first frame, including styles installed before the initial zoom
    // evaluation.
    const auto& registration = pluginImpl(baseImpl).registration;
    const auto& definitions = registration->properties;
    const auto& layerImpl = pluginImpl(baseImpl);
    for (const auto& definition : definitions) {
        const auto property = layerImpl.pluginProperties.find(definition.name);
        auto value = property == layerImpl.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                                  : property->second;
        transitioningPaintProperties.emplace(definition.name, style::PluginTransitioningPropertyValue{value});
        evaluatedPluginProperties.emplace(definition.name, std::move(value));
    }
    evaluatedPaintSnapshot = std::make_shared<const style::PluginPropertyMap>(evaluatedPluginProperties);
    passes = RenderPass::Translucent;
}

void RenderPluginStyleLayer::transition(const TransitionParameters& parameters) {
    const auto& impl = pluginImpl(baseImpl);
    const auto& definitions = impl.registration->properties;
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
    const auto& definitions = registration->properties;
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
    if (*evaluatedPaintSnapshot != evaluatedPluginProperties) {
        evaluatedPaintSnapshot = std::make_shared<const style::PluginPropertyMap>(evaluatedPluginProperties);
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
                                    const PaintParameters& parameters,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    const auto& registration = pluginImpl(baseImpl).registration;
    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        return;
    }
    // Registration may happen after this renderer initialized its shader registry.
    // Resolve this layer's immutable shader definitions once on the render
    // thread. As with built-in render layers, the retained groups have the
    // renderer's lifetime; paint updates do not invalidate shader definitions.
    if (shaderGroups.empty()) {
        plugin::registerPluginShaderGroups(shaders, ProgramParameters{parameters.pixelRatio, false}, registration);
        for (const auto& definition : registration->shaders) {
            shaderGroups.emplace(definition.id,
                                 shaders.getShaderGroup(plugin::shaderGroupName(
                                     registration->pluginID, registration->type, definition.id)));
        }
    }
    if (!layerGroup) {
        if (auto group = context.createTileLayerGroup(layerIndex, 64, getID())) {
            setLayerGroup(std::move(group), changes);
        } else {
            return;
        }
    }
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    // These drawables disable stencil; generating tile masks adds an unused pass.
    if (!layerTweaker) {
        layerTweaker = std::make_shared<PluginLayerTweaker>(getID(), evaluatedProperties, registration);
        layerGroup->addLayerTweaker(layerTweaker);
    }
    if (registration->enableStencilOverlapDedup) {
        // Every is3D + stencil-enabled drawable below shares this group's single stencil ref.
        // See the enable_stencil_overlap_dedup doc comment in plugin_api.h.
        tileLayerGroup->setStencilTiles(renderTiles);
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
        if (bucket.synchronizePaint(getID(), evaluatedPaintSnapshot, static_cast<float>(state.getZoom()))) {
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
            const auto shaderGroup = shaderGroups.at(definition.shaderID);
            StringIDSetsPair propertiesAsUniforms;
            auto* paintBinders = bucket.paintBinders(getID(), definition.key);
            auto attributes = context.createVertexAttributeArray();
            if (paintBinders) paintBinders->populateVertexAttributes(*attributes, propertiesAsUniforms);
            const auto shader = shaderGroup ? shaderGroup->getOrCreateShader(
                                                  context, propertiesAsUniforms, gfx::ProjectionVariant::Mercator)
                                            : gfx::ShaderPtr{};
            if (!shader || definition.segments.empty()) {
                continue;
            }

            std::size_t vertexCount = 0;
            gfx::AttributeDataType firstType = gfx::AttributeDataType::Invalid;
            for (const auto& binding : definition.attributes) {
                const auto stream = bucket.vertexStreams.find(binding.streamID);
                if (stream == bucket.vertexStreams.end()) continue;
                const auto type = binding.type;
                if (const auto& attr = attributes->set(binding.attributeID)) {
                    attr->setSharedRawData(stream->second, binding.byteOffset, 0, stream->second->getRawSize(), type);
                }
                vertexCount = std::max(vertexCount, stream->second->getRawCount());
                if (firstType == gfx::AttributeDataType::Invalid) firstType = type;
            }
            if (!vertexCount || firstType == gfx::AttributeDataType::Invalid) continue;

            auto builder = context.createDrawableBuilder("plugin/" + registration->type);
            builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
            builder->setRenderPass(renderPass);
            if (registration->enableStencilOverlapDedup) {
                // No depth test: visibility relative to other layers comes entirely from style layer order.
                builder->setEnableDepth(false);
                builder->setIs3D(true);
                builder->setEnableStencil(true);
            } else {
                builder->setEnableDepth(true);
                builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                builder->setIs3D(false);
                builder->setEnableStencil(false);
            }
            builder->setColorMode(gfx::ColorMode::alphaBlended());
            builder->setCullFaceMode(gfx::CullFaceMode::disabled());
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
    if (!registration->queryFeature) return false;
    const plugin::FeatureView pluginFeature(feature);
    std::vector<mln_plugin_tile_point_v1> query;
    query.reserve(queryGeometry.size());
    for (const auto& point : queryGeometry) query.push_back({point.x, point.y});
    const auto& definitions = registration->properties;
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
    return registration->queryFeature(
               &pluginFeature.value, query.data(), query.size(), &queryContext, properties.data(), properties.size()) !=
           0;
}

} // namespace mln
