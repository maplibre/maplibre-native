#include <mbgl/renderer/layers/render_symbol_layer.hpp>

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/symbol_drawable_data.hpp>
#include <mbgl/layout/symbol_layout.hpp>
#include <mbgl/programs/collision_box_program.hpp>
#include <mbgl/programs/programs.hpp>
#include <mbgl/programs/symbol_program.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/property_evaluation_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/text/glyph_atlas.hpp>
#include <mbgl/text/shaping.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>

#include <cmath>
#include <set>

namespace mbgl {

using namespace style;
namespace {

constexpr std::string_view SymbolIconShaderName = "SymbolIconShader";
constexpr std::string_view SymbolSDFIconShaderName = "SymbolSDFTextShader";
constexpr std::string_view SymbolSDFTextShaderName = "SymbolSDFIconShader";
constexpr std::string_view SymbolTextAndIconShaderName = "SymbolTextAndIconShader";

style::SymbolPropertyValues iconPropertyValues(const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_,
                                               const style::SymbolLayoutProperties::PossiblyEvaluated& layout_) {
    return style::SymbolPropertyValues{layout_.get<style::IconPitchAlignment>(),
                                       layout_.get<style::IconRotationAlignment>(),
                                       layout_.get<style::IconKeepUpright>(),
                                       evaluated_.get<style::IconTranslate>(),
                                       evaluated_.get<style::IconTranslateAnchor>(),
                                       evaluated_.get<style::IconHaloColor>().constantOr(Color::black()).a > 0 &&
                                           evaluated_.get<style::IconHaloWidth>().constantOr(1),
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
                                           evaluated_.get<style::TextHaloWidth>().constantOr(1),
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
                 SymbolSDFIconProgram::layoutUniformValues(false,
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
                 SymbolSDFIconProgram::layoutUniformValues(false,
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
             SymbolIconProgram::layoutUniformValues(false,
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
    auto& bucket = static_cast<SymbolBucket&>(*renderData.bucket);
    const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData.layerProperties);
    const auto& layout = *bucket.layout;

    auto values = textPropertyValues(evaluated, layout);
    const auto& paintPropertyValues = RenderSymbolLayer::textPaintProperties(evaluated);

    const bool alongLine = layout.get<SymbolPlacement>() != SymbolPlacementType::Point &&
                           layout.get<TextRotationAlignment>() == AlignmentType::Map;

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
                       SymbolSDFTextProgram::layoutUniformValues(true,
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
                       SymbolSDFTextProgram::layoutUniformValues(true,
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
    auto& layout = impl_cast(baseImpl).layout;

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
}

bool RenderSymbolLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderSymbolLayer::hasCrossfade() const {
    return false;
}

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
                                          auto& segments,
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
                         std::ref(bucket.icon.segments),
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
                         std::ref(bucket.sdfIcon.segments),
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
                         std::ref(bucket.text.segments),
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

    renderTileIDs.clear();
    renderTileIDs.reserve(renderTiles->size());
    std::transform(renderTiles->begin(),
                   renderTiles->end(),
                   std::inserter(renderTileIDs, renderTileIDs.end()),
                   [](const auto& tile) { return tile.get().getOverscaledTileID(); });

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

static SymbolDrawableInterpolateUBO buildInterpUBO(bool isText,
                                                   float currentZoom,
                                                   const SymbolBucket::PaintProperties& pp) {
    const auto& t = pp.textBinders;
    const auto& i = pp.iconBinders;
    return {/* .fill_color_t = */ std::get<0>(
                (isText ? t.get<TextColor>() : i.get<IconColor>())->interpolationFactor(currentZoom)),
            /* .halo_color_t = */
            std::get<0>((isText ? t.get<TextHaloColor>() : i.get<IconHaloColor>())->interpolationFactor(currentZoom)),
            /* .opacity_t = */
            std::get<0>((isText ? t.get<TextOpacity>() : i.get<IconOpacity>())->interpolationFactor(currentZoom)),
            /* .halo_width_t = */
            std::get<0>((isText ? t.get<TextHaloWidth>() : i.get<IconHaloWidth>())->interpolationFactor(currentZoom)),
            /* .halo_blur_t = */
            std::get<0>((isText ? t.get<TextHaloBlur>() : i.get<IconHaloBlur>())->interpolationFactor(currentZoom)),
            /* .padding = */ 0,
            0,
            0};
}

static SymbolDrawableTilePropsUBO buildTileUBO(const SymbolBucket& bucket,
                                               const style::SymbolType symbolType,
                                               const style::AlignmentType pitchAlignment,
                                               const bool isHalo,
                                               const float currentZoom) {
    const bool isText = (symbolType == SymbolType::Text);
    const ZoomEvaluatedSize size = isText ? bucket.textSizeBinder->evaluateForZoom(currentZoom)
                                          : bucket.iconSizeBinder->evaluateForZoom(currentZoom);
    return {
        /* .is_text = */ isText,
        /* .is_halo = */ isHalo,
        /* .pitch_with_map = */ (pitchAlignment == style::AlignmentType::Map),
        /* .is_size_zoom_constant = */ size.isZoomConstant,
        /* .is_size_feature_constant = */ size.isFeatureConstant,
        /* .size_t = */ size.sizeT,
        /* .size = */ size.size,
        /* .padding = */ 0,
    };
}

constexpr auto dataAttibName = "a_data";
constexpr auto posOffsetAttribName = "a_pos_offset";
constexpr auto pixOffsetAttribName = "a_pixeloffset";
constexpr auto projPosAttribName = "a_projected_pos";
constexpr auto fadeOpacityAttribName = "a_fade_opacity";
constexpr auto texUniformName = "u_texture";
constexpr auto iconTexUniformName = "u_texture_icon";

static void updateTileDrawable(gfx::Drawable& drawable,
                               gfx::Context& context,
                               const SymbolBucket& bucket,
                               const SymbolBucket::PaintProperties& paintProps,
                               const TransformState& state) {
    if (!drawable.getData() || !*drawable.getData()) {
        return;
    }

    const auto& drawData = static_cast<const gfx::SymbolDrawableData&>(**drawable.getData());
    const auto isText = (drawData.symbolType == SymbolType::Text);
    const auto sdfIcons = (drawData.symbolType == SymbolType::IconSDF);
    const auto& buffer = isText ? bucket.text : (sdfIcons ? bucket.sdfIcon : bucket.icon);
    const auto currentZoom = static_cast<float>(state.getZoom());

    const auto tileUBO = buildTileUBO(
        bucket, drawData.symbolType, drawData.pitchAlignment, drawData.isHalo, currentZoom);
    const auto interpolateUBO = buildInterpUBO(isText, currentZoom, paintProps);

    auto& uniforms = drawable.mutableUniformBuffers();
    uniforms.createOrUpdate(SymbolLayerTweaker::SymbolDrawableTilePropsUBOName, &tileUBO, context);
    uniforms.createOrUpdate(SymbolLayerTweaker::SymbolDrawableInterpolateUBOName, &interpolateUBO, context);

    // TODO: detect whether anything has actually changed
    // See `Placement::updateBucketDynamicVertices`
    if (const auto newAttribs = drawable.getVertexAttributes().clone()) {
        if (auto& attr = newAttribs->getOrAdd(projPosAttribName)) {
            const auto count = buffer.dynamicVertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, util::cast<float>(buffer.dynamicVertices.at(i).a1));
            }
        }

        if (auto& attr = newAttribs->getOrAdd(fadeOpacityAttribName)) {
            const auto count = buffer.opacityVertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, buffer.opacityVertices.at(i).a1[0]);
            }
        }

        drawable.setVertexAttributes(std::move(*newAttribs));
    }
}

void RenderSymbolLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& state,
                               const RenderTree& /*renderTree*/,
                               UniqueChangeRequestVec& /*changes*/) {
    if (!renderTiles || renderTiles->empty() || passes == RenderPass::None) {
        removeAllTiles();
        return;
    }

    // Set up a layer group
    if (!tileLayerGroup) {
        tileLayerGroup = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID());
        if (!tileLayerGroup) {
            return;
        }
        tileLayerGroup->setLayerTweaker(std::make_shared<SymbolLayerTweaker>(evaluatedProperties));
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

    tileLayerGroup->observeDrawables([&](gfx::UniqueDrawable& drawable) {
        // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
        const auto tileID = drawable->getTileID();
        if (drawable->getRenderPass() != passes || (tileID && renderTileIDs.find(*tileID) == renderTileIDs.end())) {
            drawable.reset();
            ++stats.tileDrawablesRemoved;
        }
    });

    const bool sortFeaturesByKey = !impl_cast(baseImpl).layout.get<SymbolSortKey>().isUndefined();
    std::multiset<RenderableSegment> renderableSegments;
    std::unique_ptr<gfx::DrawableBuilder> builder;

    const auto currentZoom = static_cast<float>(state.getZoom());
    const auto layerPrefix = getID() + "/";

    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const auto* optRenderData = getRenderDataForPass(tile, passes);
        if (!optRenderData || !optRenderData->bucket) {
            removeTile(passes, tileID);
            continue;
        }

        const auto& renderData = *optRenderData;
        const auto& bucket = static_cast<const SymbolBucket&>(*renderData.bucket);

        assert(bucket.paintProperties.find(getID()) != bucket.paintProperties.end());
        const auto& bucketPaintProperties = bucket.paintProperties.at(getID());

        // If we already have drawables for this tile, update them.
        if (tileLayerGroup->getDrawableCount(passes, tileID) > 0) {
            tileLayerGroup->observeDrawables(passes, tileID, [&](gfx::Drawable& drawable) {
                updateTileDrawable(drawable, context, bucket, bucketPaintProperties, state);
            });
            continue;
        }

        float serialKey = 1.0f;
        auto addRenderables = [&, it = renderableSegments.begin()](const SymbolBucket::Buffer& buffer,
                                                                   const SymbolType type) mutable {
            for (auto& segment : buffer.segments) {
                const auto key = sortFeaturesByKey ? segment.sortKey : (serialKey += 1.0);
                assert(segment.vertexOffset + segment.vertexLength <= buffer.vertices.elements());
                it = renderableSegments.emplace_hint(
                    it, std::ref(segment), tile, renderData, bucketPaintProperties, key, type, tileID.overscaledZ);
            }
        };

        addRenderables(bucket.icon, SymbolType::IconRGBA);
        addRenderables(bucket.sdfIcon, SymbolType::IconSDF);
        addRenderables(bucket.text, SymbolType::Text);
    }

    using RawVertexVec = std::vector<std::uint8_t>; // <int16_t, 4>
    struct RawVertices {
        RawVertexVec text, icon;
    };
    std::unordered_map<UnwrappedTileID, RawVertices> rawVertices;

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
        const auto iconTextFit = layout.get<IconTextFit>();
        const bool variablePlacedIcon = bucket.hasVariablePlacement && iconTextFit != IconTextFitType::None;
        const auto symbolPlacement = layout.get<SymbolPlacement>();

        const auto buildVertices = [&](const SymbolBucket::Buffer& buffer_, RawVertexVec& dest) -> auto& {
            const std::vector<SymbolLayoutVertex>& src = buffer_.vertices.vector();
            const auto vertSize = sizeof(SymbolLayoutVertex::a1);
            if (dest.size() != vertSize * src.size()) {
                dest.resize(vertSize * src.size());
                for (std::size_t i = 0; i < src.size(); ++i) {
                    std::memcpy(&dest[vertSize * i], &src[i].a1, vertSize);
                }
            }
            return dest;
        };

        gfx::VertexAttributeArray attrs;
        if (auto& attr = attrs.getOrAdd(dataAttibName)) {
            const auto count = buffer.vertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, util::cast<float>(buffer.vertices.at(i).a2));
            }
        }
        if (auto& attr = attrs.getOrAdd(pixOffsetAttribName)) {
            const auto count = buffer.vertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, util::cast<float>(buffer.vertices.at(i).a3));
            }
        }
        if (auto& attr = attrs.getOrAdd(projPosAttribName)) {
            const auto count = buffer.dynamicVertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, util::cast<float>(buffer.dynamicVertices.at(i).a1));
            }
        }
        if (auto& attr = attrs.getOrAdd(fadeOpacityAttribName)) {
            const auto count = buffer.opacityVertices.elements();
            attr->reserve(count);
            for (auto i = 0ULL; i < count; ++i) {
                attr->set(i, buffer.opacityVertices.at(i).a1[0]);
            }
        }

        const auto uniformProps = isText ? attrs.readDataDrivenPaintProperties<TextOpacity,
                                                                               TextColor,
                                                                               TextHaloColor,
                                                                               TextHaloWidth,
                                                                               TextHaloBlur>(
                                               bucketPaintProperties.textBinders, evaluated)
                                         : attrs.readDataDrivenPaintProperties<IconOpacity,
                                                                               IconColor,
                                                                               IconHaloColor,
                                                                               IconHaloWidth,
                                                                               IconHaloBlur>(
                                               bucketPaintProperties.iconBinders, evaluated);

        const auto textHalo = evaluated.get<style::TextHaloColor>().constantOr(Color::black()).a > 0.0f &&
                              evaluated.get<style::TextHaloWidth>().constantOr(1);
        const auto textFill = evaluated.get<style::TextColor>().constantOr(Color::black()).a > 0.0f;

        const auto iconHalo = evaluated.get<style::IconHaloColor>().constantOr(Color::black()).a > 0.0f &&
                              evaluated.get<style::IconHaloWidth>().constantOr(1);
        const auto iconFill = evaluated.get<style::IconColor>().constantOr(Color::black()).a > 0.0f;

        const auto interpolateUBO = buildInterpUBO(isText, currentZoom, bucketPaintProperties);

        const auto draw = [&](const gfx::ShaderGroupPtr& shaderGroup,
                              [[maybe_unused]] const Segment<SymbolTextAttributes>& segment,
                              [[maybe_unused]] const gfx::IndexVector<gfx::Triangles>& indices,
                              [[maybe_unused]] const RawVertexVec& vertices,
                              std::size_t vertexCount,
                              [[maybe_unused]] bool isHalo,
                              const std::string_view suffix) {
            if (!shaderGroup) {
                return;
            }

            if (!builder) {
                builder = context.createDrawableBuilder(layerPrefix);
                builder->setSubLayerIndex(0);
                builder->setNeedsStencil(false);
                builder->setRenderPass(passes);
                builder->setCullFaceMode(gfx::CullFaceMode::disabled());
                builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                builder->setCullFaceMode(gfx::CullFaceMode::disabled());
                builder->setVertexAttrName(posOffsetAttribName);
            }

            builder->setDrawableName(layerPrefix + std::string(suffix));
            builder->setVertexAttributes(std::move(attrs));

            const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                shaderGroup->getOrCreateShader(context, uniformProps, posOffsetAttribName));
            if (!shader) {
                return;
            }
            builder->setShader(shader);

            if (const auto& atlases = tile.getAtlasTextures()) {
                if (const auto samplerLocation = shader->getSamplerLocation(texUniformName)) {
                    builder->setTextureSource([=]() {
                        return gfx::Drawable::Textures{{*samplerLocation, isText ? atlases->glyph : atlases->icon}};
                    });
                }
                if (const auto samplerLocation = shader->getSamplerLocation(iconTexUniformName)) {
                    builder->setTextureSource([=]() {
                        return gfx::Drawable::Textures{{*samplerLocation, atlases->icon}};
                    });
                }
            }

            auto raw = vertices;
            builder->setRawVertices(std::move(raw), vertexCount, gfx::AttributeDataType::Short4);
            builder->setSegments(gfx::Triangles(), indices.vector(), &segment, 1);

            builder->flush();

            const auto tileUBO = buildTileUBO(bucket, renderable.type, values.pitchAlignment, isHalo, currentZoom);

            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);

                drawable->setData(std::make_unique<gfx::SymbolDrawableData>(
                    /*.isHalo=*/isHalo,
                    /*.hasVariablePlacement=*/variablePlacedIcon,
                    /*.symbolType=*/renderable.type,
                    /*.pitchAlignment=*/values.pitchAlignment,
                    /*.rotationAlignment=*/values.rotationAlignment,
                    /*.placement=*/symbolPlacement,
                    /*.textFit=*/iconTextFit));

                auto& uniforms = drawable->mutableUniformBuffers();
                uniforms.createOrUpdate(SymbolLayerTweaker::SymbolDrawableTilePropsUBOName, &tileUBO, context);
                uniforms.createOrUpdate(SymbolLayerTweaker::SymbolDrawableInterpolateUBOName, &interpolateUBO, context);

                tileLayerGroup->addDrawable(passes, tileID, std::move(drawable));
                ++stats.tileDrawablesAdded;
            }
        };

        if (isText) {
            const auto& vertices = buildVertices(buffer, rawVertices[tile.id].text);
            const auto vertexCount = buffer.vertices.elements();
            const auto& indices = buffer.triangles;
            if (bucket.iconsInText) {
                if (textHalo) {
                    draw(symbolTextAndIconGroup,
                         renderable.segment,
                         indices,
                         vertices,
                         vertexCount,
                         /* isHalo = */ true,
                         "halo");
                }

                if (textFill) {
                    draw(symbolTextAndIconGroup,
                         renderable.segment,
                         indices,
                         vertices,
                         vertexCount,
                         /* isHalo = */ false,
                         "fill");
                }
            } else {
                if (textHalo) {
                    draw(symbolSDFTextGroup,
                         renderable.segment,
                         indices,
                         vertices,
                         vertexCount,
                         /* isHalo = */ true,
                         "halo");
                }

                if (textFill) {
                    draw(symbolSDFTextGroup,
                         renderable.segment,
                         indices,
                         vertices,
                         vertexCount,
                         /* isHalo = */ false,
                         "fill");
                }
            }
        } else { // icons
            const auto& vertices = buildVertices(buffer, rawVertices[tile.id].icon);
            const auto vertexCount = buffer.vertices.elements();
            const auto& indices = buffer.triangles;
            if (sdfIcons) {
                if (iconHalo) {
                    draw(symbolSDFIconGroup,
                         renderable.segment,
                         indices,
                         vertices,
                         vertexCount,
                         /* isHalo = */ true,
                         "halo");
                }

                if (iconFill) {
                    draw(symbolSDFIconGroup,
                         renderable.segment,
                         indices,
                         vertices,
                         vertexCount,
                         /* isHalo = */ false,
                         "fill");
                }
            } else {
                draw(symbolIconGroup, renderable.segment, indices, vertices, vertexCount, /* isHalo = */ false, "icon");
            }
        }
    }
}

} // namespace mbgl
