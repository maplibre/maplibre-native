#include <mbgl/geometry/feature_index.hpp>
#include <mbgl/geometry/line_atlas.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/programs/line_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/renderer/buckets/line_bucket.hpp>
#include <mbgl/renderer/image_manager.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/render_line_layer.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/style/expression/image.hpp>
#include <mbgl/style/layers/line_layer_impl.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/intersection_tests.hpp>
#include <mbgl/util/math.hpp>

namespace mbgl {

using namespace style;

namespace {

inline const LineLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == LineLayer::Impl::staticTypeInfo());
    return static_cast<const LineLayer::Impl&>(*impl);
}

} // namespace

RenderLineLayer::RenderLineLayer(Immutable<style::LineLayer::Impl> _impl)
    : RenderLayer(makeMutable<LineLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()),
      colorRamp({256, 1}) {}

RenderLineLayer::~RenderLineLayer() = default;

void RenderLineLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    updateColorRamp();
}

void RenderLineLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<LineLayerProperties>(staticImmutableCast<LineLayer::Impl>(baseImpl),
                                                       parameters.getCrossfadeParameters(),
                                                       unevaluated.evaluate(parameters));
    auto& evaluated = properties->evaluated;

    passes = (evaluated.get<style::LineOpacity>().constantOr(1.0) > 0 &&
              evaluated.get<style::LineColor>().constantOr(Color::black()).a > 0 &&
              evaluated.get<style::LineWidth>().constantOr(1.0) > 0)
                 ? RenderPass::Translucent
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);
}

bool RenderLineLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderLineLayer::hasCrossfade() const {
    return getCrossfade<LineLayerProperties>(evaluatedProperties).t != 1;
}

void RenderLineLayer::prepare(const LayerPrepareParameters& params) {
    RenderLayer::prepare(params);
    for (const RenderTile& tile : *renderTiles) {
        const LayerRenderData* renderData = tile.getLayerRenderData(*baseImpl);
        if (!renderData) continue;

        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);
        if (evaluated.get<LineDasharray>().from.empty()) continue;

        auto& bucket = static_cast<LineBucket&>(*renderData->bucket);
        const LinePatternCap cap = bucket.layout.get<LineCap>() == LineCapType::Round ? LinePatternCap::Round
                                                                                      : LinePatternCap::Square;
        // Ensures that the dash data gets added to the atlas.
        params.lineAtlas.getDashPatternTexture(
            evaluated.get<LineDasharray>().from, evaluated.get<LineDasharray>().to, cap);
    }
}

void RenderLineLayer::upload(gfx::UploadPass& uploadPass) {
    if (!unevaluated.get<LineGradient>().getValue().isUndefined() && !colorRampTexture) {
        colorRampTexture = uploadPass.createTexture(colorRamp);
    }
}

void RenderLineLayer::render(PaintParameters& parameters) {
    return;
    assert(renderTiles);
    if (parameters.pass == RenderPass::Opaque) {
        return;
    }

    if (!parameters.shaders.populate(lineProgram)) return;
    if (!parameters.shaders.populate(lineGradientProgram)) return;
    if (!parameters.shaders.populate(lineSDFProgram)) return;
    if (!parameters.shaders.populate(linePatternProgram)) return;

    parameters.renderTileClippingMasks(renderTiles);

    for (const RenderTile& tile : *renderTiles) {
        const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
        if (!renderData) {
            continue;
        }
        auto& bucket = static_cast<LineBucket&>(*renderData->bucket);
        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);
        const auto& crossfade = getCrossfade<LineLayerProperties>(renderData->layerProperties);

        auto draw = [&](auto& programInstance,
                        auto&& uniformValues,
                        const std::optional<ImagePosition>& patternPositionA,
                        const std::optional<ImagePosition>& patternPositionB,
                        auto&& textureBindings) {
            const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

            paintPropertyBinders.setPatternParameters(patternPositionA, patternPositionB, crossfade);

            const auto allUniformValues = programInstance.computeAllUniformValues(
                std::forward<decltype(uniformValues)>(uniformValues),
                paintPropertyBinders,
                evaluated,
                static_cast<float>(parameters.state.getZoom()));
            const auto allAttributeBindings = programInstance.computeAllAttributeBindings(
                *bucket.vertexBuffer, paintPropertyBinders, evaluated);

            checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));

            programInstance.draw(parameters.context,
                                 *parameters.renderPass,
                                 gfx::Triangles(),
                                 parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                                 parameters.stencilModeForClipping(tile.id),
                                 parameters.colorModeForRenderPass(),
                                 gfx::CullFaceMode::disabled(),
                                 *bucket.indexBuffer,
                                 bucket.segments,
                                 allUniformValues,
                                 allAttributeBindings,
                                 std::forward<decltype(textureBindings)>(textureBindings),
                                 getID());
        };

        if (!evaluated.get<LineDasharray>().from.empty()) {
            const LinePatternCap cap = bucket.layout.get<LineCap>() == LineCapType::Round ? LinePatternCap::Round
                                                                                          : LinePatternCap::Square;
            const auto& dashPatternTexture = parameters.lineAtlas.getDashPatternTexture(
                evaluated.get<LineDasharray>().from, evaluated.get<LineDasharray>().to, cap);

            draw(*lineSDFProgram,
                 LineSDFProgram::layoutUniformValues(evaluated,
                                                     parameters.pixelRatio,
                                                     tile,
                                                     parameters.state,
                                                     parameters.pixelsToGLUnits,
                                                     dashPatternTexture.getFrom(),
                                                     dashPatternTexture.getTo(),
                                                     crossfade,
                                                     static_cast<float>(dashPatternTexture.getSize().width)),
                 {},
                 {},
                 LineSDFProgram::TextureBindings{
                     dashPatternTexture.textureBinding(),
                 });

        } else if (!unevaluated.get<LinePattern>().isUndefined()) {
            const auto& linePatternValue = evaluated.get<LinePattern>().constantOr(Faded<expression::Image>{"", ""});
            const Size& texsize = tile.getIconAtlasTexture().size;

            std::optional<ImagePosition> posA = tile.getPattern(linePatternValue.from.id());
            std::optional<ImagePosition> posB = tile.getPattern(linePatternValue.to.id());

            draw(*linePatternProgram,
                 LinePatternProgram::layoutUniformValues(evaluated,
                                                         tile,
                                                         parameters.state,
                                                         parameters.pixelsToGLUnits,
                                                         parameters.pixelRatio,
                                                         texsize,
                                                         crossfade),
                 posA,
                 posB,
                 LinePatternProgram::TextureBindings{
                     textures::image::Value{tile.getIconAtlasTexture().getResource(), gfx::TextureFilterType::Linear},
                 });
        } else if (!unevaluated.get<LineGradient>().getValue().isUndefined()) {
            assert(colorRampTexture);

            draw(*lineGradientProgram,
                 LineGradientProgram::layoutUniformValues(
                     evaluated, tile, parameters.state, parameters.pixelsToGLUnits, parameters.pixelRatio),
                 {},
                 {},
                 LineGradientProgram::TextureBindings{
                     textures::image::Value{colorRampTexture->getResource(), gfx::TextureFilterType::Linear},
                 });
        } else {
            draw(*lineProgram,
                 LineProgram::layoutUniformValues(
                     evaluated, tile, parameters.state, parameters.pixelsToGLUnits, parameters.pixelRatio),
                 {},
                 {},
                 LineProgram::TextureBindings{});
        }
    }
}

namespace {

GeometryCollection offsetLine(const GeometryCollection& rings, double offset) {
    assert(offset != 0.0f);
    assert(!rings.empty());

    GeometryCollection newRings;
    newRings.reserve(rings.size());

    const Point<double> zero(0, 0);
    for (const auto& ring : rings) {
        newRings.emplace_back();
        auto& newRing = newRings.back();
        newRing.reserve(ring.size());

        for (auto i = ring.begin(); i != ring.end(); ++i) {
            auto& p = *i;

            Point<double> aToB = i == ring.begin() ? zero : util::perp(util::unit(convertPoint<double>(p - *(i - 1))));
            Point<double> bToC = i + 1 == ring.end() ? zero
                                                     : util::perp(util::unit(convertPoint<double>(*(i + 1) - p)));
            Point<double> extrude = util::unit(aToB + bToC);

            const double cosHalfAngle = extrude.x * bToC.x + extrude.y * bToC.y;
            extrude *= (cosHalfAngle != 0) ? (1.0 / cosHalfAngle) : 0;

            newRing.emplace_back(convertPoint<int16_t>(extrude * offset) + p);
        }
    }

    return newRings;
}

} // namespace

bool RenderLineLayer::queryIntersectsFeature(const GeometryCoordinates& queryGeometry,
                                             const GeometryTileFeature& feature,
                                             const float zoom,
                                             const TransformState& transformState,
                                             const float pixelsToTileUnits,
                                             const mat4&,
                                             const FeatureState& featureState) const {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    // Translate query geometry
    auto translatedQueryGeometry = FeatureIndex::translateQueryGeometry(queryGeometry,
                                                                        evaluated.get<style::LineTranslate>(),
                                                                        evaluated.get<style::LineTranslateAnchor>(),
                                                                        static_cast<float>(transformState.getBearing()),
                                                                        pixelsToTileUnits);

    // Evaluate function
    auto offset = evaluated.get<style::LineOffset>().evaluate(
                      feature, zoom, featureState, style::LineOffset::defaultValue()) *
                  pixelsToTileUnits;
    // Test intersection
    const auto halfWidth = static_cast<float>(getLineWidth(feature, zoom, featureState) / 2.0 * pixelsToTileUnits);

    // Apply offset to geometry
    if (offset != 0.0f && !feature.getGeometries().empty()) {
        return util::polygonIntersectsBufferedMultiLine(
            translatedQueryGeometry.value_or(queryGeometry), offsetLine(feature.getGeometries(), offset), halfWidth);
    }

    return util::polygonIntersectsBufferedMultiLine(
        translatedQueryGeometry.value_or(queryGeometry), feature.getGeometries(), halfWidth);
}

void RenderLineLayer::updateColorRamp() {
    auto colorValue = unevaluated.get<LineGradient>().getValue();
    if (colorValue.isUndefined()) {
        return;
    }

    const auto length = colorRamp.bytes();

    for (uint32_t i = 0; i < length; i += 4) {
        const auto color = colorValue.evaluate(static_cast<double>(i) / length);
        colorRamp.data[i] = static_cast<uint8_t>(std::floor(color.r * 255.f));
        colorRamp.data[i + 1] = static_cast<uint8_t>(std::floor(color.g * 255.f));
        colorRamp.data[i + 2] = static_cast<uint8_t>(std::floor(color.b * 255.f));
        colorRamp.data[i + 3] = static_cast<uint8_t>(std::floor(color.a * 255.f));
    }

    if (colorRampTexture) {
        colorRampTexture = std::nullopt;
    }
}

float RenderLineLayer::getLineWidth(const GeometryTileFeature& feature,
                                    const float zoom,
                                    const FeatureState& featureState) const {
    const auto& evaluated = static_cast<const LineLayerProperties&>(*evaluatedProperties).evaluated;
    float lineWidth = evaluated.get<style::LineWidth>().evaluate(
        feature, zoom, featureState, style::LineWidth::defaultValue());
    float gapWidth = evaluated.get<style::LineGapWidth>().evaluate(
        feature, zoom, featureState, style::LineGapWidth::defaultValue());
    if (gapWidth) {
        return gapWidth + 2 * lineWidth;
    } else {
        return lineWidth;
    }
}

void RenderLineLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    // Remove everything
    if (tileLayerGroup) {
        changes.emplace_back(std::make_unique<RemoveLayerGroupRequest>(tileLayerGroup->getLayerIndex()));
        tileLayerGroup.reset();
    }
}

void RenderLineLayer::update(const int32_t layerIndex,
                             gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& state,
                             UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);
    std::unique_ptr<gfx::DrawableBuilder> builderLine{context.createDrawableBuilder("line")};
    builderLine->setShader(context.getGenericShader(shaders, "LineShader"));

    std::unique_ptr<gfx::DrawableBuilder> builderGradientLine{context.createDrawableBuilder("lineGradient")};
    builderGradientLine->setShader(context.getGenericShader(shaders, "LineGradientShader"));

    std::unique_ptr<gfx::DrawableBuilder> builderSDFLine{context.createDrawableBuilder("lineSDF")};
    builderLine->setShader(context.getGenericShader(shaders, "LineSDFShader"));

    std::unique_ptr<gfx::DrawableBuilder> builderPatternLine{context.createDrawableBuilder("linePattern")};
    builderLine->setShader(context.getGenericShader(shaders, "LinePatternShader"));

    if (!builderLine || !builderGradientLine || !builderSDFLine || !builderPatternLine || !renderTiles ||
        renderTiles->empty()) {
        if (tileLayerGroup) {
            stats.tileDrawablesRemoved += tileLayerGroup->getDrawableCount();
            tileLayerGroup->clearDrawables();
        }
        return;
    }
    // Set up a layer group
    if (!tileLayerGroup) {
        tileLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64);
        if (!tileLayerGroup) {
            return;
        }
        changes.emplace_back(std::make_unique<AddLayerGroupRequest>(tileLayerGroup, /*canReplace=*/true));
    }
    std::unordered_set<OverscaledTileID> newTileIDs(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(newTileIDs, newTileIDs.begin()),
                   [](const auto& renderTile) -> OverscaledTileID { return renderTile.get().getOverscaledTileID(); });
    std::vector<gfx::DrawablePtr> newTiles;

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        // Has this tile dropped out of the cover set?
        const auto tileID = drawable->getTileID();
        if (tileID && newTileIDs.find(*tileID) == newTileIDs.end()) {
            // remove it
            drawable.reset();
            ++stats.tileDrawablesRemoved;
        }
    });

    //    parameters.renderTileClippingMasks(renderTiles);
    const auto renderPass{RenderPass::Translucent};
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        auto& tileDrawable = tileLayerGroup->getDrawable(renderPass, tileID);

        const auto removeTile = [&]() {
            if (tileDrawable) {
                tileLayerGroup->removeDrawable(renderPass, tileID);
                ++stats.tileDrawablesRemoved;
            }
        };
        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile();
            continue;
        }

        auto& bucket = static_cast<LineBucket&>(*renderData->bucket);
        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);

        if (!evaluated.get<LineDasharray>().from.empty()) {
            // TODO: dash array line
            builderSDFLine->setRenderPass(renderPass);
        } else if (!unevaluated.get<LinePattern>().isUndefined()) {
            // TODO: pattern line
            builderPatternLine->setRenderPass(renderPass);
        } else if (!unevaluated.get<LineGradient>().getValue().isUndefined()) {
            // TODO: gradient line
            builderGradientLine->setRenderPass(renderPass);
        } else {
            // simple line
            auto& builder = builderLine;
            builder->setRenderPass(renderPass);
            builder->addTweaker(context.createDrawableTweaker());
            builder->setColorMode(gfx::DrawableBuilder::ColorMode::None);
            builder->setDepthType(gfx::DepthMaskType::ReadWrite);
            builder->setLayerIndex(layerIndex);
            builder->setVertexAttrName("a_pos_normal");

            // vertices
            std::vector<std::array<int16_t, 2>> rawVerts(bucket.vertices.vector().size());
            std::transform(
                bucket.vertices.vector().begin(), bucket.vertices.vector().end(), rawVerts.begin(), [](const auto& x) {
                    return x.a1;
                });
            builder->addVertices(rawVerts, 0, rawVerts.size());

            // attributes
            gfx::VertexAttributeArray vertexAttrs;
            if (auto& attr = vertexAttrs.getOrAdd(
                    "a_data", 1, gfx::AttributeDataType::Int4, 1, bucket.vertices.elements())) {
                size_t index{0};
                for (const auto& vert : bucket.vertices.vector()) {
                    attr->set(index++, gfx::VertexAttribute::int4{vert.a2[0], vert.a2[1], vert.a2[2], vert.a2[3]});
                }
            }
            builder->setVertexAttributes(std::move(vertexAttrs));

            // indexes
            for (const auto& seg : bucket.segments) {
                builder->addTriangles(bucket.triangles.vector(), seg.indexOffset, seg.indexLength);
            }

            builder->setMatrix(/* tile.translatedMatrix(properties.get<LineTranslate>(),
                                  properties.get<LineTranslateAnchor>(), state) */
                               matrix::identity4());
            builder->flush();

            // uniforms
            gfx::UniformBufferPtr lineLayerUBO1;
            {
                LineLayerUBO1 lineLayerUBO1Data;
                lineLayerUBO1Data.ratio = 1.0f / tile.id.pixelsToTileUnits(1.0f, static_cast<float>(state.getZoom()));
                lineLayerUBO1Data.device_pixel_ratio = 2; // parameters.pixelRatio;
                lineLayerUBO1Data.units_to_pixels = {
                    {0.00243902439f, -0.00169491523f}}; // {{1.0f / parameters.pixelsToGLUnits[0], 1.0f /
                                                        // parameters.pixelsToGLUnits[1]}};
                lineLayerUBO1 = context.createUniformBuffer(&lineLayerUBO1Data, sizeof(lineLayerUBO1Data));
            }

            auto newDrawables = builder->clearDrawables();
            if (!newDrawables.empty()) {
                auto& drawable = newDrawables.front();
                drawable->setTileID(tileID);
                drawable->mutableUniformBuffers().addOrReplace("LineLayerUBO1", lineLayerUBO1);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.tileDrawablesAdded;
                Log::Warning(Event::General,
                             "Adding Line drawable for " + util::toString(tileID) + " total " +
                                 std::to_string(stats.tileDrawablesAdded + 1) + " current " +
                                 std::to_string(tileLayerGroup->getDrawableCount()));
            }
        }
    }
}

} // namespace mbgl
