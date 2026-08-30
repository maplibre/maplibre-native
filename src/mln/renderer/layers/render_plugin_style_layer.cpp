#include <mln/renderer/layers/render_plugin_style_layer.hpp>

#include <mln/gfx/context.hpp>
#include <mln/gfx/color_mode.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/gfx/offscreen_texture.hpp>
#include <mln/gfx/plugin_render_graph_drawable_data.hpp>
#include <mln/gfx/shader_group.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/gfx/texture2d.hpp>
#include <mln/gfx/vertex_attribute.hpp>
#include <mln/plugin/plugin_shader.hpp>
#include <mln/renderer/change_request.hpp>
#include <mln/renderer/buckets/plugin_bucket.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layers/plugin_layer_tweaker.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/renderer/render_target.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_source.hpp>
#include <mln/shaders/shader_program_base.hpp>

#include <set>

namespace mln {
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

gfx::TextureFilterType textureFilter(mln_plugin_texture_filter filter) {
    return filter == MLN_PLUGIN_TEXTURE_FILTER_NEAREST ? gfx::TextureFilterType::Nearest
                                                       : gfx::TextureFilterType::Linear;
}

gfx::TextureWrapType textureWrap(mln_plugin_texture_wrap wrap) {
    return wrap == MLN_PLUGIN_TEXTURE_WRAP_REPEAT ? gfx::TextureWrapType::Repeat : gfx::TextureWrapType::Clamp;
}

const plugin::ShaderDefinition* findShader(const plugin::LayerType& registration, const std::string& id) {
    const auto it = std::find_if(
        registration.shaders.begin(), registration.shaders.end(), [&](const auto& shader) { return shader.id == id; });
    return it == registration.shaders.end() ? nullptr : &*it;
}

const plugin::ShaderTextureDefinition* findTexture(const plugin::ShaderDefinition& shader, uint32_t id) {
    const auto it = std::find_if(
        shader.textures.begin(), shader.textures.end(), [&](const auto& texture) { return texture.id == id; });
    return it == shader.textures.end() ? nullptr : &*it;
}

mln_plugin_raster_dem_encoding demEncoding(Tileset::RasterEncoding encoding) {
    return encoding == Tileset::RasterEncoding::Terrarium ? MLN_PLUGIN_RASTER_DEM_TERRARIUM
                                                          : MLN_PLUGIN_RASTER_DEM_MAPBOX;
}

void activateRenderTarget(const RenderTargetPtr& renderTarget, bool activate, UniqueChangeRequestVec& changes) {
    if (!renderTarget) return;
    if (activate) {
        changes.emplace_back(std::make_unique<AddRenderTargetRequest>(renderTarget));
    } else {
        changes.emplace_back(std::make_unique<RemoveRenderTargetRequest>(renderTarget));
    }
}

} // namespace

RenderPluginStyleLayer::RenderPluginStyleLayer(Immutable<style::PluginStyleLayer::Impl> impl)
    : RenderLayer(makeMutable<style::PluginStyleLayerProperties>(std::move(impl))) {
    // Unlike generated layers, a plugin layer has no generated evaluated
    // property object that initializes its pass. It must be renderable from
    // its first frame, including styles installed before the initial zoom
    // evaluation.
    const auto& registration = pluginImpl(baseImpl).registration;
    passes = renderPassFor(registration.renderStage);
    if (registration.participatesIn3DPass) passes |= RenderPass::Pass3D;
}

void RenderPluginStyleLayer::evaluate(const PropertyEvaluationParameters&) {
    const auto& registration = pluginImpl(baseImpl).registration;
    passes = renderPassFor(registration.renderStage);
    if (registration.participatesIn3DPass) passes |= RenderPass::Pass3D;

    auto properties = makeMutable<style::PluginStyleLayerProperties>(
        staticImmutableCast<style::PluginStyleLayer::Impl>(baseImpl));
    properties->renderPasses = underlying_type(passes);
    evaluatedProperties = std::move(properties);
    if (layerTweaker) layerTweaker->updateProperties(evaluatedProperties);
}

void RenderPluginStyleLayer::layerChanged(const TransitionParameters&,
                                          const Immutable<style::Layer::Impl>&,
                                          UniqueChangeRequestVec&) {
    // Plugin tweakers are registration-specific, not bucket-specific. Keeping
    // the existing tweaker lets retained drawables receive the newly evaluated
    // property snapshot. Bucket/layout revisions independently replace any
    // drawable whose geometry actually changed.
}

bool RenderPluginStyleLayer::is3D() const {
    return pluginImpl(baseImpl).registration.requires3D;
}

void RenderPluginStyleLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    sourceMaxZoom = params.source->getMaxZoom();
    updateRenderTileIDs();
}

void RenderPluginStyleLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    if (!willRender) {
        removeRenderTargets(changes);
        if (pluginImpl(baseImpl).registration.sourceKind == MLN_PLUGIN_SOURCE_RASTER_DEM) {
            removeAllDrawables();
            rasterGraphTiles.clear();
        }
    }
}

void RenderPluginStyleLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    RenderLayer::layerRemoved(changes);
    removeRenderTargets(changes);
    rasterGraphTiles.clear();
}

void RenderPluginStyleLayer::markContextDestroyed() {
    RenderLayer::markContextDestroyed();
    activatedRenderTargets.clear();
    rasterGraphTiles.clear();
    rasterSharedVertices.reset();
}

void RenderPluginStyleLayer::addRenderTarget(const RenderTargetPtr& target, UniqueChangeRequestVec& changes) {
    activateRenderTarget(target, true, changes);
    activatedRenderTargets.push_back(target);
}

void RenderPluginStyleLayer::removeRenderTargets(UniqueChangeRequestVec& changes) {
    for (const auto& target : activatedRenderTargets) activateRenderTarget(target, false, changes);
    activatedRenderTargets.clear();
}

void RenderPluginStyleLayer::update(gfx::ShaderRegistry& shaders,
                                    gfx::Context& context,
                                    const TransformState&,
                                    const std::shared_ptr<UpdateParameters>&,
                                    const PaintParameters& paintParameters,
                                    const RenderTree&,
                                    UniqueChangeRequestVec& changes) {
    const auto& registration = pluginImpl(baseImpl).registration;
    if (registration.sourceKind == MLN_PLUGIN_SOURCE_RASTER_DEM && registration.renderGraph) {
        updateRasterDEMGraph(shaders, context, paintParameters, changes);
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
    tileLayerGroup->setStencilTiles(renderTiles);
    if (!layerTweaker) {
        layerTweaker = std::make_shared<PluginLayerTweaker>(getID(), evaluatedProperties, registration);
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
                    attr->setSharedRawData(stream->second, binding.byteOffset, 0, stream->second->getRawSize(), type);
                }
                vertexCount = std::max(vertexCount, stream->second->getRawCount());
                if (firstType == gfx::AttributeDataType::Invalid) firstType = type;
            }
            if (!vertexCount || firstType == gfx::AttributeDataType::Invalid) continue;

            auto builder = context.createDrawableBuilder("plugin/" + registration.type);
            builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
            builder->setRenderPass(renderPass);
            builder->setEnableDepth(definition.depthMode != MLN_PLUGIN_DEPTH_DISABLED);
            builder->setDepthType(definition.depthMode == MLN_PLUGIN_DEPTH_READ_WRITE ? gfx::DepthMaskType::ReadWrite
                                                                                      : gfx::DepthMaskType::ReadOnly);
            builder->setIs3D(registration.requires3D);
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
                drawable->setData(std::make_unique<gfx::PluginRenderGraphDrawableData>(definition.shaderID));
                drawable->setRenderTile(renderTilesOwner, &tile);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        }
    }
}

void RenderPluginStyleLayer::updateRasterDEMGraph(gfx::ShaderRegistry& shaders,
                                                  gfx::Context& context,
                                                  const PaintParameters&,
                                                  UniqueChangeRequestVec& changes) {
    const auto& registration = pluginImpl(baseImpl).registration;
    const auto& graph = *registration.renderGraph;
    const auto mainRenderPass = renderPassFor(registration.renderStage);

    const auto deactivate = [&](const RenderTargetPtr& target) {
        activateRenderTarget(target, false, changes);
        activatedRenderTargets.erase(std::remove(activatedRenderTargets.begin(), activatedRenderTargets.end(), target),
                                     activatedRenderTargets.end());
    };
    const auto discardTileState = [&](const OverscaledTileID& tileID, RasterGraphTileState& tileState) {
        removeTile(mainRenderPass, tileID);
        for (const auto& [targetID, target] : tileState.renderTargets) {
            (void)targetID;
            deactivate(target);
        }
    };

    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        removeRenderTargets(changes);
        rasterGraphTiles.clear();
        return;
    }

    if (!layerGroup) {
        if (auto group = context.createTileLayerGroup(layerIndex, 64, getID())) {
            setLayerGroup(std::move(group), changes);
        } else {
            return;
        }
    }
    auto* mainGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    mainGroup->setStencilTiles(renderTiles);
    if (!layerTweaker) {
        layerTweaker = std::make_shared<PluginLayerTweaker>(getID(), evaluatedProperties, registration);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    std::set<OverscaledTileID> visibleTileIDs;
    for (const auto& tile : *renderTiles) visibleTileIDs.insert(tile.get().getOverscaledTileID());
    for (auto it = rasterGraphTiles.begin(); it != rasterGraphTiles.end();) {
        if (visibleTileIDs.find(it->first) == visibleTileIDs.end()) {
            discardTileState(it->first, it->second);
            it = rasterGraphTiles.erase(it);
        } else {
            ++it;
        }
    }
    stats.drawablesRemoved += mainGroup->removeDrawablesIf(
        [&](gfx::Drawable& drawable) { return drawable.getTileID() && !hasRenderTile(*drawable.getTileID()); });

    if (!rasterSharedVertices) {
        rasterSharedVertices = std::make_shared<gfx::VertexVector<HillshadeLayoutVertex>>(
            RenderStaticData::rasterVertices());
    }
    const auto staticIndices = RenderStaticData::quadTriangleIndices();
    const auto staticSegments = RenderStaticData::rasterSegments();

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();
        auto* rawBucket = tile.getBucket(*baseImpl);
        if (!rawBucket || !rawBucket->hasData()) {
            if (const auto it = rasterGraphTiles.find(tileID); it != rasterGraphTiles.end()) {
                discardTileState(tileID, it->second);
                rasterGraphTiles.erase(it);
            }
            continue;
        }
        auto& bucket = static_cast<HillshadeBucket&>(*rawBucket);
        auto existing = rasterGraphTiles.find(tileID);
        if (existing != rasterGraphTiles.end() &&
            (existing->second.bucketID != bucket.getID() || existing->second.demRevision != bucket.getDEMRevision() ||
             existing->second.maskRevision != bucket.getMaskRevision())) {
            discardTileState(tileID, existing->second);
            rasterGraphTiles.erase(existing);
            existing = rasterGraphTiles.end();
        }
        if (existing != rasterGraphTiles.end()) continue;

        RasterGraphTileState tileState;
        tileState.bucketID = bucket.getID();
        tileState.demRevision = bucket.getDEMRevision();
        tileState.maskRevision = bucket.getMaskRevision();
        tileState.sourceTexture = context.createTexture2D();
        tileState.sourceTexture->setImage(bucket.getDEMData().getImagePtr());

        const auto dimension = static_cast<uint32_t>(bucket.getDEMData().dim);
        const auto stride = static_cast<uint32_t>(bucket.getDEMData().stride);
        for (const auto& targetDefinition : graph.renderTargets) {
            auto target = context.createRenderTarget({dimension, dimension}, gfx::TextureChannelDataType::UnsignedByte);
            if (!target) continue;
            auto targetGroup = context.createTileLayerGroup(0, 1, getID());
            if (!targetGroup) continue;
            targetGroup->addLayerTweaker(layerTweaker);
            target->addLayerGroup(targetGroup, true);
            addRenderTarget(target, changes);
            tileState.renderTargets.emplace(targetDefinition.id, std::move(target));
        }
        if (tileState.renderTargets.size() != graph.renderTargets.size()) {
            for (const auto& [targetID, target] : tileState.renderTargets) {
                (void)targetID;
                deactivate(target);
            }
            continue;
        }

        bool complete = true;
        for (const auto& pass : graph.passes) {
            const auto* shaderDefinition = findShader(registration, pass.shaderID);
            const auto shaderGroup = shaders.getShaderGroup(
                plugin::shaderGroupName(registration.pluginID, pass.shaderID));
            const auto shader = shaderGroup ? shaderGroup->getOrCreateShader(context, {}) : gfx::ShaderPtr{};
            if (!shaderDefinition || !shader) {
                complete = false;
                break;
            }

            const bool masked = pass.geometry == MLN_PLUGIN_GRAPH_GEOMETRY_RASTER_DEM_MASKED_TILE &&
                                !bucket.vertices.empty() && !bucket.indices.empty() && !bucket.segments.empty();
            const auto vertices = masked ? bucket.sharedVertices : rasterSharedVertices;
            std::shared_ptr<gfx::IndexVector<gfx::Triangles>> indices =
                masked ? bucket.sharedIndices : std::make_shared<gfx::IndexVector<gfx::Triangles>>(staticIndices);
            const auto* segments = masked ? &bucket.segments : &staticSegments;

            auto attributes = context.createVertexAttributeArray();
            for (const auto& attribute : shaderDefinition->attributes) {
                const auto& binding = attributes->set(attribute.id);
                if (!binding) continue;
                if (attribute.location == 0) {
                    binding->setSharedRawData(vertices,
                                              offsetof(HillshadeLayoutVertex, a1),
                                              0,
                                              sizeof(HillshadeLayoutVertex),
                                              gfx::AttributeDataType::Short2);
                } else if (attribute.location == 1) {
                    binding->setSharedRawData(vertices,
                                              offsetof(HillshadeLayoutVertex, a2),
                                              0,
                                              sizeof(HillshadeLayoutVertex),
                                              gfx::AttributeDataType::Short2);
                }
            }

            auto builder = context.createDrawableBuilder("plugin/" + registration.type + "/" + pass.shaderID);
            builder->setShader(std::static_pointer_cast<gfx::ShaderProgramBase>(shader));
            builder->setRenderPass(pass.renderTargetID ? RenderPass::Translucent : mainRenderPass);
            builder->setEnableDepth(pass.depthMode != MLN_PLUGIN_DEPTH_DISABLED);
            builder->setDepthType(pass.depthMode == MLN_PLUGIN_DEPTH_READ_WRITE ? gfx::DepthMaskType::ReadWrite
                                                                                : gfx::DepthMaskType::ReadOnly);
            builder->setIs3D(registration.requires3D);
            builder->setColorMode(colorMode(pass.blendMode));
            builder->setCullFaceMode(pass.enableCullFace ? gfx::CullFaceMode::backCCW()
                                                         : gfx::CullFaceMode::disabled());
            builder->setEnableStencil(pass.enableStencil);
            builder->setVertexAttributes(std::move(attributes));
            builder->setRawVertices({}, vertices->elements(), gfx::AttributeDataType::Short2);
            switch (pass.drawMode) {
                case MLN_PLUGIN_DRAW_MODE_TRIANGLES:
                    builder->setSegments(gfx::Triangles(), indices->vector(), segments->data(), segments->size());
                    break;
                case MLN_PLUGIN_DRAW_MODE_LINES:
                case MLN_PLUGIN_DRAW_MODE_POINTS:
                    complete = false;
                    break;
            }
            if (!complete) break;

            for (const auto& textureBinding : pass.textures) {
                const auto* textureDefinition = findTexture(*shaderDefinition, textureBinding.textureID);
                if (!textureDefinition) {
                    complete = false;
                    break;
                }
                gfx::Texture2DPtr texture;
                if (textureBinding.source == MLN_PLUGIN_TEXTURE_SOURCE_RASTER_DEM) {
                    texture = tileState.sourceTexture;
                } else {
                    const auto targetIt = tileState.renderTargets.find(textureBinding.renderTargetID);
                    if (targetIt != tileState.renderTargets.end()) texture = targetIt->second->getTexture();
                }
                if (!texture) {
                    complete = false;
                    break;
                }
                texture->setSamplerConfiguration({.filter = textureFilter(textureBinding.filter),
                                                  .wrapU = textureWrap(textureBinding.wrapU),
                                                  .wrapV = textureWrap(textureBinding.wrapV)});
                builder->setTexture(texture, textureDefinition->id);
            }
            if (!complete) break;

            builder->flush(context);
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                drawable->setData(
                    std::make_unique<gfx::PluginRenderGraphDrawableData>(pass.shaderID,
                                                                         pass.id,
                                                                         dimension,
                                                                         stride,
                                                                         demEncoding(bucket.getDEMData().encoding),
                                                                         sourceMaxZoom));
                if (pass.renderTargetID) {
                    auto targetGroup = std::static_pointer_cast<TileLayerGroup>(
                        tileState.renderTargets.at(pass.renderTargetID)->getLayerGroup(0));
                    targetGroup->addDrawable(RenderPass::Translucent, tileID, std::move(drawable));
                } else {
                    drawable->setRenderTile(renderTilesOwner, &tile);
                    mainGroup->addDrawable(mainRenderPass, tileID, std::move(drawable));
                }
                ++stats.drawablesAdded;
            }
        }

        if (!complete) {
            removeTile(mainRenderPass, tileID);
            for (const auto& [targetID, target] : tileState.renderTargets) {
                (void)targetID;
                deactivate(target);
            }
            continue;
        }
        setRenderTileBucketID(tileID, bucket.getID());
        rasterGraphTiles.emplace(tileID, std::move(tileState));
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
    std::vector<style::PluginPropertyValue::EvaluationStorage> storage(definitions.size());
    properties.reserve(definitions.size());
    for (size_t i = 0; i < definitions.size(); ++i) {
        const auto& definition = definitions[i];
        const auto propertyIt = impl.pluginProperties.find(definition.name);
        const auto value = propertyIt == impl.pluginProperties.end() ? style::defaultPluginPropertyValue(definition)
                                                                     : propertyIt->second;
        mln_plugin_property_value_v1 property{};
        property.struct_size = sizeof(property);
        property.name = {definition.name.data(), definition.name.size()};
        property.value = value.evaluate(zoom, feature, featureState, definition, storage[i]);
        property.explicitly_set = propertyIt != impl.pluginProperties.end();
        properties.push_back(property);
    }
    pluginFeature.evaluated_properties = properties.data();
    pluginFeature.evaluated_property_count = properties.size();
    return registration.queryFeature(
               &pluginFeature, query.data(), query.size(), pixelsToTileUnits, properties.data(), properties.size()) !=
           0;
}

} // namespace mln
