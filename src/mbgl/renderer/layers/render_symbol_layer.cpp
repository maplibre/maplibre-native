#include <mbgl/renderer/layers/render_symbol_layer.hpp>

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/layout/symbol_layout.hpp>
#include <mbgl/programs/collision_box_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/symbol_program.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/property_evaluation_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/text/glyph_atlas.hpp>
#include <mbgl/text/shaping.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>

#if MLN_DRAWABLE_RENDERER
#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/symbol_drawable_data.hpp>
#include <mbgl/gfx/collision_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/gfx/collision_drawable_data.hpp>
#include <mbgl/renderer/layers/collision_layer_tweaker.hpp>
#endif // MLN_DRAWABLE_RENDERER

#include <cmath>
#include <set>

namespace mbgl {

using namespace style;
namespace {

#if MLN_DRAWABLE_RENDERER

constexpr std::string_view SymbolIconShaderName = "SymbolIconShader";
constexpr std::string_view SymbolSDFIconShaderName = "SymbolSDFTextShader";
constexpr std::string_view SymbolSDFTextShaderName = "SymbolSDFIconShader";
constexpr std::string_view SymbolTextAndIconShaderName = "SymbolTextAndIconShader";
constexpr std::string_view CollisionBoxShaderName = "CollisionBoxShader";
constexpr std::string_view CollisionCircleShaderName = "CollisionCircleShader";

#endif // MLN_DRAWABLE_RENDERER

style::SymbolPropertyValues iconPropertyValues(const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_,
                                               const style::SymbolLayoutProperties::PossiblyEvaluated& layout_) {
    return style::SymbolPropertyValues{layout_.get<style::IconPitchAlignment>(),
                                       layout_.get<style::IconRotationAlignment>(),
                                       layout_.get<style::IconKeepUpright>(),
                                       evaluated_.get<style::IconTranslate>(),
                                       evaluated_.get<style::IconTranslateAnchor>(),
                                       evaluated_.get<style::IconHaloColor>().constantOr(Color::black()).a > 0 &&
                                           evaluated_.get<style::IconHaloWidth>().constantOr(1) != 0,
                                       evaluated_.get<style::IconColor>().constantOr(Color::black()).a > 0};
}

style::SymbolPropertyValues textPropertyValues(const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_,
                                               const style::SymbolLayoutProperties::PossiblyEvaluated& layout_) {
    return style::SymbolPropertyValues{layout_.get<style::TextPitchAlignment>(),
                                       layout_.get<style::TextRotationAlignment>(),
                                       layout_.get<style::TextKeepUpright>(),
                                       evaluated_.get<style::TextTranslate>(),
                                       evaluated_.get<style::TextTranslateAnchor>(),
                                       evaluated_.get<style::TextHaloColor>().constantOr(Color::black()).a > 0 &&
                                           evaluated_.get<style::TextHaloWidth>().constantOr(1) != 0,
                                       evaluated_.get<style::TextColor>().constantOr(Color::black()).a > 0};
}

using SegmentWrapper = std::reference_wrapper<const Segment<SymbolTextAttributes>>;
using SegmentVectorWrapper = std::reference_wrapper<const SegmentVector<SymbolTextAttributes>>;
using SegmentsWrapper = variant<SegmentWrapper, SegmentVectorWrapper>;

struct RenderableSegment {
    RenderableSegment(SegmentWrapper segment_,
                      const RenderTile& tile_,
                      const LayerRenderData& renderData_,
                      const SymbolBucket::PaintProperties& bucketPaintProperties_,
                      float sortKey_,
                      const SymbolType type_,
                      const uint8_t overscaledZ_ = 0)
        : segment(segment_),
          tile(tile_),
          renderData(renderData_),
          bucketPaintProperties(bucketPaintProperties_),
          sortKey(sortKey_),
          type(type_),
          overscaledZ(overscaledZ_) {}

    SegmentWrapper segment;
    const RenderTile& tile;
    const LayerRenderData& renderData;
    const SymbolBucket::PaintProperties& bucketPaintProperties;
    float sortKey;
    SymbolType type;
    uint8_t overscaledZ;

    friend bool operator<(const RenderableSegment& lhs, const RenderableSegment& rhs) {
        // Sort renderable segments by a sort key.
        if (lhs.sortKey < rhs.sortKey) {
            return true;
        }

        // In cases when sort key is the same, sort by the type of a segment
        // (text over icons), and for segments of the same type with the same
        // sort key, sort by a tile id.
        if (lhs.sortKey == rhs.sortKey) {
            if (lhs.type != SymbolType::Text && rhs.type == SymbolType::Text) {
                return true;
            }

            if (lhs.type == rhs.type) {
                return lhs.tile.id < rhs.tile.id;
            }
        }

        return false;
    }
};

#if MLN_LEGACY_RENDERER

template <typename DrawFn>
void drawIcon(const RenderSymbolLayer::Programs& programs,
              const DrawFn& draw,
              const RenderTile& tile,
              const LayerRenderData& renderData,
              SegmentsWrapper iconSegments,
              const SymbolBucket::PaintProperties& bucketPaintProperties,
              const PaintParameters& parameters,
              const bool sdfIcons) {
    auto& bucket = static_cast<SymbolBucket&>(*renderData.bucket);
    const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData.layerProperties);
    const auto& layout = *bucket.layout;
    auto values = iconPropertyValues(evaluated, layout);
    const auto& paintPropertyValues = RenderSymbolLayer::iconPaintProperties(evaluated);

    const bool alongLine = layout.get<SymbolPlacement>() != SymbolPlacementType::Point &&
                           layout.get<IconRotationAlignment>() == AlignmentType::Map;

    const bool iconScaled = layout.get<IconSize>().constantOr(1.0) != 1.0 || bucket.iconsNeedLinear;
    const bool iconTransformed = values.rotationAlignment == AlignmentType::Map || parameters.state.getPitch() != 0;

    const bool linear = sdfIcons || parameters.state.isChanging() || iconScaled || iconTransformed;
    const auto filterType = linear ? gfx::TextureFilterType::Linear : gfx::TextureFilterType::Nearest;
    const gfx::TextureBinding textureBinding = tile.getIconAtlasTextureBinding(filterType);

    const Size& iconSize = tile.getIconAtlasTexture()->getSize();
    const bool variablePlacedIcon = bucket.hasVariablePlacement && layout.get<IconTextFit>() != IconTextFitType::None;

    if (sdfIcons) {
        if (values.hasHalo) {
            draw(*programs.symbolSDFIconProgram,
                 SymbolSDFIconProgram::layoutUniformValues(/* isText = */ false,
                                                           variablePlacedIcon,
                                                           values,
                                                           iconSize,
                                                           parameters.pixelsToGLUnits,
                                                           parameters.pixelRatio,
                                                           alongLine,
                                                           tile,
                                                           parameters.state,
                                                           parameters.symbolFadeChange,
                                                           SymbolSDFPart::Halo),
                 bucket.sdfIcon,
                 iconSegments,
                 bucket.iconSizeBinder,
                 bucketPaintProperties.iconBinders,
                 paintPropertyValues,
                 SymbolSDFIconProgram::TextureBindings{textureBinding},
                 "halo");
        }

        if (values.hasFill) {
            draw(*programs.symbolSDFIconProgram,
                 SymbolSDFIconProgram::layoutUniformValues(/* isText = */ false,
                                                           variablePlacedIcon,
                                                           values,
                                                           iconSize,
                                                           parameters.pixelsToGLUnits,
                                                           parameters.pixelRatio,
                                                           alongLine,
                                                           tile,
                                                           parameters.state,
                                                           parameters.symbolFadeChange,
                                                           SymbolSDFPart::Fill),
                 bucket.sdfIcon,
                 iconSegments,
                 bucket.iconSizeBinder,
                 bucketPaintProperties.iconBinders,
                 paintPropertyValues,
                 SymbolSDFIconProgram::TextureBindings{textureBinding},
                 "fill");
        }
    } else {
        draw(*programs.symbolIconProgram,
             SymbolIconProgram::layoutUniformValues(/* isText = */ false,
                                                    variablePlacedIcon,
                                                    values,
                                                    iconSize,
                                                    parameters.pixelsToGLUnits,
                                                    alongLine,
                                                    tile,
                                                    parameters.state,
                                                    parameters.symbolFadeChange),
             bucket.icon,
             iconSegments,
             bucket.iconSizeBinder,
             bucketPaintProperties.iconBinders,
             paintPropertyValues,
             SymbolIconProgram::TextureBindings{textureBinding},
             "icon");
    }
}

template <typename DrawFn>
void drawText(const RenderSymbolLayer::Programs& programs,
              const DrawFn& draw,
              const RenderTile& tile,
              const LayerRenderData& renderData,
              SegmentsWrapper textSegments,
              const SymbolBucket::PaintProperties& bucketPaintProperties,
              const PaintParameters& parameters) {
    const auto& bucket = static_cast<SymbolBucket&>(*renderData.bucket);
    const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData.layerProperties);
    const auto& layout = *bucket.layout;

    const auto values = textPropertyValues(evaluated, layout);
    const auto& paintPropertyValues = RenderSymbolLayer::textPaintProperties(evaluated);

    const bool alongLine = layout.get<SymbolPlacement>() != SymbolPlacementType::Point &&
                           values.rotationAlignment == AlignmentType::Map;

    const Size& glyphTexSize = tile.getGlyphAtlasTexture()->getSize();
    const gfx::TextureBinding glyphTextureBinding = tile.getGlyphAtlasTextureBinding(gfx::TextureFilterType::Linear);

    const auto drawGlyphs = [&](auto& program, const auto& uniforms, const auto& textures, SymbolSDFPart part) {
        draw(program,
             uniforms,
             bucket.text,
             textSegments,
             bucket.textSizeBinder,
             bucketPaintProperties.textBinders,
             paintPropertyValues,
             textures,
             (part == SymbolSDFPart::Halo) ? "halo" : "fill");
    };

    if (bucket.iconsInText) {
        const ZoomEvaluatedSize partiallyEvaluatedTextSize = bucket.textSizeBinder->evaluateForZoom(
            static_cast<float>(parameters.state.getZoom()));
        const bool transformed = values.rotationAlignment == AlignmentType::Map || parameters.state.getPitch() != 0;
        const Size& iconTexSize = tile.getIconAtlasTexture()->getSize();
        const bool linear = parameters.state.isChanging() || transformed || !partiallyEvaluatedTextSize.isZoomConstant;
        const auto filterType = linear ? gfx::TextureFilterType::Linear : gfx::TextureFilterType::Nearest;
        const gfx::TextureBinding iconTextureBinding = tile.getIconAtlasTextureBinding(filterType);
        if (values.hasHalo) {
            drawGlyphs(*programs.symbolTextAndIconProgram,
                       SymbolTextAndIconProgram::layoutUniformValues(bucket.hasVariablePlacement,
                                                                     values,
                                                                     glyphTexSize,
                                                                     iconTexSize,
                                                                     parameters.pixelsToGLUnits,
                                                                     parameters.pixelRatio,
                                                                     alongLine,
                                                                     tile,
                                                                     parameters.state,
                                                                     parameters.symbolFadeChange,
                                                                     SymbolSDFPart::Halo),
                       SymbolTextAndIconProgram::TextureBindings{glyphTextureBinding, iconTextureBinding},
                       SymbolSDFPart::Halo);
        }

        if (values.hasFill) {
            drawGlyphs(*programs.symbolTextAndIconProgram,
                       SymbolTextAndIconProgram::layoutUniformValues(bucket.hasVariablePlacement,
                                                                     values,
                                                                     glyphTexSize,
                                                                     iconTexSize,
                                                                     parameters.pixelsToGLUnits,
                                                                     parameters.pixelRatio,
                                                                     alongLine,
                                                                     tile,
                                                                     parameters.state,
                                                                     parameters.symbolFadeChange,
                                                                     SymbolSDFPart::Fill),
                       SymbolTextAndIconProgram::TextureBindings{glyphTextureBinding, iconTextureBinding},
                       SymbolSDFPart::Fill);
        }
    } else {
        if (values.hasHalo) {
            drawGlyphs(*programs.symbolSDFTextProgram,
                       SymbolSDFTextProgram::layoutUniformValues(/* isText = */ true,
                                                                 bucket.hasVariablePlacement,
                                                                 values,
                                                                 glyphTexSize,
                                                                 parameters.pixelsToGLUnits,
                                                                 parameters.pixelRatio,
                                                                 alongLine,
                                                                 tile,
                                                                 parameters.state,
                                                                 parameters.symbolFadeChange,
                                                                 SymbolSDFPart::Halo),
                       SymbolSDFTextProgram::TextureBindings{glyphTextureBinding},
                       SymbolSDFPart::Halo);
        }

        if (values.hasFill) {
            drawGlyphs(*programs.symbolSDFTextProgram,
                       SymbolSDFTextProgram::layoutUniformValues(/* isText = */ true,
                                                                 bucket.hasVariablePlacement,
                                                                 values,
                                                                 glyphTexSize,
                                                                 parameters.pixelsToGLUnits,
                                                                 parameters.pixelRatio,
                                                                 alongLine,
                                                                 tile,
                                                                 parameters.state,
                                                                 parameters.symbolFadeChange,
                                                                 SymbolSDFPart::Fill),
                       SymbolSDFTextProgram::TextureBindings{glyphTextureBinding},
                       SymbolSDFPart::Fill);
        }
    }
}

#endif // MLN_LEGACY_RENDERER

inline const SymbolLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == SymbolLayer::Impl::staticTypeInfo());
    return static_cast<const SymbolLayer::Impl&>(*impl);
}

} // namespace

RenderSymbolLayer::RenderSymbolLayer(Immutable<style::SymbolLayer::Impl> _impl)
    : RenderLayer(makeMutable<SymbolLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {}

RenderSymbolLayer::~RenderSymbolLayer() = default;

void RenderSymbolLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    hasFormatSectionOverrides = SymbolLayerPaintPropertyOverrides::hasOverrides(
        impl_cast(baseImpl).layout.get<TextField>());
}

void RenderSymbolLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    auto properties = makeMutable<SymbolLayerProperties>(staticImmutableCast<SymbolLayer::Impl>(baseImpl),
                                                         unevaluated.evaluate(parameters));
    auto& evaluated = properties->evaluated;
    const auto& layout = impl_cast(baseImpl).layout;

    if (hasFormatSectionOverrides) {
        SymbolLayerPaintPropertyOverrides::setOverrides(layout, evaluated);
    }

    auto hasIconOpacity = evaluated.get<style::IconColor>().constantOr(Color::black()).a > 0 ||
                          evaluated.get<style::IconHaloColor>().constantOr(Color::black()).a > 0;
    auto hasTextOpacity = evaluated.get<style::TextColor>().constantOr(Color::black()).a > 0 ||
                          evaluated.get<style::TextHaloColor>().constantOr(Color::black()).a > 0;

    passes = ((evaluated.get<style::IconOpacity>().constantOr(1) > 0 && hasIconOpacity && iconSize > 0) ||
              (evaluated.get<style::TextOpacity>().constantOr(1) > 0 && hasTextOpacity && textSize > 0))
                 ? RenderPass::Translucent
                 : RenderPass::None;
    properties->renderPasses = mbgl::underlying_type(passes);
    evaluatedProperties = std::move(properties);

#if MLN_DRAWABLE_RENDERER
    if (layerGroup && layerGroup->getLayerTweaker()) {
        layerGroup->getLayerTweaker()->updateProperties(evaluatedProperties);
    }
#endif // MLN_DRAWABLE_RENDERER
}

bool RenderSymbolLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderSymbolLayer::hasCrossfade() const {
    return false;
}

#if MLN_LEGACY_RENDERER

void RenderSymbolLayer::render(PaintParameters& parameters) {
    assert(renderTiles);
    if (parameters.pass == RenderPass::Opaque) {
        return;
    }

    if (!parameters.shaders.getLegacyGroup().populate(programs.symbolIconProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(programs.symbolSDFIconProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(programs.symbolSDFTextProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(programs.symbolTextAndIconProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(programs.collisionBoxProgram)) return;
    if (!parameters.shaders.getLegacyGroup().populate(programs.collisionCircleProgram)) return;

    const bool sortFeaturesByKey = !impl_cast(baseImpl).layout.get<SymbolSortKey>().isUndefined();
    std::multiset<RenderableSegment> renderableSegments;

    const auto draw = [&parameters, this](auto& programInstance,
                                          const auto& uniformValues,
                                          const auto& buffers,
                                          const auto& segments,
                                          const auto& symbolSizeBinder,
                                          const auto& binders,
                                          const auto& paintProperties,
                                          const auto& textureBindings,
                                          const std::string& suffix) {
        const auto allUniformValues = programInstance.computeAllUniformValues(
            uniformValues, *symbolSizeBinder, binders, paintProperties, static_cast<float>(parameters.state.getZoom()));

        const auto allAttributeBindings = programInstance.computeAllAttributeBindings(*buffers.vertexBuffer,
                                                                                      *buffers.dynamicVertexBuffer,
                                                                                      *buffers.opacityVertexBuffer,
                                                                                      binders,
                                                                                      paintProperties);

        this->checkRenderability(parameters, programInstance.activeBindingCount(allAttributeBindings));

        segments.match(
            [&](const SegmentWrapper& segment) {
                programInstance.draw(parameters.context,
                                     *parameters.renderPass,
                                     gfx::Triangles(),
                                     parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                                     gfx::StencilMode::disabled(),
                                     parameters.colorModeForRenderPass(),
                                     gfx::CullFaceMode::disabled(),
                                     *buffers.indexBuffer,
                                     segment,
                                     allUniformValues,
                                     allAttributeBindings,
                                     textureBindings,
                                     this->getID() + "/" + suffix);
            },
            [&](const SegmentVectorWrapper& segmentVector) {
                programInstance.draw(parameters.context,
                                     *parameters.renderPass,
                                     gfx::Triangles(),
                                     parameters.depthModeForSublayer(0, gfx::DepthMaskType::ReadOnly),
                                     gfx::StencilMode::disabled(),
                                     parameters.colorModeForRenderPass(),
                                     gfx::CullFaceMode::disabled(),
                                     *buffers.indexBuffer,
                                     segmentVector,
                                     allUniformValues,
                                     allAttributeBindings,
                                     textureBindings,
                                     this->getID() + "/" + suffix);
            });
    };

    for (const RenderTile& tile : *renderTiles) {
        const LayerRenderData* renderData = getRenderDataForPass(tile, parameters.pass);
        if (!renderData) {
            continue;
        }

        auto& bucket = static_cast<SymbolBucket&>(*renderData->bucket);
        assert(bucket.paintProperties.find(getID()) != bucket.paintProperties.end());
        const auto& bucketPaintProperties = bucket.paintProperties.at(getID());

        // Prevent a flickering issue when a symbol is moved.
        // bucket.justReloaded = false;

        auto addRenderables =
            [&renderableSegments, &tile, renderData, &bucketPaintProperties, it = renderableSegments.begin()](
                auto& segments, const SymbolType type) mutable {
                for (auto& segment : segments) {
                    it = renderableSegments.emplace_hint(it,
                                                         SegmentWrapper{std::ref(segment)},
                                                         tile,
                                                         *renderData,
                                                         bucketPaintProperties,
                                                         segment.sortKey,
                                                         type);
                }
            };

        if (bucket.hasIconData()) {
            if (sortFeaturesByKey) {
                addRenderables(bucket.icon.segments, SymbolType::IconRGBA);
            } else {
                drawIcon(programs,
                         draw,
                         tile,
                         *renderData,
                         std::cref(bucket.icon.segments),
                         bucketPaintProperties,
                         parameters,
                         false /*sdfIcon*/
                );
            }
        }

        if (bucket.hasSdfIconData()) {
            if (sortFeaturesByKey) {
                addRenderables(bucket.sdfIcon.segments, SymbolType::IconSDF);
            } else {
                drawIcon(programs,
                         draw,
                         tile,
                         *renderData,
                         std::cref(bucket.sdfIcon.segments),
                         bucketPaintProperties,
                         parameters,
                         true /*sdfIcon*/
                );
            }
        }

        if (bucket.hasTextData()) {
            if (sortFeaturesByKey) {
                addRenderables(bucket.text.segments, SymbolType::Text);
            } else {
                drawText(programs,
                         draw,
                         tile,
                         *renderData,
                         std::cref(bucket.text.segments),
                         bucketPaintProperties,
                         parameters);
            }
        }

        const auto drawCollisonData = [&](const bool isText,
                                          const bool hasCollisionBox,
                                          const bool hasCollisionCircle) {
            if (!hasCollisionBox && !hasCollisionCircle) return;

            static const style::Properties<>::PossiblyEvaluated properties{};
            static const CollisionBoxProgram::Binders paintAttributeData(properties, 0);
            const auto pixelRatio = tile.id.pixelsToTileUnits(1.0f, static_cast<float>(parameters.state.getZoom()));
            const auto scale = static_cast<float>(
                std::pow(2, parameters.state.getZoom() - tile.getOverscaledTileID().overscaledZ));
            std::array<float, 2> extrudeScale = {{parameters.pixelsToGLUnits[0] / (pixelRatio * scale),
                                                  parameters.pixelsToGLUnits[1] / (pixelRatio * scale)}};
            const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData->layerProperties);
            const auto& layout = *bucket.layout;
            const auto values = isText ? textPropertyValues(evaluated, layout) : iconPropertyValues(evaluated, layout);
            const bool needTranslate = values.translate[0] != 0 || values.translate[1] != 0;

            if (hasCollisionBox) {
                const auto& collisionBox = isText ? bucket.textCollisionBox : bucket.iconCollisionBox;
                programs.collisionBoxProgram->draw(
                    parameters.context,
                    *parameters.renderPass,
                    gfx::Lines{1.0f},
                    gfx::DepthMode::disabled(),
                    gfx::StencilMode::disabled(),
                    parameters.colorModeForRenderPass(),
                    gfx::CullFaceMode::disabled(),
                    CollisionBoxProgram::LayoutUniformValues{
                        uniforms::matrix::Value(
                            (needTranslate
                                 ? tile.translatedMatrix(values.translate, values.translateAnchor, parameters.state)
                                 : tile.matrix)),
                        uniforms::extrude_scale::Value(extrudeScale),
                        uniforms::camera_to_center_distance::Value(parameters.state.getCameraToCenterDistance())},
                    *collisionBox->vertexBuffer,
                    *collisionBox->dynamicVertexBuffer,
                    *collisionBox->indexBuffer,
                    collisionBox->segments,
                    paintAttributeData,
                    properties,
                    CollisionBoxProgram::TextureBindings{},
                    static_cast<float>(parameters.state.getZoom()),
                    getID());
            }
            if (hasCollisionCircle) {
                const auto& collisionCircle = isText ? bucket.textCollisionCircle : bucket.iconCollisionCircle;
                programs.collisionCircleProgram->draw(
                    parameters.context,
                    *parameters.renderPass,
                    gfx::Triangles(),
                    gfx::DepthMode::disabled(),
                    gfx::StencilMode::disabled(),
                    parameters.colorModeForRenderPass(),
                    gfx::CullFaceMode::disabled(),
                    CollisionCircleProgram::LayoutUniformValues{
                        uniforms::matrix::Value(
                            (needTranslate
                                 ? tile.translatedMatrix(values.translate, values.translateAnchor, parameters.state)
                                 : tile.matrix)),
                        uniforms::extrude_scale::Value(extrudeScale),
                        uniforms::overscale_factor::Value(
                            static_cast<float>(tile.getOverscaledTileID().overscaleFactor())),
                        uniforms::camera_to_center_distance::Value(parameters.state.getCameraToCenterDistance())},
                    *collisionCircle->vertexBuffer,
                    *collisionCircle->dynamicVertexBuffer,
                    *collisionCircle->indexBuffer,
                    collisionCircle->segments,
                    paintAttributeData,
                    properties,
                    CollisionCircleProgram::TextureBindings{},
                    static_cast<float>(parameters.state.getZoom()),
                    getID());
            }
        };
        drawCollisonData(false /*isText*/, bucket.hasIconCollisionBoxData(), bucket.hasIconCollisionCircleData());
        drawCollisonData(true /*isText*/, bucket.hasTextCollisionBoxData(), bucket.hasTextCollisionCircleData());
    }

    if (sortFeaturesByKey) {
        for (auto& renderable : renderableSegments) {
            if (renderable.type == SymbolType::Text) {
                drawText(programs,
                         draw,
                         renderable.tile,
                         renderable.renderData,
                         renderable.segment,
                         renderable.bucketPaintProperties,
                         parameters);
            } else {
                drawIcon(programs,
                         draw,
                         renderable.tile,
                         renderable.renderData,
                         renderable.segment,
                         renderable.bucketPaintProperties,
                         parameters,
                         renderable.type == SymbolType::IconSDF);
            }
        }
    }
}

#endif // MLN_LEGACY_RENDERER

// static
style::IconPaintProperties::PossiblyEvaluated RenderSymbolLayer::iconPaintProperties(
    const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_) {
    return style::IconPaintProperties::PossiblyEvaluated{evaluated_.get<style::IconOpacity>(),
                                                         evaluated_.get<style::IconColor>(),
                                                         evaluated_.get<style::IconHaloColor>(),
                                                         evaluated_.get<style::IconHaloWidth>(),
                                                         evaluated_.get<style::IconHaloBlur>(),
                                                         evaluated_.get<style::IconTranslate>(),
                                                         evaluated_.get<style::IconTranslateAnchor>()};
}

// static
style::TextPaintProperties::PossiblyEvaluated RenderSymbolLayer::textPaintProperties(
    const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_) {
    return style::TextPaintProperties::PossiblyEvaluated{evaluated_.get<style::TextOpacity>(),
                                                         evaluated_.get<style::TextColor>(),
                                                         evaluated_.get<style::TextHaloColor>(),
                                                         evaluated_.get<style::TextHaloWidth>(),
                                                         evaluated_.get<style::TextHaloBlur>(),
                                                         evaluated_.get<style::TextTranslate>(),
                                                         evaluated_.get<style::TextTranslateAnchor>()};
}

void RenderSymbolLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTilesSortedByYPosition();

#if MLN_DRAWABLE_RENDERER
    updateRenderTileIDs();
#endif // MLN_DRAWABLE_RENDERER

    addRenderPassesFromTiles();

    placementData.clear();

    for (const RenderTile& renderTile : *renderTiles) {
        auto* bucket = static_cast<SymbolBucket*>(renderTile.getBucket(*baseImpl));
        if (bucket && bucket->bucketLeaderID == getID()) {
            // Only place this layer if it's the "group leader" for the bucket
            const Tile* tile = params.source->getRenderedTile(renderTile.id);
            assert(tile);
            assert(tile->kind == Tile::Kind::Geometry);

            auto featureIndex = static_cast<const GeometryTile*>(tile)->getFeatureIndex();

            if (bucket->sortKeyRanges.empty()) {
                placementData.push_back({*bucket, renderTile, featureIndex, baseImpl->source, std::nullopt});
            } else {
                for (const auto& sortKeyRange : bucket->sortKeyRanges) {
                    BucketPlacementData layerData{*bucket, renderTile, featureIndex, baseImpl->source, sortKeyRange};
                    auto sortPosition = std::upper_bound(
                        placementData.cbegin(), placementData.cend(), layerData, [](const auto& lhs, const auto& rhs) {
                            assert(lhs.sortKeyRange && rhs.sortKeyRange);
                            return lhs.sortKeyRange->sortKey < rhs.sortKeyRange->sortKey;
                        });
                    placementData.insert(sortPosition, std::move(layerData));
                }
            }
        }
    }
}

#if MLN_DRAWABLE_RENDERER

namespace {

template <typename TText, typename TIcon>
const auto& getProperty(const SymbolBucket::PaintProperties& paintProps, bool isText) {
    return isText ? paintProps.textBinders.get<TText>() : paintProps.iconBinders.get<TIcon>();
}

template <typename TText, typename TIcon, std::size_t N>
auto getInterpFactor(const SymbolBucket::PaintProperties& paintProps, bool isText, float currentZoom) {
    return std::get<N>(getProperty<TText, TIcon>(paintProps, isText)->interpolationFactor(currentZoom));
}

SymbolDrawableInterpolateUBO buildInterpUBO(const SymbolBucket::PaintProperties& paint, const bool t, const float z) {
    return {/* .fill_color_t = */ getInterpFactor<TextColor, IconColor, 0>(paint, t, z),
            /* .halo_color_t = */ getInterpFactor<TextHaloColor, IconHaloColor, 0>(paint, t, z),
            /* .opacity_t = */ getInterpFactor<TextOpacity, IconOpacity, 0>(paint, t, z),
            /* .halo_width_t = */ getInterpFactor<TextHaloWidth, IconHaloWidth, 0>(paint, t, z),
            /* .halo_blur_t = */ getInterpFactor<TextHaloBlur, IconHaloBlur, 0>(paint, t, z),
            /* .padding = */ 0,
            0,
            0};
}

SymbolDrawableTilePropsUBO buildTileUBO(const SymbolBucket& bucket,
                                        const gfx::SymbolDrawableData& drawData,
                                        const float currentZoom) {
    const bool isText = (drawData.symbolType == SymbolType::Text);
    const ZoomEvaluatedSize size = isText ? bucket.textSizeBinder->evaluateForZoom(currentZoom)
                                          : bucket.iconSizeBinder->evaluateForZoom(currentZoom);
    return {
        /* .is_text = */ isText,
        /* .is_halo = */ drawData.isHalo,
        /* .pitch_with_map = */ (drawData.pitchAlignment == style::AlignmentType::Map),
        /* .is_size_zoom_constant = */ size.isZoomConstant,
        /* .is_size_feature_constant = */ size.isFeatureConstant,
        /* .size_t = */ size.sizeT,
        /* .size = */ size.size,
        /* .padding = */ 0,
    };
}

static const StringIdentity idDataAttibName = StringIndexer::get("a_data");
static const StringIdentity idPosOffsetAttribName = StringIndexer::get("a_pos_offset");
static const StringIdentity idPixOffsetAttribName = StringIndexer::get("a_pixeloffset");
static const StringIdentity idProjPosAttribName = StringIndexer::get("a_projected_pos");
static const StringIdentity idFadeOpacityAttribName = StringIndexer::get("a_fade_opacity");
static const StringIdentity idTexUniformName = StringIndexer::get("u_texture");
static const StringIdentity idTexIconUniformName = StringIndexer::get("u_texture_icon");

std::vector<std::string> updateTileAttributes(const SymbolBucket::Buffer& buffer,
                                              const bool isText,
                                              const SymbolBucket::PaintProperties& paintProps,
                                              const SymbolPaintProperties::PossiblyEvaluated& evaluated,
                                              gfx::VertexAttributeArray& attribs) {
    if (const auto& attr = attribs.getOrAdd(idPosOffsetAttribName)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(SymbolLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutVertex),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.getOrAdd(idDataAttibName)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(SymbolLayoutVertex, a2),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutVertex),
                               gfx::AttributeDataType::UShort4);
    }
    if (const auto& attr = attribs.getOrAdd(idPixOffsetAttribName)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(SymbolLayoutVertex, a3),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutVertex),
                               gfx::AttributeDataType::Short4);
    }

    if (const auto& attr = attribs.getOrAdd(idProjPosAttribName)) {
        using Vertex = gfx::Vertex<SymbolDynamicLayoutAttributes>;
        attr->setSharedRawData(buffer.sharedDynamicVertices,
                               offsetof(Vertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(Vertex),
                               gfx::AttributeDataType::Float3);
    }
    if (const auto& attr = attribs.getOrAdd(idFadeOpacityAttribName)) {
        using Vertex = gfx::Vertex<SymbolOpacityAttributes>;
        attr->setSharedRawData(buffer.sharedOpacityVertices,
                               offsetof(Vertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(Vertex),
                               gfx::AttributeDataType::Float);
    }

    return isText
               ? attribs
                     .readDataDrivenPaintProperties<TextOpacity, TextColor, TextHaloColor, TextHaloWidth, TextHaloBlur>(
                         paintProps.textBinders, evaluated)
               : attribs
                     .readDataDrivenPaintProperties<IconOpacity, IconColor, IconHaloColor, IconHaloWidth, IconHaloBlur>(
                         paintProps.iconBinders, evaluated);
}

void updateTileDrawable(gfx::Drawable& drawable,
                        gfx::Context& context,
                        const SymbolBucket& bucket,
                        const SymbolBucket::PaintProperties& paintProps,
                        const SymbolPaintProperties::PossiblyEvaluated& evaluated,
                        const TransformState& state,
                        gfx::UniformBufferPtr& textInterpUBO,
                        gfx::UniformBufferPtr& iconInterpUBO) {
    if (!drawable.getData()) {
        return;
    }

    auto& drawData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
    const auto isText = (drawData.symbolType == SymbolType::Text);
    const auto sdfIcons = (drawData.symbolType == SymbolType::IconSDF);
    const auto currentZoom = static_cast<float>(state.getZoom());

    // This property can be set after the initial appearance of the tile, as part of the layout process.
    drawData.bucketVariablePlacement = bucket.hasVariablePlacement;

    auto& uniforms = drawable.mutableUniformBuffers();

    // Create or update the shared interpolation UBO
    gfx::UniformBufferPtr& interpUBO = isText ? textInterpUBO : iconInterpUBO;
    if (interpUBO) {
        uniforms.addOrReplace(SymbolLayerTweaker::idSymbolDrawableInterpolateUBOName, interpUBO);
    } else {
        const auto ubo = buildInterpUBO(paintProps, isText, currentZoom);
        interpUBO = uniforms.get(SymbolLayerTweaker::idSymbolDrawableInterpolateUBOName);
        if (interpUBO) {
            interpUBO->update(&ubo, sizeof(ubo));
        } else {
            interpUBO = context.createUniformBuffer(&ubo, sizeof(ubo));
            uniforms.addOrReplace(SymbolLayerTweaker::idSymbolDrawableInterpolateUBOName, interpUBO);
        }
    }

    const auto tileUBO = buildTileUBO(bucket, drawData, currentZoom);
    uniforms.createOrUpdate(SymbolLayerTweaker::idSymbolDrawableTilePropsUBOName, &tileUBO, context);

    const auto& buffer = isText ? bucket.text : (sdfIcons ? bucket.sdfIcon : bucket.icon);
    const auto vertexCount = buffer.vertices().elements();

    drawable.setVertices({}, vertexCount, gfx::AttributeDataType::Short4);

    // TODO: detect whether anything has actually changed
    // See `Placement::updateBucketDynamicVertices`

    gfx::VertexAttributeArray attribs;
    updateTileAttributes(buffer, isText, paintProps, evaluated, attribs);
    drawable.setVertexAttributes(std::move(attribs));
}

static const StringIdentity idCollisionPosAttribName = StringIndexer::get("a_pos");
static const StringIdentity idCollisionAnchorPosAttribName = StringIndexer::get("a_anchor_pos");
static const StringIdentity idCollisionExtrudeAttribName = StringIndexer::get("a_extrude");
static const StringIdentity idCollisionPlacedAttribName = StringIndexer::get("a_placed");
static const StringIdentity idCollisionShiftAttribName = StringIndexer::get("a_shift");

gfx::VertexAttributeArray getCollisionVertexAttributes(const SymbolBucket::CollisionBuffer& buffer, bool staticCopy) {
    gfx::VertexAttributeArray vertexAttrs;

    if (staticCopy) {
        if (auto& attr = vertexAttrs.getOrAdd(idCollisionPosAttribName)) {
            std::size_t index{0};
            for (auto& v : buffer.vertices().vector()) {
                attr->set<gfx::VertexAttribute::int2>(index++, {v.a1[0], v.a1[1]});
            }
        }
        if (auto& attr = vertexAttrs.getOrAdd(idCollisionAnchorPosAttribName)) {
            std::size_t index{0};
            for (auto& v : buffer.vertices().vector()) {
                attr->set<gfx::VertexAttribute::int2>(index++, {v.a2[0], v.a2[1]});
            }
        }
        if (auto& attr = vertexAttrs.getOrAdd(idCollisionExtrudeAttribName)) {
            std::size_t index{0};
            for (auto& v : buffer.vertices().vector()) {
                attr->set<gfx::VertexAttribute::int2>(index++, {v.a3[0], v.a3[1]});
            }
        }

        if (auto& attr = vertexAttrs.getOrAdd(idCollisionPlacedAttribName)) {
            std::size_t index{0};
            for (auto& v : buffer.dynamicVertices().vector()) {
                attr->set<gfx::VertexAttribute::int2>(index++, {v.a1[0], v.a1[1]});
            }
        }
        if (auto& attr = vertexAttrs.getOrAdd(idCollisionShiftAttribName)) {
            std::size_t index{0};
            for (auto& v : buffer.dynamicVertices().vector()) {
                attr->set<gfx::VertexAttribute::float2>(index++, {v.a2[0], v.a2[1]});
            }
        }
    } else {
        if (const auto& attr = vertexAttrs.getOrAdd(idCollisionPosAttribName)) {
            attr->setSharedRawData(buffer.sharedVertices,
                                   offsetof(CollisionBoxVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(CollisionBoxVertex),
                                   gfx::AttributeDataType::Short2);
        }
        if (const auto& attr = vertexAttrs.getOrAdd(idCollisionAnchorPosAttribName)) {
            attr->setSharedRawData(buffer.sharedVertices,
                                   offsetof(CollisionBoxVertex, a2),
                                   /*vertexOffset=*/0,
                                   sizeof(CollisionBoxVertex),
                                   gfx::AttributeDataType::Short2);
        }
        if (const auto& attr = vertexAttrs.getOrAdd(idCollisionExtrudeAttribName)) {
            attr->setSharedRawData(buffer.sharedVertices,
                                   offsetof(CollisionBoxVertex, a3),
                                   /*vertexOffset=*/0,
                                   sizeof(CollisionBoxVertex),
                                   gfx::AttributeDataType::Short2);
        }

        using DynamicVertex = gfx::Vertex<CollisionBoxDynamicAttributes>;

        if (const auto& attr = vertexAttrs.getOrAdd(idCollisionPlacedAttribName)) {
            attr->setSharedRawData(buffer.sharedDynamicVertices,
                                   offsetof(DynamicVertex, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(DynamicVertex),
                                   gfx::AttributeDataType::UByte2);
        }
        if (const auto& attr = vertexAttrs.getOrAdd(idCollisionShiftAttribName)) {
            attr->setSharedRawData(buffer.sharedDynamicVertices,
                                   offsetof(DynamicVertex, a2),
                                   /*vertexOffset=*/0,
                                   sizeof(DynamicVertex),
                                   gfx::AttributeDataType::Float2);
        }
    }

    return vertexAttrs;
}

} // namespace

void RenderSymbolLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    if (const auto tileGroup = static_cast<TileLayerGroup*>(layerGroup.get())) {
        stats.drawablesRemoved += tileGroup->removeDrawables(renderPass, tileID).size();
    }
    if (collisionTileLayerGroup) {
        stats.drawablesRemoved += collisionTileLayerGroup->removeDrawables(renderPass, tileID).size();
    }
}

void RenderSymbolLayer::removeAllDrawables() {
    if (layerGroup) {
        stats.drawablesRemoved += layerGroup->getDrawableCount();
        layerGroup->clearDrawables();
    }
    if (collisionTileLayerGroup) {
        stats.drawablesRemoved += collisionTileLayerGroup->getDrawableCount();
        collisionTileLayerGroup->clearDrawables();
    }
}

void RenderSymbolLayer::update(gfx::ShaderRegistry& shaders,
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
            layerGroup_->setLayerTweaker(std::make_shared<SymbolLayerTweaker>(evaluatedProperties));
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }

    if (!collisionTileLayerGroup) {
        if ((collisionTileLayerGroup = context.createTileLayerGroup(
                 layerIndex, /*initialCapacity=*/64, getID() + "-collision"))) {
            collisionTileLayerGroup->setLayerTweaker(std::make_shared<CollisionLayerTweaker>(evaluatedProperties));
            activateLayerGroup(collisionTileLayerGroup, true, changes);
        }
    }

    if (!symbolIconGroup) {
        symbolIconGroup = shaders.getShaderGroup(std::string(SymbolIconShaderName));
    }
    if (!symbolSDFIconGroup) {
        symbolSDFIconGroup = shaders.getShaderGroup(std::string(SymbolSDFIconShaderName));
    }
    if (!symbolSDFTextGroup) {
        symbolSDFTextGroup = shaders.getShaderGroup(std::string(SymbolSDFTextShaderName));
    }
    if (!symbolTextAndIconGroup) {
        symbolTextAndIconGroup = shaders.getShaderGroup(std::string(SymbolTextAndIconShaderName));
    }
    if (!collisionBoxGroup) {
        collisionBoxGroup = shaders.getShaderGroup(std::string(CollisionBoxShaderName));
    }
    if (!collisionCircleGroup) {
        collisionCircleGroup = shaders.getShaderGroup(std::string(CollisionCircleShaderName));
    }

    // remove drawables that are dropped out of scope
    auto* tileLayerGroup = static_cast<TileLayerGroup*>(layerGroup.get());
    stats.drawablesRemoved += tileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        if (drawable.getRenderPass() != passes || (tileID && !hasRenderTile(*tileID))) {
            return true;
        }
        return false;
    });
    stats.drawablesRemoved += collisionTileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto& tileID = drawable.getTileID();
        if (drawable.getRenderPass() != passes || (tileID && !hasRenderTile(*tileID))) {
            return true;
        }
        return false;
    });

    const bool sortFeaturesByKey = !impl_cast(baseImpl).layout.get<SymbolSortKey>().isUndefined();
    std::multiset<RenderableSegment> renderableSegments;
    std::unique_ptr<gfx::DrawableBuilder> builder;

    const auto currentZoom = static_cast<float>(state.getZoom());
    const auto layerPrefix = getID() + "/";
    const auto layerCollisionPrefix = getID() + "-collision/";

    std::unique_ptr<gfx::DrawableBuilder> collisionBuilder = context.createDrawableBuilder(layerCollisionPrefix);
    collisionBuilder->setSubLayerIndex(0);
    collisionBuilder->setEnableStencil(false);
    collisionBuilder->setEnableDepth(false);
    collisionBuilder->setRenderPass(passes);
    collisionBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
    collisionBuilder->setColorMode(gfx::ColorMode::alphaBlended());
    collisionBuilder->setVertexAttrNameId(idCollisionPosAttribName);

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const auto* optRenderData = getRenderDataForPass(tile, passes);
        if (!optRenderData || !optRenderData->bucket || !optRenderData->bucket->hasData()) {
            removeTile(passes, tileID);
            continue;
        }

        const auto& renderData = *optRenderData;
        const auto& bucket = static_cast<const SymbolBucket&>(*renderData.bucket);

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(passes, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        assert(bucket.paintProperties.find(getID()) != bucket.paintProperties.end());
        const auto& bucketPaintProperties = bucket.paintProperties.at(getID());

        auto addCollisionDrawables =
            [&collisionBuilder, this, &bucket, &renderData, &context, &layerCollisionPrefix, &tileID](
                const bool isText, const bool hasCollisionBox, const bool hasCollisionCircle) {
                if (!hasCollisionBox && !hasCollisionCircle) return;

                const auto& layout = *bucket.layout;
                const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData.layerProperties);
                const auto values = isText ? textPropertyValues(evaluated, layout)
                                           : iconPropertyValues(evaluated, layout);
                const std::string suffix = isText ? "text/" : "icon/";

                auto addVertices = [&collisionBuilder](const auto& vertices, bool staticCopy) {
                    if (staticCopy) {
                        std::vector<std::array<int16_t, 2>> verts(vertices.size());
                        std::transform(vertices.begin(),
                                       vertices.end(),
                                       verts.begin(),
                                       [](const auto& v) -> std::array<int16_t, 2> { return v.a1; });
                        collisionBuilder->addVertices(verts, 0, verts.size());
                    } else {
                        collisionBuilder->setRawVertices({}, vertices.size(), gfx::AttributeDataType::Short2);
                    }
                };

                constexpr bool staticVertexAndAttributes = true;

                if (hasCollisionBox) {
                    const auto& collisionBox = isText ? bucket.textCollisionBox : bucket.iconCollisionBox;
                    if (const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                            collisionBoxGroup->getOrCreateShader(
                                context, {}, StringIndexer::get(idCollisionPosAttribName)))) {
                        collisionBuilder->setDrawableName(layerCollisionPrefix + suffix + "box");
                        collisionBuilder->setShader(shader);
                        addVertices(collisionBox->vertices().vector(), staticVertexAndAttributes);
                        collisionBuilder->setVertexAttributes(
                            getCollisionVertexAttributes(*collisionBox, staticVertexAndAttributes));
                        collisionBuilder->setSegments(gfx::Lines(1.0f),
                                                      collisionBox->sharedLines,
                                                      collisionBox->segments.data(),
                                                      collisionBox->segments.size());
                        collisionBuilder->flush();
                    }
                }

                if (hasCollisionCircle) {
                    const auto& collisionCircle = isText ? bucket.textCollisionCircle : bucket.iconCollisionCircle;
                    if (const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                            collisionCircleGroup->getOrCreateShader(
                                context, {}, StringIndexer::get(idCollisionPosAttribName)))) {
                        collisionBuilder->setDrawableName(layerCollisionPrefix + suffix + "circle");
                        collisionBuilder->setShader(shader);
                        addVertices(collisionCircle->vertices().vector(), staticVertexAndAttributes);
                        collisionBuilder->setVertexAttributes(
                            getCollisionVertexAttributes(*collisionCircle, staticVertexAndAttributes));
                        collisionBuilder->setSegments(gfx::Triangles(),
                                                      collisionCircle->sharedTriangles,
                                                      collisionCircle->segments.data(),
                                                      collisionCircle->segments.size());
                        collisionBuilder->flush();
                    }
                }

                // add drawables to layer group
                for (auto& drawable : collisionBuilder->clearDrawables()) {
                    drawable->setTileID(tileID);
                    auto drawData = std::make_unique<gfx::CollisionDrawableData>(values.translate,
                                                                                 values.translateAnchor);
                    drawable->setData(std::move(drawData));
                    collisionTileLayerGroup->addDrawable(passes, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            };

        // If we already have drawables for this tile, update them.
        if (tileLayerGroup->getDrawableCount(passes, tileID) > 0) {
            gfx::UniformBufferPtr textInterpUBO, iconInterpUBO;

            // Just update the drawables we already created
            tileLayerGroup->visitDrawables(passes, tileID, [&](gfx::Drawable& drawable) {
                const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData.layerProperties);
                updateTileDrawable(
                    drawable, context, bucket, bucketPaintProperties, evaluated, state, textInterpUBO, iconInterpUBO);
            });

            // re-create collision drawables
            collisionTileLayerGroup->removeDrawables(passes, tileID);
            addCollisionDrawables(
                false /*isText*/, bucket.hasIconCollisionBoxData(), bucket.hasIconCollisionCircleData());
            addCollisionDrawables(
                true /*isText*/, bucket.hasTextCollisionBoxData(), bucket.hasTextCollisionCircleData());

            continue;
        }

        float serialKey = 1.0f;
        auto addRenderables = [&, it = renderableSegments.begin()](const SymbolBucket::Buffer& buffer,
                                                                   const SymbolType type) mutable {
            for (const auto& segment : buffer.segments) {
                const auto key = sortFeaturesByKey ? segment.sortKey : (serialKey += 1.0);
                assert(segment.vertexOffset + segment.vertexLength <= buffer.vertices().elements());
                it = renderableSegments.emplace_hint(
                    it, std::ref(segment), tile, renderData, bucketPaintProperties, key, type, tileID.overscaledZ);
            }
        };

        const auto& atlases = tile.getAtlasTextures();
        if (!atlases) {
            continue;
        }

        if (bucket.hasIconData() && atlases->icon) {
            addRenderables(bucket.icon, SymbolType::IconRGBA);
        }
        if (bucket.hasSdfIconData() && atlases->icon) {
            addRenderables(bucket.sdfIcon, SymbolType::IconSDF);
        }
        if (bucket.hasTextData() && atlases->glyph) {
            addRenderables(bucket.text, SymbolType::Text);
        }

        addCollisionDrawables(false /*isText*/, bucket.hasIconCollisionBoxData(), bucket.hasIconCollisionCircleData());
        addCollisionDrawables(true /*isText*/, bucket.hasTextCollisionBoxData(), bucket.hasTextCollisionCircleData());
    }

    // We'll be processing renderables across tiles, potentially out-of-order, so keep
    // track of some things by tile ID so we don't have to re-build them multiple times.
    using RawVertexVec = std::vector<std::uint8_t>; // <int16_t, 4>
    struct TileInfo {
        RawVertexVec textVertices, iconVertices;
        gfx::DrawableTweakerPtr textTweaker, iconTweaker;
        gfx::UniformBufferPtr textInterp, iconInterp;
    };
    std::unordered_map<UnwrappedTileID, TileInfo> tileCache;

    for (auto& renderable : renderableSegments) {
        const auto isText = (renderable.type == SymbolType::Text);
        const auto sdfIcons = (renderable.type == SymbolType::IconSDF);

        const auto& tile = renderable.tile;
        const auto tileID = tile.id.overscaleTo(renderable.overscaledZ);
        const auto& bucket = static_cast<const SymbolBucket&>(*renderable.renderData.bucket);
        const auto& buffer = isText ? bucket.text : (sdfIcons ? bucket.sdfIcon : bucket.icon);

        const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderable.renderData.layerProperties);
        const auto& bucketPaintProperties = bucket.paintProperties.at(getID());

        const auto& layout = *bucket.layout;
        const auto values = isText ? textPropertyValues(evaluated, layout) : iconPropertyValues(evaluated, layout);

        const auto& atlases = tile.getAtlasTextures();
        if (!atlases) {
            assert(false);
            continue;
        }

        auto& tileInfo = tileCache[tile.id];

        const auto vertexCount = buffer.vertices().elements();

        gfx::VertexAttributeArray attribs;
        const auto uniformProps = updateTileAttributes(buffer, isText, bucketPaintProperties, evaluated, attribs);

        const auto textHalo = evaluated.get<style::TextHaloColor>().constantOr(Color::black()).a > 0.0f &&
                              evaluated.get<style::TextHaloWidth>().constantOr(1);
        const auto textFill = evaluated.get<style::TextColor>().constantOr(Color::black()).a > 0.0f;

        const auto iconHalo = evaluated.get<style::IconHaloColor>().constantOr(Color::black()).a > 0.0f &&
                              evaluated.get<style::IconHaloWidth>().constantOr(1);
        const auto iconFill = evaluated.get<style::IconColor>().constantOr(Color::black()).a > 0.0f;

        // Share interpolation UBOs across all the elements of the same type in each tile
        auto& interpUBO = isText ? tileInfo.textInterp : tileInfo.iconInterp;
        if (!interpUBO) {
            const auto interpolateBuf = buildInterpUBO(bucketPaintProperties, isText, currentZoom);
            interpUBO = context.createUniformBuffer(&interpolateBuf, sizeof(interpolateBuf));
        }

        if (builder) {
            builder->clearTweakers();
        }

        const auto draw =
            [&](const gfx::ShaderGroupPtr& shaderGroup, const bool isHalo, const std::string_view suffix) {
                if (!shaderGroup) {
                    return;
                }

                // We can use the same tweakers for all the segments in a tile
                if (isText && !tileInfo.textTweaker) {
                    const bool textSizeIsZoomConstant =
                        bucket.textSizeBinder->evaluateForZoom(static_cast<float>(state.getZoom())).isZoomConstant;
                    tileInfo.textTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(atlases,
                                                                                         idTexIconUniformName,
                                                                                         idTexUniformName,
                                                                                         isText,
                                                                                         false,
                                                                                         values.rotationAlignment,
                                                                                         false,
                                                                                         textSizeIsZoomConstant);
                }
                if (!isText && !tileInfo.iconTweaker) {
                    const bool iconScaled = layout.get<IconSize>().constantOr(1.0) != 1.0 || bucket.iconsNeedLinear;
                    tileInfo.iconTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(atlases,
                                                                                         idTexIconUniformName,
                                                                                         idTexUniformName,
                                                                                         isText,
                                                                                         sdfIcons,
                                                                                         values.rotationAlignment,
                                                                                         iconScaled,
                                                                                         false);
                }

                if (!builder) {
                    builder = context.createDrawableBuilder(layerPrefix);
                    builder->setSubLayerIndex(0);
                    builder->setEnableStencil(false);
                    builder->setRenderPass(passes);
                    builder->setCullFaceMode(gfx::CullFaceMode::disabled());
                    builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                    builder->setColorMode(
                        ((mbgl::underlying_type(passes) & mbgl::underlying_type(RenderPass::Translucent)) != 0)
                            ? gfx::ColorMode::alphaBlended()
                            : gfx::ColorMode::unblended());
                    builder->setVertexAttrNameId(idPosOffsetAttribName);
                }

                const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                    shaderGroup->getOrCreateShader(context, uniformProps, StringIndexer::get(idPosOffsetAttribName)));
                if (!shader) {
                    return;
                }
                builder->setShader(shader);

                builder->clearTweakers();
                builder->addTweaker(isText ? tileInfo.textTweaker : tileInfo.iconTweaker);
                builder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short4);
                builder->setDrawableName(layerPrefix + std::string(suffix));
                builder->setVertexAttributes(attribs);

                builder->setSegments(gfx::Triangles(), buffer.sharedTriangles, &renderable.segment.get(), 1);

                builder->flush();

                for (auto& drawable : builder->clearDrawables()) {
                    drawable->setTileID(tileID);

                    auto drawData = std::make_unique<gfx::SymbolDrawableData>(
                        /*.isHalo=*/isHalo,
                        /*.bucketVaraiblePlacement=*/bucket.hasVariablePlacement,
                        /*.symbolType=*/renderable.type,
                        /*.pitchAlignment=*/values.pitchAlignment,
                        /*.rotationAlignment=*/values.rotationAlignment,
                        /*.placement=*/layout.get<SymbolPlacement>(),
                        /*.textFit=*/layout.get<IconTextFit>());

                    const auto tileUBO = buildTileUBO(bucket, *drawData, currentZoom);
                    drawable->setData(std::move(drawData));

                    auto& uniforms = drawable->mutableUniformBuffers();
                    uniforms.createOrUpdate(SymbolLayerTweaker::idSymbolDrawableTilePropsUBOName, &tileUBO, context);
                    uniforms.addOrReplace(SymbolLayerTweaker::idSymbolDrawableInterpolateUBOName, interpUBO);

                    tileLayerGroup->addDrawable(passes, tileID, std::move(drawable));
                    ++stats.drawablesAdded;
                }
            };

        if (isText) {
            if (bucket.iconsInText) {
                if (textHalo) {
                    draw(symbolTextAndIconGroup, /* isHalo = */ true, "halo");
                }

                if (textFill) {
                    draw(symbolTextAndIconGroup, /* isHalo = */ false, "fill");
                }
            } else {
                if (textHalo) {
                    draw(symbolSDFTextGroup, /* isHalo = */ true, "halo");
                }

                if (textFill) {
                    draw(symbolSDFTextGroup, /* isHalo = */ false, "fill");
                }
            }
        } else { // icons
            if (sdfIcons) {
                if (iconHalo) {
                    draw(symbolSDFIconGroup, /* isHalo = */ true, "halo");
                }

                if (iconFill) {
                    draw(symbolSDFIconGroup, /* isHalo = */ false, "fill");
                }
            } else {
                draw(symbolIconGroup, /* isHalo = */ false, "icon");
            }
        }
    }
}

#endif // MLN_DRAWABLE_RENDERER

} // namespace mbgl
