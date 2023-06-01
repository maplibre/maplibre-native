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
#include <mbgl/renderer/layers/line_layer_tweaker.hpp>
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
#include <mbgl/shaders/shader_program_base.hpp>

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

void RenderLineLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    stats.tileDrawablesRemoved += tileLayerGroup->removeDrawables(renderPass, tileID).size();
}

void RenderLineLayer::update(gfx::ShaderRegistry& shaders,
                             gfx::Context& context,
                             const TransformState& /*state*/,
                             [[maybe_unused]] const RenderTree& renderTree,
                             [[maybe_unused]] UniqueChangeRequestVec& changes) {
    std::unique_lock<std::mutex> guard(mutex);

    if (!renderTiles || renderTiles->empty()) {
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
        tileLayerGroup->setLayerTweaker(std::make_shared<LineLayerTweaker>(evaluatedProperties));
        changes.emplace_back(std::make_unique<AddLayerGroupRequest>(tileLayerGroup, /*canReplace=*/true));
    }

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        // Has this tile dropped out of the cover set?
        if (const auto it = std::find_if(renderTiles->begin(),
                                         renderTiles->end(),
                                         [&drawable](const auto& renderTile) {
                                             return drawable->getTileID() == renderTile.get().getOverscaledTileID();
                                         });
            it == renderTiles->end()) {
            // remove it
            drawable.reset();
            ++stats.tileDrawablesRemoved;
        }
    });

    const auto renderPass{RenderPass::Translucent};
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        auto& tileDrawable = tileLayerGroup->getDrawable(renderPass, tileID);
        if (tileDrawable) {
            continue;
        }

        const LayerRenderData* renderData = getRenderDataForPass(tile, renderPass);
        if (!renderData) {
            removeTile(renderPass, tileID);
            continue;
        }

        auto& bucket = static_cast<LineBucket&>(*renderData->bucket);
        const auto& evaluated = getEvaluated<LineLayerProperties>(renderData->layerProperties);

        if (!evaluated.get<LineDasharray>().from.empty()) {
            // TODO: dash array line: LineSDFShader
        } else if (!unevaluated.get<LinePattern>().isUndefined()) {
            // TODO: pattern line: LinePatternShader
        } else if (!unevaluated.get<LineGradient>().getValue().isUndefined()) {
            // TODO: gradient line: LineGradientShader
        } else {
            // simple line
            std::unique_ptr<gfx::DrawableBuilder> builder{context.createDrawableBuilder("line")};
            if (!lineShader) {
                lineShader = context.getGenericShader(shaders, "LineShader");
            }

            builder->setShader(lineShader);
            builder->setRenderPass(renderPass);
            builder->setColorAttrMode(gfx::DrawableBuilder::ColorAttrMode::None);
            builder->setSubLayerIndex(0);
            builder->setDepthType((renderPass == RenderPass::Opaque) ? gfx::DepthMaskType::ReadWrite
                                                                     : gfx::DepthMaskType::ReadOnly);
            builder->setCullFaceMode(gfx::CullFaceMode::disabled());
            builder->setVertexAttrName("a_pos_normal");

            std::vector<std::array<int16_t, 2>> rawVerts;
            const auto buildVertices = [&rawVerts, &bucket]() {
                if (rawVerts.size() != bucket.vertices.vector().size()) {
                    rawVerts.resize(bucket.vertices.vector().size());
                    std::transform(bucket.vertices.vector().begin(),
                                   bucket.vertices.vector().end(),
                                   rawVerts.begin(),
                                   [](const auto& x) { return x.a1; });
                }
            };

            // vertices
            buildVertices();
            auto vertexCount = rawVerts.size();
            builder->addVertices(rawVerts, 0, vertexCount);

            // attributes
            {
                gfx::VertexAttributeArray vertexAttrs;
                if (auto& attr = vertexAttrs.getOrAdd("a_data")) {
                    size_t index{0};
                    for (const auto& vert : bucket.vertices.vector()) {
                        attr->set(index++, gfx::VertexAttribute::int4{vert.a2[0], vert.a2[1], vert.a2[2], vert.a2[3]});
                    }
                }

                // test whether the shader has the extra attributes
                if (lineShader->getVertexAttributes().size() > 2) {
                    const auto& paintPropertyBinders = bucket.paintPropertyBinders.at(getID());

                    if (auto& binder = paintPropertyBinders.get<LineColor>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = vertexAttrs.getOrAdd("a_color")) {
                            for (std::size_t i = 0; i < vertexCount; ++i) {
                                const auto& packedColor =
                                    static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 2>>*>(
                                        binder->getVertexValue(i < count ? i : 0))
                                        ->a1;
                                attr->set<gfx::VertexAttribute::float4>(
                                    i, {packedColor[0], packedColor[1], packedColor[0], packedColor[1]});
                            }
                        }
                    }
                    if (auto& binder = paintPropertyBinders.get<LineBlur>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = vertexAttrs.getOrAdd("a_blur")) {
                            for (std::size_t i = 0; i < vertexCount; ++i) {
                                const auto& blur =
                                    static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                        binder->getVertexValue(i < count ? i : 0))
                                        ->a1;
                                attr->set<gfx::VertexAttribute::float2>(i, {blur[0], blur[0]});
                            }
                        }
                    }
                    if (auto& binder = paintPropertyBinders.get<LineOpacity>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = vertexAttrs.getOrAdd("a_opacity")) {
                            for (std::size_t i = 0; i < vertexCount; ++i) {
                                const auto& opacity =
                                    static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                        binder->getVertexValue(i < count ? i : 0))
                                        ->a1;
                                attr->set<gfx::VertexAttribute::float2>(i, {opacity[0], opacity[0]});
                            }
                        }
                    }
                    if (auto& binder = paintPropertyBinders.get<LineGapWidth>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = vertexAttrs.getOrAdd("a_gapwidth")) {
                            for (std::size_t i = 0; i < vertexCount; ++i) {
                                const auto& gapwidth =
                                    static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                        binder->getVertexValue(i < count ? i : 0))
                                        ->a1;
                                attr->set<gfx::VertexAttribute::float2>(i, {gapwidth[0], gapwidth[0]});
                            }
                        }
                    }
                    if (auto& binder = paintPropertyBinders.get<LineOffset>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = vertexAttrs.getOrAdd("a_offset")) {
                            for (std::size_t i = 0; i < vertexCount; ++i) {
                                const auto& offset =
                                    static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                        binder->getVertexValue(i < count ? i : 0))
                                        ->a1;
                                attr->set<gfx::VertexAttribute::float2>(i, {offset[0], offset[0]});
                            }
                        }
                    }
                    if (auto& binder = paintPropertyBinders.get<LineOffset>()) {
                        const auto count = binder->getVertexCount();
                        if (auto& attr = vertexAttrs.getOrAdd("a_width")) {
                            for (std::size_t i = 0; i < vertexCount; ++i) {
                                const auto& width =
                                    static_cast<const gfx::detail::VertexType<gfx::AttributeType<float, 1>>*>(
                                        binder->getVertexValue(i < count ? i : 0))
                                        ->a1;
                                attr->set<gfx::VertexAttribute::float2>(i, {width[0], width[0]});
                            }
                        }
                    }
                }

                builder->setVertexAttributes(std::move(vertexAttrs));
            }

            // segments
            builder->setSegments(gfx::Triangles(),
                                 bucket.triangles.vector(),
                                 reinterpret_cast<const std::vector<Segment<void>>&>(bucket.segments));
            //                using namespace std::string_literals;
            //                Log::Warning(Event::General,
            //                             "SEG: "s + util::toString(tileID) +
            //                             " vertex offset: " + std::to_string(seg.vertexOffset) +
            //                             " vertex len: " + std::to_string(seg.vertexLength) +
            //                             " index offset: " + std::to_string(seg.indexOffset) +
            //                             " index len: " + std::to_string(seg.indexLength) +
            //                             " max(index): " + std::to_string(builder->maxIndex())
            //                             );

            // finish
            builder->flush();
            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                tileLayerGroup->addDrawable(renderPass, tileID, std::move(drawable));
                ++stats.tileDrawablesAdded;
            }
        }
    }
}

} // namespace mbgl
