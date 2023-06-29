#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/render_pass.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/programs/fill_extrusion_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/fill_extrusion_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/layers/render_fill_extrusion_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/fill_extrusion_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/math.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/fill_extrusion_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/fill_extrusion_layer_tweaker.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#endif // MLN_DRAWABLE_RENDERER

namespace mbgl {

using namespace style;

namespace {

#if MLN_DRAWABLE_RENDERER

constexpr std::string_view FillExtrusionShaderName = "FillExtrusionShader";
constexpr std::string_view FillExtrusionPatternShaderName = "FillExtrusionPatternShader";

constexpr auto normAttribName = "a_normal_ed";

#endif // MLN_DRAWABLE_RENDERER

inline const FillExtrusionLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == FillExtrusionLayer::Impl::staticTypeInfo());
    return static_cast<const FillExtrusionLayer::Impl&>(*impl);
}

} // namespace

RenderFillExtrusionLayer::RenderFillExtrusionLayer(Immutable<style::FillExtrusionLayer::Impl> _impl)
    : RenderLayer(makeMutable<FillExtrusionLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderFillExtrusionLayer::~RenderFillExtrusionLayer() = default;

void RenderFillExtrusionLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
}

void RenderFillExtrusionLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<FillExtrusionLayerProperties>(staticImmutableCast<FillExtrusionLayer::Impl>(baseImpl),
                                                                parameters.getCrossfadeParameters(),
                                                                unevaluated.evaluate(parameters));

    passes = (properties->evaluated.get<style::FillExtrusionOpacity>() > 0)
                 ? (RenderPass::Translucent | RenderPass::Pass3D)
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

#if MLN_DRAWABLE_RENDERER
    if (layerGroup) {
        layerGroup->setLayerTweaker(std::make_shared<FillExtrusionLayerTweaker>(evaluatedProperties));
    }
#endif // MLN_DRAWABLE_RENDERER
}

bool RenderFillExtrusionLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderFillExtrusionLayer::hasCrossfade() const {
    return getCrossfade<FillExtrusionLayerProperties>(evaluatedProperties).t != 1;
}

bool RenderFillExtrusionLayer::is3D() const {
    return true;
}

void RenderFillExtrusionLayer::prepare(const LayerPrepareParameters& params) {
    RenderLayer::prepare(params);

#if MLN_DRAWABLE_RENDERER
    updateRenderTileIDs();
#endif // MLN_DRAWABLE_RENDERER
}

#if MLN_LEGACY_RENDERER
void RenderFillExtrusionLayer::render(PaintParameters& parameters) {
    assert(renderTiles);
    if (parameters.pass != RenderPass::Translucent) {
        return;
    }

    if (!parameters.shaders.getLegacyGroup().populate(fillExtrusionProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(fillExtrusionPatternProgram)) return;

    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).crossfade;
    if (evaluatedProperties->renderPasses == mbgl::underlying_type(RenderPass::None)) {
        return;
    }

    const auto depthMode = parameters.depthModeFor3D();

    auto draw = [&](auto& programInstance,
                    const auto& evaluated_,
                    const auto& crossfade_,
                    const gfx::StencilMode& stencilMode,
                    const gfx::ColorMode& colorMode,
                    const auto& tileBucket,
                    const auto& uniformValues,
                    const std::optional<ImagePosition>& patternPositionA,
                    const std::optional<ImagePosition>& patternPositionB,
                    const auto& textureBindings,
                    const std::string& uniqueName) {
        const auto& paintPropertyBinders = tileBucket.paintPropertyBinders.at(getID());
        paintPropertyBinders.setPatternParameters(patternPositionA, patternPositionB, crossfade_);

        const auto allUniformValues = programInstance.computeAllUniformValues(
            uniformValues, paintPropertyBinders, evaluated_, static_cast<float>(parameters.state.getZoom()));
        const auto allAttributeBindings = programInstance.computeAllAttributeBindings(
            *tileBucket.vertexBuffer, paintPropertyBinders, evaluated_);

        checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));

        programInstance.draw(parameters.context,
                             *parameters.renderPass,
                             gfx::Triangles(),
                             depthMode,
                             stencilMode,
                             colorMode,
                             gfx::CullFaceMode::backCCW(),
                             *tileBucket.indexBuffer,
                             tileBucket.triangleSegments,
                             allUniformValues,
                             allAttributeBindings,
                             textureBindings,
                             getID() + "/" + uniqueName);
    };

    if (unevaluated.get<FillExtrusionPattern>().isUndefined()) {
        // Draw solid color extrusions
        auto drawTiles =
            [&](const gfx::StencilMode& stencilMode_, const gfx::ColorMode& colorMode_, const std::string& name) {
                for (const RenderTile& tile : *renderTiles) {
                    const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
                    if (!renderData) {
                        continue;
                    }
                    const auto& bucket = static_cast<FillExtrusionBucket&>(*renderData->bucket);
                    draw(*fillExtrusionProgram,
                         evaluated,
                         crossfade,
                         stencilMode_,
                         colorMode_,
                         bucket,
                         FillExtrusionProgram::layoutUniformValues(
                             tile.translatedClipMatrix(evaluated.get<FillExtrusionTranslate>(),
                                                       evaluated.get<FillExtrusionTranslateAnchor>(),
                                                       parameters.state),
                             parameters.state,
                             evaluated.get<FillExtrusionOpacity>(),
                             parameters.evaluatedLight,
                             evaluated.get<FillExtrusionVerticalGradient>()),
                         {},
                         {},
                         FillExtrusionProgram::TextureBindings{},
                         name);
                }
            };

        if (evaluated.get<FillExtrusionOpacity>() == 1) {
            // Draw opaque extrusions
            drawTiles(gfx::StencilMode::disabled(), parameters.colorModeForRenderPass(), "color");
        } else {
            // Draw transparent buildings in two passes so that only the closest
            // surface is drawn. First draw all the extrusions into only the
            // depth buffer. No colors are drawn.
            drawTiles(gfx::StencilMode::disabled(), gfx::ColorMode::disabled(), "depth");

            // Then draw all the extrusions a second time, only coloring
            // fragments if they have the same depth value as the closest
            // fragment in the previous pass. Use the stencil buffer to prevent
            // the second draw in cases where we have coincident polygons.
            drawTiles(parameters.stencilModeFor3D(), parameters.colorModeForRenderPass(), "color");
        }
    } else {
        // Draw textured extrusions
        const auto fillPatternValue = evaluated.get<FillExtrusionPattern>().constantOr(
            mbgl::Faded<expression::Image>{"", ""});
        auto drawTiles =
            [&](const gfx::StencilMode& stencilMode_, const gfx::ColorMode& colorMode_, const std::string& name) {
                for (const RenderTile& tile : *renderTiles) {
                    const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
                    if (!renderData) {
                        continue;
                    }
                    const auto& bucket = static_cast<FillExtrusionBucket&>(*renderData->bucket);
                    const std::optional<ImagePosition> patternPosA = tile.getPattern(fillPatternValue.from.id());
                    const std::optional<ImagePosition> patternPosB = tile.getPattern(fillPatternValue.to.id());
                    const auto numTiles = std::pow(2, tile.id.canonical.z);
                    const auto heightFactor = static_cast<float>(-numTiles / util::tileSize_D / 8.0);

                    draw(*fillExtrusionPatternProgram,
                         evaluated,
                         crossfade,
                         stencilMode_,
                         colorMode_,
                         bucket,
                         FillExtrusionPatternProgram::layoutUniformValues(
                             tile.translatedClipMatrix(evaluated.get<FillExtrusionTranslate>(),
                                                       evaluated.get<FillExtrusionTranslateAnchor>(),
                                                       parameters.state),
                             tile.getIconAtlasTexture()->getSize(),
                             crossfade,
                             tile.id,
                             parameters.state,
                             evaluated.get<FillExtrusionOpacity>(),
                             heightFactor,
                             parameters.pixelRatio,
                             parameters.evaluatedLight,
                             evaluated.get<FillExtrusionVerticalGradient>()),
                         patternPosA,
                         patternPosB,
                         FillExtrusionPatternProgram::TextureBindings{
                             textures::image::Value{tile.getIconAtlasTextureBinding(gfx::TextureFilterType::Linear)},
                         },
                         name);
                }
            };

        // Draw transparent buildings in two passes so that only the closest
        // surface is drawn. First draw all the extrusions into only the depth
        // buffer. No colors are drawn.
        drawTiles(gfx::StencilMode::disabled(), gfx::ColorMode::disabled(), "depth");

        // Then draw all the extrusions a second time, only coloring fragments
        // if they have the same depth value as the closest fragment in the
        // previous pass. Use the stencil buffer to prevent the second draw in
        // cases where we have coincident polygons.
        drawTiles(parameters.stencilModeFor3D(), parameters.colorModeForRenderPass(), "color");
    }
}
#endif // MLN_LEGACY_RENDERER

bool RenderFillExtrusionLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                                      const GeometryTileFeature& feature,
                                                      const float,
                                                      const TransformState& transformState,
                                                      const float pixelsToTileUnits,
                                                      const mat4&,
                                                      const FeatureState&) const {
    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;
    auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(
        queryGeometry,
        evaluated.get<style::FillExtrusionTranslate>(),
        evaluated.get<style::FillExtrusionTranslateAnchor>(),
        static_cast<float>(transformState.getBearing()),
        pixelsToTileUnits);

    return util::polygonIntersectsMultiPolygon(translatedQueryGeometry.value_or(queryGeometry),
                                               feature.getGeometries());
}

#if MLN_DRAWABLE_RENDERER

void RenderFillExtrusionLayer::update(gfx::ShaderRegistry& shaders,
                                      gfx::Context& context,
                                      const TransformState& state,
                                      const RenderTree& /*renderTree*/,
                                      UniqueChangeRequestVec& changes) {
    if (!renderTiles || renderTiles->empty() || passes == RenderPass::None) {
        removeAllDrawables();
        return;
    }

    // Set up a layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            layerGroup_->setLayerTweaker(std::make_shared<FillExtrusionLayerTweaker>(evaluatedProperties));
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }

    if (!fillExtrusionGroup) {
        fillExtrusionGroup = shaders.getShaderGroup(std::string(FillExtrusionShaderName));
    }
    if (!fillExtrusionPatternGroup) {
        fillExtrusionPatternGroup = shaders.getShaderGroup(std::string(FillExtrusionPatternShaderName));
    }

    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());

    const auto& evaluated = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).evaluated;
    const auto& crossfade = static_cast<const FillExtrusionLayerProperties&>(*evaluatedProperties).crossfade;

    // If `FillExtrusionOpacity` is zero, no need to render anything
    if (evaluatedProperties->renderPasses == mbgl::underlying_type(RenderPass::None)) {
        removeAllDrawables();
        return;
    }

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto tileID = drawable->getTileID();
        if (drawable->getRenderPass() != passes || (tileID && renderTileIDs.find(*tileID) == renderTileIDs.end())) {
            drawable.reset();
            ++stats.drawablesRemoved;
        }
    });

    const auto zoom = static_cast<float>(state.getZoom());
    const auto layerPrefix = getID() + "/";
    const auto hasPattern = !unevaluated.get<FillExtrusionPattern>().isUndefined();
    const auto opaque = evaluated.get<FillExtrusionOpacity>() >= 1;

    std::unique_ptr<gfx::DrawableBuilder> depthBuilder;
    std::unique_ptr<gfx::DrawableBuilder> colorBuilder;
    gfx::VertexAttributeArray vertexAttrs;
    std::vector<uint8_t> rawVertices;

    const auto& shaderGroup = hasPattern ? fillExtrusionPatternGroup : fillExtrusionGroup;
    if (!shaderGroup) {
        removeAllDrawables();
        return;
    }

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const auto* optRenderData = getRenderDataForPass(tile, passes);
        if (!optRenderData || !optRenderData->bucket) {
            removeTile(passes, tileID);
            continue;
        }

        const auto& renderData = *optRenderData;
        const auto& bucket = static_cast<const FillExtrusionBucket&>(*renderData.bucket);

        vertexAttrs.clear();

        const auto vertexCount = bucket.vertices.elements();
        constexpr auto vertexSize = sizeof(FillExtrusionLayoutVertex::a1);

        const auto defPattern = mbgl::Faded<expression::Image>{"", ""};
        const auto fillPatternValue = evaluated.get<FillExtrusionPattern>().constantOr(defPattern);
        const auto patternPosA = tile.getPattern(fillPatternValue.from.id());
        const auto patternPosB = tile.getPattern(fillPatternValue.to.id());

        const auto& binders = bucket.paintPropertyBinders.at(getID());
        if (hasPattern) {
            binders.setPatternParameters(patternPosA, patternPosB, crossfade);
        }

        const FillExtrusionInterpolateUBO interpUBO = {
            /* .base_t = */ std::get<0>(binders.get<FillExtrusionBase>()->interpolationFactor(zoom)),
            /* .height_t = */ std::get<0>(binders.get<FillExtrusionHeight>()->interpolationFactor(zoom)),
            /* .color_t = */ std::get<0>(binders.get<FillExtrusionColor>()->interpolationFactor(zoom)),
            /* .pattern_from_t = */ std::get<0>(binders.get<FillExtrusionPattern>()->interpolationFactor(zoom)),
            /* .pattern_to_t = */ std::get<0>(binders.get<FillExtrusionPattern>()->interpolationFactor(zoom)),
            /* .pad = */ 0, 0, 0
        };

        const FillExtrusionDrawableTilePropsUBO tilePropsUBO = {
            /* pattern_from = */ patternPosA ? util::cast<float>(patternPosA->tlbr()) : std::array<float, 4>{0},
            /* pattern_to = */ patternPosB ? util::cast<float>(patternPosB->tlbr()) : std::array<float, 4>{0},
        };

        // If we already have drawables for this tile, update them.
        if (tileLayerGroup->getDrawableCount(passes, tileID) > 0) {
            // Just update the drawables we already created
            tileLayerGroup->observeDrawables(passes, tileID, [&](gfx::Drawable& drawable) {
                auto& uniforms = drawable.mutableUniformBuffers();
                uniforms.createOrUpdate(FillExtrusionLayerTweaker::FillExtrusionTilePropsUBOName, &tilePropsUBO, context);
                uniforms.createOrUpdate(FillExtrusionLayerTweaker::FillExtrusionInterpolateUBOName, &interpUBO, context);
            });
            continue;
        }

        const auto uniformProps =
            vertexAttrs.readDataDrivenPaintProperties<FillExtrusionBase,
                                                      FillExtrusionColor,
                                                      FillExtrusionHeight,
                                                      FillExtrusionPattern>(
                binders, evaluated);

        const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                             shaderGroup->getOrCreateShader(context, uniformProps));
        if (!shader) {
            assert(false);
            continue;
        }

        // The non-pattern path in `render()` only uses two-pass rendering if there's translucency.
        // The pattern path always uses two passes.
        const auto doDepthPass = (!opaque || hasPattern);

        if (doDepthPass && !depthBuilder) {
            if (auto builder = context.createDrawableBuilder(layerPrefix + "depth")) {
                builder->setShader(shader);
                builder->setIs3D(true);
                builder->setEnableStencil(false);
                builder->setEnableColor(false);
                builder->setRenderPass(passes);
                builder->setCullFaceMode(gfx::CullFaceMode::backCCW());
                depthBuilder = std::move(builder);
            }
        }
        if (!colorBuilder) {
            if (auto builder = context.createDrawableBuilder(layerPrefix + "color")) {
                builder->setShader(shader);
                builder->setIs3D(true);
                builder->setEnableColor(true);
                builder->setRenderPass(passes);
                builder->setCullFaceMode(gfx::CullFaceMode::backCCW());
                colorBuilder = std::move(builder);
            }
        }

        if (hasPattern) {
            if (const auto& atlases = tile.getAtlasTextures()) {
                if (const auto texSamplerLocation = shader->getSamplerLocation("u_image")) {
                    colorBuilder->setTextureSource([=]() -> gfx::Drawable::Textures {
                        return {{*texSamplerLocation, atlases->icon}};
                    });
                }
            }
        }

        if (const auto& attr = vertexAttrs.getOrAdd(normAttribName)) {
            const auto count = bucket.vertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, util::cast<float>(bucket.vertices.at(i).a2));  // int16_t x4
            }
        }

        rawVertices.resize(vertexSize * vertexCount);
        for (std::size_t i = 0; i < vertexCount; ++i) {
            std::memcpy(&rawVertices[vertexSize * i], &bucket.vertices.at(i).a1, vertexSize);
        }

        colorBuilder->setEnableStencil(doDepthPass);
        if (doDepthPass) {
            auto vertexCopy = rawVertices;
            depthBuilder->setRawVertices(std::move(vertexCopy), vertexCount, gfx::AttributeDataType::Short2);
        }
        colorBuilder->setRawVertices(std::move(rawVertices), vertexCount, gfx::AttributeDataType::Short2);

        if (doDepthPass) {
            gfx::VertexAttributeArray attribCopy;
            attribCopy.copy(vertexAttrs);
            depthBuilder->setVertexAttributes(std::move(attribCopy));
        }
        colorBuilder->setVertexAttributes(std::move(vertexAttrs));

        const auto finish = [&](gfx::DrawableBuilder& builder) {
            builder.setSegments(gfx::Triangles(),
                                bucket.triangles.vector(),
                                bucket.triangleSegments.data(),
                                bucket.triangleSegments.size());

            builder.flush();

            for (auto& drawable : builder.clearDrawables()) {
                drawable->setTileID(tileID);
                
                auto& uniforms = drawable->mutableUniformBuffers();
                uniforms.createOrUpdate(FillExtrusionLayerTweaker::FillExtrusionTilePropsUBOName, &tilePropsUBO, context);
                uniforms.createOrUpdate(FillExtrusionLayerTweaker::FillExtrusionInterpolateUBOName, &interpUBO, context);
                
                tileLayerGroup->addDrawable(passes, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        };
        
        if (doDepthPass) {
            finish(*depthBuilder);
        }
        finish(*colorBuilder);
    }
}

#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
