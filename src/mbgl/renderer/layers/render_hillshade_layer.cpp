#include <mbgl/renderer/layers/render_hillshade_layer.hpp>
#include <mbgl/renderer/buckets/hillshade_bucket.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/sources/render_raster_dem_source.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/hillshade_program.hpp>
#include <mbgl/programs/hillshade_prepare_program.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/style/layers/hillshade_layer_impl.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/offscreen_texture.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/util/geo.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/renderer/layers/hillshade_layer_tweaker.hpp>
#include <mbgl/renderer/layers/hillshade_prepare_layer_tweaker.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/render_target.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/hillshade_prepare_drawable_data.hpp>
#include <mbgl/gfx/shader_group.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#endif

namespace mbgl {

using namespace style;

namespace {

inline const HillshadeLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == HillshadeLayer::Impl::staticTypeInfo());
    return static_cast<const HillshadeLayer::Impl&>(*impl);
}

} // namespace

RenderHillshadeLayer::RenderHillshadeLayer(Immutable<style::HillshadeLayer::Impl> _impl)
    : RenderLayer(makeMutable<HillshadeLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderHillshadeLayer::~RenderHillshadeLayer() = default;

std::array<float, 2> RenderHillshadeLayer::getLatRange(const UnwrappedTileID& id) {
    const LatLng latlng0 = LatLng(id);
    const LatLng latlng1 = LatLng(UnwrappedTileID(id.canonical.z, id.canonical.x, id.canonical.y + 1));
    return {{static_cast<float>(latlng0.latitude()), static_cast<float>(latlng1.latitude())}};
}

std::array<float, 2> RenderHillshadeLayer::getLight(const PaintParameters& parameters) {
    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;
    float azimuthal = util::deg2radf(evaluated.get<HillshadeIlluminationDirection>());
    if (evaluated.get<HillshadeIlluminationAnchor>() == HillshadeIlluminationAnchorType::Viewport)
        azimuthal = azimuthal - static_cast<float>(parameters.state.getBearing());
    return {{evaluated.get<HillshadeExaggeration>(), azimuthal}};
}

void RenderHillshadeLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderHillshadeLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<HillshadeLayerProperties>(staticImmutableCast<HillshadeLayer::Impl>(baseImpl),
                                                            unevaluated.evaluate(parameters));
    passes = (properties->evaluated.get<style::HillshadeExaggeration>() > 0)
                 ? (RenderPass::Translucent | RenderPass::Pass3D)
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);
#if MLN_DRAWABLE_RENDERER
    /*if (renderTarget) {
        renderTarget->getLayerGroup(0)->setLayerTweaker(std::make_shared<HillshadePrepareLayerTweaker>(evaluatedProperties));
    }*/
    if (layerGroup) {
        layerGroup->setLayerTweaker(std::make_shared<HillshadeLayerTweaker>(evaluatedProperties));
    }
#endif
}

bool RenderHillshadeLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderHillshadeLayer::hasCrossfade() const {
    return false;
}

void RenderHillshadeLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTiles();
    maxzoom = params.source->getMaxZoom();
}

#if MLN_LEGACY_RENDERER
void RenderHillshadeLayer::render(PaintParameters& parameters) {
    assert(renderTiles);
    if (parameters.pass != RenderPass::Translucent && parameters.pass != RenderPass::Pass3D) return;

    if (!parameters.shaders.getLegacyGroup().populate(hillshadeProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(hillshadePrepareProgram)) return;

    const auto& evaluated = static_cast<const HillshadeLayerProperties&>(*evaluatedProperties).evaluated;
    auto draw = [&](const mat4& matrix,
                    const auto& vertexBuffer,
                    const auto& indexBuffer,
                    const auto& segments,
                    const UnwrappedTileID& id,
                    const auto& textureBindings) {
        const HillshadeProgram::Binders paintAttributeData{evaluated, 0};

        const auto allUniformValues = HillshadeProgram::computeAllUniformValues(
            HillshadeProgram::LayoutUniformValues{
                uniforms::matrix::Value(matrix),
                uniforms::highlight::Value(evaluated.get<HillshadeHighlightColor>()),
                uniforms::shadow::Value(evaluated.get<HillshadeShadowColor>()),
                uniforms::accent::Value(evaluated.get<HillshadeAccentColor>()),
                uniforms::light::Value(getLight(parameters)),
                uniforms::latrange::Value(getLatRange(id)),
            },
            paintAttributeData,
            evaluated,
            static_cast<float>(parameters.state.getZoom()));
        const auto allAttributeBindings = HillshadeProgram::computeAllAttributeBindings(
            vertexBuffer, paintAttributeData, evaluated);

        checkRenderability(parameters, HillshadeProgram::activeBindingCount(allAttributeBindings));

        hillshadeProgram->draw(parameters.context,
                               *parameters.renderPass,
                               gfx::Triangles(),
                               parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                               gfx::StencilMode::disabled(),
                               parameters.colorModeForRenderPass(),
                               gfx::CullFaceMode::disabled(),
                               indexBuffer,
                               segments,
                               allUniformValues,
                               allAttributeBindings,
                               textureBindings,
                               getID());
    };

    mat4 mat;
    matrix::ortho(mat, 0, util::EXTENT, -util::EXTENT, 0, 0, 1);
    matrix::translate(mat, mat, 0, -util::EXTENT, 0);

    for (const RenderTile& tile : *renderTiles) {
        auto* bucket_ = tile.getBucket(*baseImpl);
        if (!bucket_) {
            continue;
        }
        auto& bucket = static_cast<HillshadeBucket&>(*bucket_);

        if (!bucket.hasData()) {
            continue;
        }

        if (!bucket.isPrepared() && parameters.pass == RenderPass::Pass3D) {
            assert(bucket.dem);
            const uint16_t stride = bucket.getDEMData().stride;
            const uint16_t tilesize = bucket.getDEMData().dim;
            auto view = parameters.context.createOffscreenTexture({tilesize, tilesize},
                                                                  gfx::TextureChannelDataType::UnsignedByte);

            auto renderPass = parameters.encoder->createRenderPass("hillshade prepare",
                                                                   {*view, Color{0.0f, 0.0f, 0.0f, 0.0f}, {}, {}});

            const Properties<>::PossiblyEvaluated properties;
            const HillshadePrepareProgram::Binders paintAttributeData{properties, 0};

            const auto allUniformValues = HillshadePrepareProgram::computeAllUniformValues(
                HillshadePrepareProgram::LayoutUniformValues{
                    uniforms::matrix::Value(mat),
                    uniforms::dimension::Value({{stride, stride}}),
                    uniforms::zoom::Value(static_cast<float>(tile.id.canonical.z)),
                    uniforms::maxzoom::Value(static_cast<float>(maxzoom)),
                    uniforms::unpack::Value(bucket.getDEMData().getUnpackVector()),
                },
                paintAttributeData,
                properties,
                static_cast<float>(parameters.state.getZoom()));
            const auto allAttributeBindings = HillshadePrepareProgram::computeAllAttributeBindings(
                *parameters.staticData.rasterVertexBuffer, paintAttributeData, properties);

            checkRenderability(parameters, HillshadePrepareProgram::activeBindingCount(allAttributeBindings));

            // Copy over the segments so that we can create our own DrawScopes
            // that get destroyed after this draw call.
            auto segments = RenderStaticData::rasterSegments();
            hillshadePrepareProgram->draw(parameters.context,
                                          *renderPass,
                                          gfx::Triangles(),
                                          parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                                          gfx::StencilMode::disabled(),
                                          parameters.colorModeForRenderPass(),
                                          gfx::CullFaceMode::disabled(),
                                          *parameters.staticData.quadTriangleIndexBuffer,
                                          segments,
                                          allUniformValues,
                                          allAttributeBindings,
                                          HillshadePrepareProgram::TextureBindings{
                                              textures::image::Value{bucket.dem->getResource()},
                                          },
                                          "prepare");
            bucket.texture = std::move(view->getTexture());
            bucket.setPrepared(true);
        } else if (parameters.pass == RenderPass::Translucent) {
            assert(bucket.texture);

            if (bucket.vertexBuffer && bucket.indexBuffer) {
                // Draw only the parts of the tile that aren't drawn by another tile in the layer.
                draw(parameters.matrixForTile(tile.id, true),
                     *bucket.vertexBuffer,
                     *bucket.indexBuffer,
                     bucket.segments,
                     tile.id,
                     HillshadeProgram::TextureBindings{
                         textures::image::Value{bucket.texture->getResource(), gfx::TextureFilterType::Linear},
                     });
            } else {
                // Draw the full tile.
                if (bucket.segments.empty()) {
                    // Copy over the segments so that we can create our own DrawScopes.
                    bucket.segments = RenderStaticData::rasterSegments();
                }
                draw(parameters.matrixForTile(tile.id, true),
                     *parameters.staticData.rasterVertexBuffer,
                     *parameters.staticData.quadTriangleIndexBuffer,
                     bucket.segments,
                     tile.id,
                     HillshadeProgram::TextureBindings{
                         textures::image::Value{bucket.texture->getResource(), gfx::TextureFilterType::Linear},
                     });
            }
        }
    }
}
#endif // MLN_LEGACY_RENDERER

#if MLN_DRAWABLE_RENDERER
void activateRenderTarget(const RenderTargetPtr& renderTarget_, bool activate, UniqueChangeRequestVec& changes) {
    if (renderTarget_) {
        if (activate) {
            // The RenderTree has determined this render target should be included in the renderable set for a frame
            changes.emplace_back(std::make_unique<AddRenderTargetRequest>(renderTarget_));
        } else {
            // The RenderTree is informing us we should not render anything
            changes.emplace_back(std::make_unique<RemoveRenderTargetRequest>(renderTarget_));
        }
    }
}

void RenderHillshadeLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    for (auto pair : renderTargets) {
        activateRenderTarget(pair.second, willRender, changes);
    }
}

void RenderHillshadeLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    RenderLayer::removeTile(renderPass, tileID);
}

void RenderHillshadeLayer::removeAllDrawables() {
    RenderLayer::removeAllDrawables();
}

static const std::string HillshadePrepareShaderGroupName = "HillshadePrepareShader";
static const std::string HillshadeShaderGroupName = "HillshadeShader";

void RenderHillshadeLayer::update(gfx::ShaderRegistry& shaders,
                                gfx::Context& context,
                                const TransformState& state,
                                [[maybe_unused]] const RenderTree& renderTree,
                                [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!renderTiles || renderTiles->empty()) {
        removeAllDrawables();
        for (auto pair : renderTargets) {
            activateRenderTarget(pair.second, false, changes);
        }
        renderTargets.clear();
        return;
    }

    // Set up a layer group
    if (!layerGroup) {
        auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
        if (!layerGroup_) {
            return;
        }
        layerGroup_->setLayerTweaker(std::make_shared<HillshadeLayerTweaker>(evaluatedProperties));
        setLayerGroup(std::move(layerGroup_), changes);
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

    if (!hillshadePrepareShader) {
        hillshadePrepareShader = context.getGenericShader(shaders, HillshadePrepareShaderGroupName);
    }
    if (!hillshadeShader) {
        hillshadeShader = context.getGenericShader(shaders, HillshadeShaderGroupName);
    }
    if (!hillshadePrepareShader || !hillshadeShader) {
        removeAllDrawables();
        return;
    }

    std::unordered_set<OverscaledTileID> newTileIDs(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(newTileIDs, newTileIDs.begin()),
                   [](const auto& renderTile) -> OverscaledTileID { return renderTile.get().getOverscaledTileID(); });

    std::unique_ptr<gfx::DrawableBuilder> hillshadeBuilder;
    std::unique_ptr<gfx::DrawableBuilder> hillshadePrepareBuilder;
    std::vector<gfx::DrawablePtr> newTiles;
    gfx::VertexAttributeArray hillshadePrepareVertexAttrs;
    gfx::VertexAttributeArray hillshadeVertexAttrs;
    auto renderPass = RenderPass::Translucent;

    if (!(mbgl::underlying_type(renderPass) & evaluatedProperties->renderPasses)) {
        return;
    }

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        const auto tileID = drawable->getTileID();
        if (tileID && newTileIDs.find(*tileID) == newTileIDs.end()) {
            // remove it
            drawable.reset();
            ++stats.drawablesRemoved;
            
            const auto result = renderTargets.find(*tileID);
            if (result != renderTargets.end()) {
                activateRenderTarget(result->second, false, changes);
                renderTargets.erase(result);
            }
        }
    });

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();
        
        auto* bucket_ = tile.getBucket(*baseImpl);
        if (!bucket_) {
            removeTile(renderPass, tileID);
            continue;
        }
        auto& bucket = static_cast<HillshadeBucket&>(*bucket_);

        if (!bucket.hasData()) {
            continue;
        }
        
        if (tileLayerGroup->getDrawableCount(renderPass, tileID) > 0) {
            continue;
        }

        // Set up tile render target
        const uint16_t tilesize = bucket.getDEMData().dim;
        auto renderTarget = context.createRenderTarget({tilesize, tilesize}, gfx::TextureChannelDataType::UnsignedByte);
        if (!renderTarget) {
            continue;
        }
        if (!renderTargets.insert(std::make_pair(tileID, renderTarget)).second) {
            continue;
        }
        activateRenderTarget(renderTarget, true, changes);
        
        auto singleTileLayerGroup = context.createTileLayerGroup(0, /*initialCapacity=*/1, getID());
        if (!singleTileLayerGroup) {
            return;
        }
        singleTileLayerGroup->setLayerTweaker(std::make_shared<HillshadePrepareLayerTweaker>(evaluatedProperties));
        renderTarget->addLayerGroup(singleTileLayerGroup, /*canReplace=*/true);
        
        hillshadePrepareVertexAttrs.clear();
        
        const auto staticDataVertices = RenderStaticData::rasterVertices();
        const auto staticDataIndices = RenderStaticData::quadTriangleIndices();
        const auto staticDataSegments = RenderStaticData::rasterSegments();
        
        if (auto& attr = hillshadePrepareVertexAttrs.getOrAdd("a_texture_pos")) {
            std::size_t index{0};
            for (auto& v : staticDataVertices.vector()) {
                attr->set<gfx::VertexAttribute::int2>(index++, {v.a2[0], v.a2[1]});
            }
        }
        
        std::vector<std::array<int16_t, 2>> prepareRawVerts;
        const auto buildPrepareVertices = [&]() {
            const auto& verts = staticDataVertices.vector();
            if (prepareRawVerts.size() < verts.size()) {
                prepareRawVerts.resize(verts.size());
                std::transform(verts.begin(), verts.end(), prepareRawVerts.begin(), [](const auto& x) { return x.a1; });
            }
        };

        hillshadePrepareBuilder = context.createDrawableBuilder("hillshadePrepare");
        hillshadePrepareBuilder->setShader(hillshadePrepareShader);
        hillshadePrepareBuilder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                                 : gfx::DepthMaskType::ReadOnly);
        hillshadePrepareBuilder->setColorMode(gfx::ColorMode::alphaBlended());
        hillshadePrepareBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());

        hillshadePrepareBuilder->setRenderPass(renderPass);
        hillshadePrepareBuilder->setVertexAttributes(hillshadePrepareVertexAttrs);

        buildPrepareVertices();
        hillshadePrepareBuilder->addVertices(prepareRawVerts, 0, prepareRawVerts.size());

        hillshadePrepareBuilder->setSegments(gfx::Triangles(), staticDataIndices.vector(), staticDataSegments.data(), staticDataSegments.size());
        
        auto prepareImageLocation = hillshadePrepareShader->getSamplerLocation("u_image");
        if (prepareImageLocation.has_value()) {
            std::shared_ptr<gfx::Texture2D> texture = context.createTexture2D();
            texture->setImage(bucket.getDEMData().getImagePtr());
            texture->setSamplerConfiguration({gfx::TextureFilterType::Linear, gfx::TextureWrapType::Clamp, gfx::TextureWrapType::Clamp});
            hillshadePrepareBuilder->setTexture(texture, prepareImageLocation.value());
        }
        
        hillshadePrepareBuilder->flush();

        for (auto& drawable : hillshadePrepareBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            drawable->setData(std::make_unique<gfx::HillshadePrepareDrawableData>(bucket.getDEMData().stride, bucket.getDEMData().encoding));
            singleTileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
        
        // Continue with tile
        hillshadeVertexAttrs.clear();
        
        auto* vertices = &staticDataVertices;
        auto* indices = &staticDataIndices;
        auto* segments = &staticDataSegments;
        
        if (!bucket.vertices.empty() && !bucket.indices.empty()) {
            vertices = &bucket.vertices;
            indices = &bucket.indices;
            if (!bucket.segments.empty()) {
                segments = &bucket.segments;
            }
        }
        
        if (auto& attr = hillshadeVertexAttrs.getOrAdd("a_texture_pos")) {
            std::size_t index{0};
            for (auto& v : vertices->vector()) {
                attr->set<gfx::VertexAttribute::int2>(index++, {v.a2[0], v.a2[1]});
            }
        }
        
        std::vector<std::array<int16_t, 2>> rawVerts;
        const auto buildVertices = [&]() {
            const auto& verts = vertices->vector();
            if (rawVerts.size() < verts.size()) {
                rawVerts.resize(verts.size());
                std::transform(verts.begin(), verts.end(), rawVerts.begin(), [](const auto& x) { return x.a1; });
            }
        };

        hillshadeBuilder = context.createDrawableBuilder("hillshade");
        hillshadeBuilder->setShader(hillshadeShader);
        hillshadeBuilder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                          : gfx::DepthMaskType::ReadOnly);
        hillshadeBuilder->setColorMode(gfx::ColorMode::alphaBlended());
        hillshadeBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());

        hillshadeBuilder->setRenderPass(renderPass);
        hillshadeBuilder->setVertexAttributes(hillshadeVertexAttrs);

        buildVertices();
        hillshadeBuilder->addVertices(rawVerts, 0, rawVerts.size());

        hillshadeBuilder->setSegments(gfx::Triangles(), indices->vector(), segments->data(), segments->size());
        
        auto imageLocation = hillshadeShader->getSamplerLocation("u_image");
        if (imageLocation.has_value()) {
            hillshadeBuilder->setTexture(renderTarget->getTexture(), imageLocation.value());
        }
        
        hillshadeBuilder->flush();

        for (auto& drawable : hillshadeBuilder->clearDrawables()) {
            drawable->setTileID(tileID);
            tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
            ++stats.drawablesAdded;
        }
    }
}
#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
