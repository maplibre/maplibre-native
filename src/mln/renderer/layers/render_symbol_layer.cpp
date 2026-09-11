#include <mln/renderer/layers/render_symbol_layer.hpp>

#include <mln/gfx/collision_drawable_data.hpp>
#include <mln/gfx/cull_face_mode.hpp>
#include <mln/gfx/drawable_atlases_tweaker.hpp>
#include <mln/gfx/drawable_builder.hpp>
#include <mln/gfx/shader_registry.hpp>
#include <mln/gfx/symbol_drawable_data.hpp>
#include <mln/layout/symbol_layout.hpp>
#include <mln/layout/symbol_projection.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/renderer/buckets/symbol_bucket.hpp>
#include <mln/renderer/layer_group.hpp>
#include <mln/renderer/layers/collision_layer_tweaker.hpp>
#include <mln/renderer/layers/symbol_layer_tweaker.hpp>
#include <mln/renderer/paint_parameters.hpp>
#include <mln/renderer/property_evaluation_parameters.hpp>
#include <mln/renderer/render_source.hpp>
#include <mln/renderer/render_static_data.hpp>
#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/render_tree.hpp>
#include <mln/renderer/tile_render_data.hpp>
#include <mln/renderer/update_parameters.hpp>
#include <mln/renderer/upload_parameters.hpp>
#include <mln/shaders/shader_program_base.hpp>
#include <mln/shaders/symbol_layer_ubo.hpp>
#include <mln/style/layers/symbol_layer_impl.hpp>
#include <mln/text/shaping.hpp>
#include <mln/tile/geometry_tile_data.hpp>
#include <mln/tile/geometry_tile.hpp>
#include <mln/tile/tile.hpp>
#include <mln/util/convert.hpp>
#include <mln/util/mat2.hpp>
#include <mln/util/mat4.hpp>
#include <mln/util/math.hpp>

#include <cmath>
#include <set>

namespace mln {

using namespace style;
using namespace shaders;

namespace {

constexpr std::string_view SymbolIconShaderName = "SymbolIconShader";
constexpr std::string_view SymbolSDFShaderName = "SymbolSDFShader";
constexpr std::string_view SymbolTextAndIconShaderName = "SymbolTextAndIconShader";
constexpr std::string_view CollisionBoxShaderName = "CollisionBoxShader";
constexpr std::string_view CollisionCircleShaderName = "CollisionCircleShader";

style::SymbolPropertyValues iconPropertyValues(const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_,
                                               const style::SymbolLayoutProperties::PossiblyEvaluated& layout_) {
    return style::SymbolPropertyValues{
        .pitchAlignment = layout_.get<style::IconPitchAlignment>(),
        .rotationAlignment = layout_.get<style::IconRotationAlignment>(),
        .keepUpright = layout_.get<style::IconKeepUpright>(),
        .translate = evaluated_.get<style::IconTranslate>(),
        .translateAnchor = evaluated_.get<style::IconTranslateAnchor>(),
        .hasHalo = evaluated_.get<style::IconHaloColor>().constantOr(Color::black()).a > 0 &&
                   evaluated_.get<style::IconHaloWidth>().constantOr(1) != 0,
        .hasFill = evaluated_.get<style::IconColor>().constantOr(Color::black()).a > 0};
}

style::SymbolPropertyValues textPropertyValues(const style::SymbolPaintProperties::PossiblyEvaluated& evaluated_,
                                               const style::SymbolLayoutProperties::PossiblyEvaluated& layout_) {
    return style::SymbolPropertyValues{
        .pitchAlignment = layout_.get<style::TextPitchAlignment>(),
        .rotationAlignment = layout_.get<style::TextRotationAlignment>(),
        .keepUpright = layout_.get<style::TextKeepUpright>(),
        .translate = evaluated_.get<style::TextTranslate>(),
        .translateAnchor = evaluated_.get<style::TextTranslateAnchor>(),
        .hasHalo = evaluated_.get<style::TextHaloColor>().constantOr(Color::black()).a > 0 &&
                   evaluated_.get<style::TextHaloWidth>().constantOr(1) != 0,
        .hasFill = evaluated_.get<style::TextColor>().constantOr(Color::black()).a > 0};
}

using SegmentWrapper = std::reference_wrapper<const SegmentBase>;
using SegmentVectorWrapper = std::reference_wrapper<const SegmentVector>;
using SegmentsWrapper = variant<SegmentWrapper, SegmentVectorWrapper>;

struct RenderableSegment {
    RenderableSegment(SegmentWrapper segment_,
                      const RenderTile& tile_,
                      const LayerRenderData& renderData_,
                      const SymbolBucket::PaintProperties& bucketPaintProperties_,
                      float sortKey_,
                      const SymbolType type_)
        : segment(segment_),
          tile(tile_),
          renderData(renderData_),
          bucketPaintProperties(bucketPaintProperties_),
          sortKey(sortKey_),
          type(type_),
          overscaledZ(tile.getOverscaledTileID().overscaledZ) {}

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

struct SegmentGroup {
    // A reference to the first or only segment
    RenderableSegment renderable;
    // A reference to multiple segments, or none
    SegmentVectorWrapper segments;

    bool operator<(const SegmentGroup& other) const { return renderable < other.renderable; }
};

inline const SymbolLayer::Impl& impl_cast(const Immutable<style::Layer::Impl>& impl) {
    assert(impl->getTypeInfo() == SymbolLayer::Impl::staticTypeInfo());
    return static_cast<const SymbolLayer::Impl&>(*impl);
}

} // namespace

RenderSymbolLayer::RenderSymbolLayer(Immutable<style::SymbolLayer::Impl> _impl)
    : RenderLayer(makeMutable<SymbolLayerProperties>(std::move(_impl))),
      unevaluated(impl_cast(baseImpl).paint.untransitioned()) {
    styleDependencies = unevaluated.getDependencies();
}

RenderSymbolLayer::~RenderSymbolLayer() = default;

void RenderSymbolLayer::transition(const TransitionParameters& parameters) {
    unevaluated = impl_cast(baseImpl).paint.transitioned(parameters, std::move(unevaluated));
    hasFormatSectionOverrides = SymbolLayerPaintPropertyOverrides::hasOverrides(
        impl_cast(baseImpl).layout.get<TextField>());
    styleDependencies = unevaluated.getDependencies();
}

void RenderSymbolLayer::evaluate(const PropertyEvaluationParameters& parameters) {
    const auto previousProperties = staticImmutableCast<SymbolLayerProperties>(evaluatedProperties);
    auto properties = makeMutable<SymbolLayerProperties>(
        staticImmutableCast<SymbolLayer::Impl>(baseImpl),
        unevaluated.evaluate(parameters, previousProperties->evaluated));

    auto& evaluated = properties->evaluated;
    const auto& layout = impl_cast(baseImpl).layout;

    if (hasFormatSectionOverrides) {
        SymbolLayerPaintPropertyOverrides::setOverrides(layout, evaluated);
    }

    const auto hasIconOpacity = evaluated.get<style::IconColor>().constantOr(IconColor::defaultValue()).a > 0 ||
                                evaluated.get<style::IconHaloColor>().constantOr(IconHaloColor::defaultValue()).a > 0;
    const auto hasTextOpacity = evaluated.get<style::TextColor>().constantOr(TextColor::defaultValue()).a > 0 ||
                                evaluated.get<style::TextHaloColor>().constantOr(TextHaloColor::defaultValue()).a > 0;

    passes = ((evaluated.get<style::IconOpacity>().constantOr(1) > 0 && hasIconOpacity && iconSize > 0) ||
              (evaluated.get<style::TextOpacity>().constantOr(1) > 0 && hasTextOpacity && textSize > 0))
                 ? RenderPass::Translucent
                 : RenderPass::None;
    properties->renderPasses = mln::underlying_type(passes);
    evaluatedProperties = std::move(properties);

    // The symbol tweaker supports updating properties.
    // When the style changes, it will be replaced in `RenderLayer::layerChanged`
    if (layerTweaker) {
        layerTweaker->updateProperties(evaluatedProperties);
    }
}

bool RenderSymbolLayer::hasTransition() const {
    return unevaluated.hasTransition();
}

bool RenderSymbolLayer::hasCrossfade() const {
    return false;
}

void RenderSymbolLayer::prepare(const LayerPrepareParameters& params) {
    renderTiles = params.source->getRenderTilesSortedByYPosition();

    updateRenderTileIDs();
    addRenderPassesFromTiles();

    placementData.clear();

    for (const RenderTile& renderTile : *renderTiles) {
        auto* bucket = static_cast<SymbolBucket*>(renderTile.getBucket(*baseImpl));
        if (bucket && bucket->bucketLeaderID == getID() && static_cast<Bucket*>(bucket)->check(SYM_GUARD_LOC)) {
            // Only place this layer if it's the "group leader" for the bucket
            const Tile* tile = params.source->getRenderedTile(renderTile.id);
            assert(tile);
            assert(tile->kind == Tile::Kind::Geometry);

            auto featureIndex = static_cast<const GeometryTile*>(tile)->getFeatureIndex();

            if (bucket->sortKeyRanges.empty()) {
                placementData.push_back({.bucket = *bucket,
                                         .tile = renderTile,
                                         .featureIndex = featureIndex,
                                         .sourceId = baseImpl->source,
                                         .sortKeyRange = std::nullopt});
            } else {
                for (const auto& sortKeyRange : bucket->sortKeyRanges) {
                    BucketPlacementData layerData{.bucket = *bucket,
                                                  .tile = renderTile,
                                                  .featureIndex = featureIndex,
                                                  .sourceId = baseImpl->source,
                                                  .sortKeyRange = sortKeyRange};
                    auto sortPosition = std::upper_bound( // NOLINT(modernize-use-ranges)
                        placementData.cbegin(),
                        placementData.cend(),
                        layerData,
                        [](const auto& lhs, const auto& rhs) {
                            assert(lhs.sortKeyRange && rhs.sortKeyRange);
                            return lhs.sortKeyRange->sortKey < rhs.sortKeyRange->sortKey;
                        });
                    placementData.insert(sortPosition, std::move(layerData));
                }
            }
        }
    }
}

namespace {
const SegmentVector emptySegmentVector;
constexpr auto posOffsetAttribName = "a_pos_offset";
[[maybe_unused]] constexpr auto sortedInstanceUniformName = "sorted_instance";

void updateTileAttributes(const SymbolBucket::Buffer& buffer,
                          const bool isText,
                          const SymbolBucket::PaintProperties& paintProps,
                          const SymbolPaintProperties::PossiblyEvaluated& evaluated,
                          gfx::VertexAttributeArray& attribs,
                          StringIDSetsPair* propertiesAsUniforms) {
    if (isText) {
        attribs.readDataDrivenPaintProperties<TextOpacity, TextColor, TextHaloColor, TextHaloWidth, TextHaloBlur>(
            paintProps.textBinders, evaluated, propertiesAsUniforms, idSymbolOpacityAttribute);
    } else {
        attribs.readDataDrivenPaintProperties<IconOpacity, IconColor, IconHaloColor, IconHaloWidth, IconHaloBlur>(
            paintProps.iconBinders, evaluated, propertiesAsUniforms, idSymbolOpacityAttribute);
    }

#if MLN_USE_SYMBOL_INSTANCING
    if (!buffer.sortedInstances().empty()) {
        if (const auto& attr = attribs.set(idSymbolSortedInstanceAttribute)) {
            attr->setSharedRawData(buffer.sharedSortedInstances,
                                   offsetof(SymbolSortedInstance, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(SymbolSortedInstance),
                                   gfx::AttributeDataType::UShort);
        }
    } else if (propertiesAsUniforms) {
        propertiesAsUniforms->first.emplace(sortedInstanceUniformName);
        propertiesAsUniforms->second.emplace(idSymbolSortedInstanceAttribute);
    }
    if (const auto& attr = attribs.set(idSymbolPosScaleAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a1),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.set(idSymbolOffsetTlTrAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a2),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.set(idSymbolOffsetBlBrAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a3),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.set(idSymbolTextureRectAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a4),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::UShort4);
    }
    if (const auto& attr = attribs.set(idSymbolPixelOffsetAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a5),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.set(idSymbolSizeSdfAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a6),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::UShort2);
    }
#else
    if (const auto& attr = attribs.set(idSymbolPosOffsetAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a1),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.set(idSymbolDataAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a2),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::UShort4);
    }
    if (const auto& attr = attribs.set(idSymbolPixelOffsetAttribute)) {
        attr->setSharedRawData(buffer.sharedAttributeData,
                               offsetof(SymbolLayoutAttributes, a3),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutAttributes),
                               gfx::AttributeDataType::Short4);
    }
#endif

    if (const auto& attr = attribs.set(idSymbolProjectedPosAttribute)) {
        attr->setSharedRawData(buffer.sharedDynamicAttributeData,
                               offsetof(SymbolDynamicLayoutAttributes, a1),
                               /*vertexOffset=*/0,
                               sizeof(SymbolDynamicLayoutAttributes),
                               gfx::AttributeDataType::Float3);
    }
    if (const auto& attr = attribs.set(idSymbolFadeOpacityAttribute)) {
        attr->setSharedRawData(buffer.sharedOpacityAttributeData,
                               offsetof(SymbolOpacityAttributes, a1),
                               /*vertexOffset=*/0,
                               sizeof(SymbolOpacityAttributes),
                               gfx::AttributeDataType::Float);
    }
}

void updateTileDrawable(gfx::Drawable& drawable,
                        const SymbolBucket& bucket,
                        const SymbolBucket::PaintProperties& paintProps,
                        const SymbolPaintProperties::PossiblyEvaluated& evaluated) {
    if (!drawable.getData()) {
        return;
    }

    auto& drawData = static_cast<gfx::SymbolDrawableData&>(*drawable.getData());
    const auto isText = (drawData.symbolType == SymbolType::Text);
    const auto sdfIcons = (drawData.symbolType == SymbolType::IconSDF);

    // This property can be set after the initial appearance of the tile, as part of the layout process.
    drawData.bucketVariablePlacement = bucket.hasVariablePlacement;

    const auto& buffer = isText ? bucket.text : (sdfIcons ? bucket.sdfIcon : bucket.icon);

#if MLN_USE_SYMBOL_INSTANCING
    if (auto& instanceAttribs = drawable.getInstanceAttributes()) {
        updateTileAttributes(buffer, isText, paintProps, evaluated, *instanceAttribs, nullptr);
    }
#else
    const auto vertexCount = buffer.attributeData().elements();
    drawable.setVertices({}, vertexCount, gfx::AttributeDataType::Short4);

    // TODO: detect whether anything has actually changed
    // See `Placement::updateBucketDynamicAttributeData`

    if (auto& attribs = drawable.getVertexAttributes()) {
        updateTileAttributes(buffer, isText, paintProps, evaluated, *attribs, nullptr);
    }
#endif
}

gfx::VertexAttributeArrayPtr getCollisionVertexAttributes(gfx::Context& context,
                                                          const SymbolBucket::CollisionBuffer& buffer) {
    auto vertexAttrs = context.createVertexAttributeArray();

    if (const auto& attr = vertexAttrs->set(idCollisionPosVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(CollisionBoxLayoutVertexAttributes, a1),
                               /*vertexOffset=*/0,
                               sizeof(CollisionBoxLayoutVertexAttributes),
                               gfx::AttributeDataType::Short2);
    }
    if (const auto& attr = vertexAttrs->set(idCollisionAnchorPosVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(CollisionBoxLayoutVertexAttributes, a2),
                               /*vertexOffset=*/0,
                               sizeof(CollisionBoxLayoutVertexAttributes),
                               gfx::AttributeDataType::Short2);
    }
    if (const auto& attr = vertexAttrs->set(idCollisionExtrudeVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(CollisionBoxLayoutVertexAttributes, a3),
                               /*vertexOffset=*/0,
                               sizeof(CollisionBoxLayoutVertexAttributes),
                               gfx::AttributeDataType::Short2);
    }

    if (const auto& attr = vertexAttrs->set(idCollisionPlacedVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedDynamicVertices,
                               offsetof(CollisionBoxDynamicVertexAttributes, a1),
                               /*vertexOffset=*/0,
                               sizeof(CollisionBoxDynamicVertexAttributes),
                               gfx::AttributeDataType::UShort2);
    }
    if (const auto& attr = vertexAttrs->set(idCollisionShiftVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedDynamicVertices,
                               offsetof(CollisionBoxDynamicVertexAttributes, a2),
                               /*vertexOffset=*/0,
                               sizeof(CollisionBoxDynamicVertexAttributes),
                               gfx::AttributeDataType::Float2);
    }

    return vertexAttrs;
}

} // namespace

void RenderSymbolLayer::markLayerRenderable(bool willRender, UniqueChangeRequestVec& changes) {
    RenderLayer::markLayerRenderable(willRender, changes);
    if (collisionTileLayerGroup) {
        activateLayerGroup(collisionTileLayerGroup, willRender, changes);
    }
}

void RenderSymbolLayer::layerRemoved(UniqueChangeRequestVec& changes) {
    RenderLayer::layerRemoved(changes);
    if (collisionTileLayerGroup) {
        activateLayerGroup(collisionTileLayerGroup, false, changes);
    }
}

void RenderSymbolLayer::layerIndexChanged(int32_t newLayerIndex, UniqueChangeRequestVec& changes) {
    RenderLayer::layerIndexChanged(newLayerIndex, changes);

    changeLayerIndex(collisionTileLayerGroup, newLayerIndex, changes);
}

std::size_t RenderSymbolLayer::removeTile(RenderPass renderPass, const OverscaledTileID& tileID) {
    const auto oldValue = stats.drawablesRemoved;
    if (const auto tileGroup = static_cast<TileLayerGroup*>(layerGroup.get())) {
        stats.drawablesRemoved += tileGroup->removeDrawables(renderPass, tileID).size();
    }
    if (collisionTileLayerGroup) {
        stats.drawablesRemoved += collisionTileLayerGroup->removeDrawables(renderPass, tileID).size();
    }
    return stats.drawablesRemoved - oldValue;
}

std::size_t RenderSymbolLayer::removeAllDrawables() {
    const auto oldValue = stats.drawablesRemoved;
    if (layerGroup) {
        stats.drawablesRemoved += layerGroup->getDrawableCount();
        layerGroup->clearDrawables();
    }
    if (collisionTileLayerGroup) {
        stats.drawablesRemoved += collisionTileLayerGroup->getDrawableCount();
        collisionTileLayerGroup->clearDrawables();
    }
    return stats.drawablesRemoved - oldValue;
}

namespace {
mat4 getScreenMatrix(vec2f translation) {
    mat4 m;
    matrix::ortho(m, 0, util::EXTENT, -util::EXTENT, 0, 0, 1);
    matrix::translate(m, m, 0, -util::EXTENT, 0);
    matrix::translate(m, m, translation[0], translation[1], 0);
    return m;
}
} // namespace

void RenderSymbolLayer::captureRenderedFeatures(const RenderTile& tile,
                                                const SymbolBucket& bucket,
                                                const style::SymbolPaintProperties::PossiblyEvaluated& evaluated,
                                                const TransformState& state,
                                                const TransformParameters& transformParams) {
    if (!bucket.hasData()) {
        return;
    }

    const auto& iconOpacity = evaluated.get<IconOpacity>();
    const auto& iconColor = evaluated.get<IconColor>();
    const auto& iconHaloColor = evaluated.get<IconHaloColor>();
    const auto& textOpacity = evaluated.get<TextOpacity>();
    const auto& textColor = evaluated.get<TextColor>();
    const auto& textHaloColor = evaluated.get<TextHaloColor>();

    const auto visibleText = bucket.hasTextData() && (textOpacity.constantOr(TextOpacity::defaultValue()) > 0) &&
                             (textColor.constantOr(TextColor::defaultValue()).a > 0 ||
                              textHaloColor.constantOr(TextHaloColor::defaultValue()).a > 0);
    const auto visibleIcons = (bucket.hasIconData() || bucket.hasSdfIconData()) &&
                              (iconOpacity.constantOr(IconOpacity::defaultValue()) > 0) &&
                              (iconColor.constantOr(IconColor::defaultValue()).a > 0 ||
                               iconHaloColor.constantOr(IconHaloColor::defaultValue()).a > 0);
    if (!visibleText && !visibleIcons) {
        return;
    }

    const auto& bucketPaintProperties = bucket.paintProperties.at(getID());
    const auto& iconBinders = bucketPaintProperties.iconBinders;
    const auto& textBinders = bucketPaintProperties.textBinders;

    const auto& tileID = tile.getOverscaledTileID().toUnwrapped();

    constexpr bool inViewportPixelUnits = false;
    constexpr bool nearClipped = false;
    constexpr bool aligned = false;
    constexpr bool is3d = false;
    constexpr bool enableDepth = true;
    constexpr std::int32_t subLayerIndex = 1;

    const auto textTranslation = evaluated.get<TextTranslate>();
    const auto iconTranslation = evaluated.get<IconTranslate>();
    const auto textTranslationAnchor = evaluated.get<TextTranslateAnchor>();
    const auto iconTranslationAnchor = evaluated.get<IconTranslateAnchor>();
    const std::optional<mln::Point<double>> origin = std::nullopt;
    const auto zoom = state.getZoom();
    const auto zoomFraction = state.getZoomFraction();
    const float pixelsToTileUnits = tileID.pixelsToTileUnits(1.f, zoom);
    const auto cameraToCenterDistance = state.getCameraToCenterDistance();

    const auto& bucketLayout = *bucket.layout;
    const auto placement = bucketLayout.get<SymbolPlacement>();
    const auto textFit = bucketLayout.get<IconTextFit>();

    const auto& layerProperties = static_cast<const SymbolLayerProperties&>(*evaluatedProperties);
    const auto& layoutProperties = layerProperties.layerImpl().layout;
    const auto screenSpaceProp = layoutProperties.get<SymbolScreenSpace>();
    const bool isScreenSpace = screenSpaceProp.isConstant() ? screenSpaceProp.asConstant()
                                                            : SymbolScreenSpace::defaultValue();
    const bool isOffset = !layoutProperties.get<IconOffset>().isUndefined();

    const auto getTileMatrix = [&](const auto& translation, const auto& translationAnchor) {
        return LayerTweaker::getTileMatrix(tileID,
                                           state,
                                           transformParams,
                                           layerIndex,
                                           translation,
                                           translationAnchor,
                                           origin,
                                           is3d,
                                           enableDepth,
                                           subLayerIndex,
                                           nearClipped,
                                           inViewportPixelUnits,
                                           aligned);
    };
    const mat4 textDrawableMatrix = isScreenSpace ? getScreenMatrix(textTranslation)
                                                  : getTileMatrix(textTranslation, textTranslationAnchor);
    const mat4 iconDrawableMatrix = isScreenSpace ? getScreenMatrix(iconTranslation)
                                                  : getTileMatrix(iconTranslation, iconTranslationAnchor);

    const auto computeBufferBounds = [&](const SymbolBucket::Buffer& buffer, bool isText) {
        const auto values = isText ? textPropertyValues(evaluated, bucketLayout)
                                   : iconPropertyValues(evaluated, bucketLayout);
        const bool pitchWithMap = values.pitchAlignment == style::AlignmentType::Map;
        const bool rotateWithMap = values.rotationAlignment == style::AlignmentType::Map;
        const bool alongLine = placement != SymbolPlacementType::Point &&
                               values.rotationAlignment == AlignmentType::Map;
        const bool hasVariablePlacement = bucket.hasVariablePlacement && (isText || textFit != IconTextFitType::None);

        const auto& sizeBinder = isText ? bucket.textSizeBinder : bucket.iconSizeBinder;
        const auto evaluatedSize = sizeBinder->evaluateForZoom(zoom);
        const auto u_size = evaluatedSize.size;
        const auto u_size_t = evaluatedSize.sizeT;

        const mat4 textLabelPlaneMatrix =
            (alongLine || hasVariablePlacement)
                ? matrix::identity4()
                : getLabelPlaneMatrix(textDrawableMatrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);
        const mat4 iconLabelPlaneMatrix =
            (alongLine || hasVariablePlacement)
                ? matrix::identity4()
                : getLabelPlaneMatrix(iconDrawableMatrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);
        const mat4 textGLCoordMatrix = getGlCoordMatrix(
            textDrawableMatrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);
        const mat4 iconGLCoordMatrix = getGlCoordMatrix(
            iconDrawableMatrix, pitchWithMap, rotateWithMap, state, pixelsToTileUnits);

        const bool rotateInShader = rotateWithMap && !pitchWithMap && !alongLine;

        const auto& staticVertices = buffer.attributeData();
        const auto& dynamicVertices = buffer.dynamicAttributeData();

#if MLN_USE_SYMBOL_INSTANCING
        // When instancing, each entry in the attribute vectors describes a whole quad,
        // so the running offset counts quads rather than individual corner vertices.
        std::size_t startIndex = 0;
#endif

        for (const auto& symbol : buffer.placedSymbols) {
            if (symbol.hidden || !symbol.featureId) {
                continue;
            }

#if MLN_USE_SYMBOL_INSTANCING
            const auto quadCount = symbol.glyphOffsets.size();
            const auto vertexCount = quadCount * 4;
#else
            const auto startIndex = symbol.startIndex;
            const auto vertexCount = symbol.glyphOffsets.size() * 4;
#endif

            // Consider the dynamic opacity and color of the first vertex.  If either is zero, skip the symbol.
            const auto& opacityProperty = isText ? textOpacity : iconOpacity;
            if (!opacityProperty.isConstant()) {
                const auto& opacityBinder = isText ? textBinders.get<TextOpacity>() : iconBinders.get<IconOpacity>();
                const auto [vertex] = opacityBinder->getVertexValue(startIndex);
                if (mln::util::interpolate(vertex.a1[0], vertex.a1[1], zoomFraction) == 0) {
                    continue;
                }
            }
            const auto& colorProperty = isText ? textColor : iconColor;
            if (!colorProperty.isConstant()) {
                const auto& colorBinder = isText ? textBinders.get<TextColor>() : iconBinders.get<IconColor>();
                const auto [vertex] = colorBinder->getVertexValue(startIndex);
                if (unpack_mix_alpha(vertex.a1, zoomFraction) == 0) {
                    continue;
                }
            }

            const auto getVertex = [&](std::size_t i) {
                using namespace matrix;
                using namespace vector;

#if MLN_USE_SYMBOL_INSTANCING
                // A single instance covers all four corners of the quad, which the shader
                // selects with the static vertex position, one of (0,0), (1,0), (0,1), (1,1).
                const auto cornerX = i & 1;
                const auto cornerY = (i >> 1) & 1;

                // Select the correct instance for this vertex
                const auto& staticVertex = staticVertices.at(startIndex + i / 4);
                const auto& dynamicVertex = dynamicVertices.at(startIndex + i / 4);

                // `offset_tltr` holds the top corners, `offset_blbr` the bottom ones.
                const auto& corners = cornerY ? staticVertex.a3 : staticVertex.a2;

                const vec2 a_pos = {static_cast<double>(staticVertex.a1[0]), static_cast<double>(staticVertex.a1[1])};
                const vec2 a_offset = {static_cast<double>(corners[2 * cornerX]),
                                       static_cast<double>(corners[2 * cornerX + 1])};
                const vec2 a_size = {static_cast<double>(staticVertex.a6[0]), static_cast<double>(staticVertex.a6[1])};
                // `pixeloffset` holds the top-left and bottom-right offsets, interpolated per corner.
                const vec2 a_pxoffset = {
                    staticVertex.a5[0] + static_cast<double>(cornerX) * (staticVertex.a5[2] - staticVertex.a5[0]),
                    staticVertex.a5[1] + static_cast<double>(cornerY) * (staticVertex.a5[3] - staticVertex.a5[1])};
                const vec2 a_minFontScale = {staticVertex.a1[2] / 256.0, staticVertex.a1[3] / 256.0};
#else
                const auto& staticVertex = staticVertices.at(startIndex + i);
                const auto& dynamicVertex = dynamicVertices.at(startIndex + i);

                const vec2 a_pos = {static_cast<double>(staticVertex.a1[0]), static_cast<double>(staticVertex.a1[1])};
                const vec2 a_offset = {static_cast<double>(staticVertex.a1[2]),
                                       static_cast<double>(staticVertex.a1[3])};
                const vec2 a_size = {static_cast<double>(staticVertex.a2[2]), static_cast<double>(staticVertex.a2[3])};
                const vec2 a_pxoffset = {static_cast<double>(staticVertex.a3[0]),
                                         static_cast<double>(staticVertex.a3[1])};
                const vec2 a_minFontScale = {staticVertex.a3[2] / 256.0, staticVertex.a3[3] / 256.0};
#endif
                const auto a_size_min = floor(a_size[0] / 2);
                const vec2 in_projected_pos = {dynamicVertex.a1[0], dynamicVertex.a1[1]};
                const auto segment_angle = -dynamicVertex.a1[2];
                const auto& drawableMatrix = isText ? textDrawableMatrix : iconDrawableMatrix;
                const auto& labelPlaneMatrix = isText ? textLabelPlaneMatrix : iconLabelPlaneMatrix;
                const auto& glCoordMatrix = isText ? textGLCoordMatrix : iconGLCoordMatrix;
                const vec4 projectedPoint = matrix::transformMat4({a_pos[0], a_pos[1], 0, 1}, drawableMatrix);
                const auto camera_to_anchor_distance = projectedPoint[3];
                const auto aspect_ratio = state.getSize().aspectRatio();

                const float distance_ratio = pitchWithMap ? camera_to_anchor_distance / cameraToCenterDistance
                                                          : cameraToCenterDistance / camera_to_anchor_distance;
                const float perspective_ratio = util::clamp(0.5 + 0.5 * distance_ratio, 0.0, 4.0);

                float size;
                if (!evaluatedSize.isZoomConstant && !evaluatedSize.isFeatureConstant) {
                    size = util::interpolate(a_size_min, a_size[1], u_size_t) / 128.0;
                } else if (evaluatedSize.isZoomConstant && !evaluatedSize.isFeatureConstant) {
                    size = a_size_min / 128.0;
                } else {
                    size = u_size;
                }
                if (!isOffset) {
                    size *= perspective_ratio;
                }

                const auto fontScale = isText ? size / 24.0 : size;

                float symbol_rotation = 0.0;
                if (rotateInShader) {
                    const vec4 offsetProjectedPoint = drawableMatrix * vec(a_pos + vec2{1, 0}, 0, 1);
                    const vec2 a = slice<0, 2>(projectedPoint) / projectedPoint[3];
                    const vec2 b = slice<0, 2>(offsetProjectedPoint) / offsetProjectedPoint[3];
                    symbol_rotation = std::atan2((b[1] - a[1]) / aspect_ratio, b[0] - a[0]);
                }

                const auto angle_sin = std::sin(segment_angle + symbol_rotation);
                const auto angle_cos = std::cos(segment_angle + symbol_rotation);
                const mat2 rotation_matrix{angle_cos, -1.0 * angle_sin, angle_sin, angle_cos};

                const vec4 projected_pos = labelPlaneMatrix * vec(in_projected_pos, 0, 1);
                const vec2 pos0 = {projected_pos[0] / projected_pos[3], projected_pos[1] / projected_pos[3]};
                const vec2 posOffset = a_offset * max(a_minFontScale, fontScale) / 32.0 + a_pxoffset / 16.0;

                const vec4 outPos = glCoordMatrix * vec(pos0 + rotation_matrix * posOffset, 0.0, 1.0);
                return slice<0, 3>(outPos) / outPos[3];
            };
            if (const auto bound = computeFeatureNDCBound(vertexCount, getVertex)) {
                stats.addRenderedFeature(*symbol.featureId, *bound, {tile.getOverscaledTileID()});
            }
#if MLN_USE_SYMBOL_INSTANCING
            startIndex += quadCount;
#endif
        }
    };

    const bool iconsVisible = evaluated.get<style::IconOpacity>().constantOr(1) > 0 &&
                              (evaluated.get<style::IconColor>().constantOr(IconColor::defaultValue()).a > 0 ||
                               evaluated.get<style::IconHaloColor>().constantOr(IconHaloColor::defaultValue()).a > 0);
    const bool textVisible = evaluated.get<style::TextOpacity>().constantOr(1) > 0 &&
                             (evaluated.get<style::TextColor>().constantOr(TextColor::defaultValue()).a > 0 ||
                              evaluated.get<style::TextHaloColor>().constantOr(TextHaloColor::defaultValue()).a > 0);

    if (bucket.hasIconData() && iconsVisible) {
        computeBufferBounds(bucket.icon, /* isText = */ false);
    }
    if (bucket.hasTextData() && textVisible) {
        computeBufferBounds(bucket.text, /* isText = */ true);
    }
    if (bucket.hasSdfIconData() && (textVisible || iconsVisible)) {
        computeBufferBounds(bucket.sdfIcon, /* isText = */ false);
    }
}

void RenderSymbolLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& state,
                               const std::shared_ptr<UpdateParameters>& updateParameters,
                               const PaintParameters&,
                               const RenderTree& renderTree,
                               UniqueChangeRequestVec& changes) {
    stats.renderedFeatures.clear();

    if (!renderTiles || renderTiles->empty() || passes == RenderPass::None) {
        removeAllDrawables();
        return;
    }

    // Set up a layer group
    if (!layerGroup) {
        if (auto layerGroup_ = context.createTileLayerGroup(layerIndex, /*initialCapacity=*/64, getID())) {
            setLayerGroup(std::move(layerGroup_), changes);
        }
    }

    if (!layerTweaker) {
        layerTweaker = std::make_shared<SymbolLayerTweaker>(getID(), evaluatedProperties);
        layerGroup->addLayerTweaker(layerTweaker);
    }

    const auto& getCollisionTileLayerGroup = [&] {
        if (!collisionTileLayerGroup) {
            collisionTileLayerGroup = context.createTileLayerGroup(
                layerIndex, /*initialCapacity=*/64, getID() + "-collision");
            if (collisionTileLayerGroup) {
                activateLayerGroup(collisionTileLayerGroup, true, changes);
            }
        }
        if (!collisionLayerTweaker) {
            collisionLayerTweaker = std::make_shared<CollisionLayerTweaker>(getID(), evaluatedProperties);
            collisionTileLayerGroup->addLayerTweaker(collisionLayerTweaker);
        }
        return collisionTileLayerGroup;
    };

    if (!symbolIconGroup) {
        symbolIconGroup = shaders.getShaderGroup(std::string(SymbolIconShaderName));
    }
    if (!symbolSDFGroup) {
        symbolSDFGroup = shaders.getShaderGroup(std::string(SymbolSDFShaderName));
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

#if MLN_USE_SYMBOL_INSTANCING
    if (!staticDataVertices) {
        staticDataVertices = std::make_shared<SymbolVertexVector>(RenderStaticData::symbolVertices());
    }
    if (!staticDataIndices) {
        staticDataIndices = std::make_shared<TriangleIndexVector>(RenderStaticData::symbolTriangleIndices());
    }
#endif

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
    if (collisionTileLayerGroup) {
        stats.drawablesRemoved += collisionTileLayerGroup->removeDrawablesIf([&](gfx::Drawable& drawable) {
            // If the render pass has changed or the tile has  dropped out of the cover set, remove it.
            const auto& tileID = drawable.getTileID();
            if (drawable.getRenderPass() != passes || (tileID && !hasRenderTile(*tileID))) {
                return true;
            }
            return false;
        });
    }

    const auto& layout = impl_cast(baseImpl).layout;
    const bool sortFeaturesByKey = !layout.get<SymbolSortKey>().isUndefined();
    std::multiset<SegmentGroup> renderableSegments;
    std::unique_ptr<gfx::DrawableBuilder> builder;
    const bool isOffset = !layout.get<IconOffset>().isUndefined();

    const auto currentZoom = static_cast<float>(state.getZoom());
    const auto layerPrefix = getID() + "/";
    const auto layerCollisionPrefix = getID() + "-collision/";

    std::unique_ptr<gfx::DrawableBuilder> collisionBuilder = context.createDrawableBuilder(layerCollisionPrefix);
    collisionBuilder->setSubLayerIndex(0);
    collisionBuilder->setEnableDepth(false);
    collisionBuilder->setRenderPass(passes);
    collisionBuilder->setCullFaceMode(gfx::CullFaceMode::disabled());
    collisionBuilder->setColorMode(gfx::ColorMode::alphaBlended());

    StringIDSetsPair propertiesAsUniforms;
    for (const RenderTile& tile : *renderTiles) {
        const auto& tileID = tile.getOverscaledTileID();

        const auto* optRenderData = getRenderDataForPass(tile, passes);
        if (!optRenderData || !optRenderData->bucket || !optRenderData->bucket->hasData() ||
            !optRenderData->bucket->check(SYM_GUARD_LOC)) {
            removeTile(passes, tileID);
            continue;
        }

        const auto& renderData = *optRenderData;
        const auto& bucket = static_cast<const SymbolBucket&>(*renderData.bucket);
        const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderData.layerProperties);

        const auto prevBucketID = getRenderTileBucketID(tileID);
        if (prevBucketID != util::SimpleIdentity::Empty && prevBucketID != bucket.getID()) {
            // This tile was previously set up from a different bucket, drop and re-create any drawables for it.
            removeTile(passes, tileID);
        }
        setRenderTileBucketID(tileID, bucket.getID());

        assert(bucket.paintProperties.contains(getID()));
        const auto& bucketPaintProperties = bucket.paintProperties.at(getID());

        if (updateParameters->captureRenderedFeatures) {
            const auto& params = renderTree.getParameters().transformParams;
            captureRenderedFeatures(tile, bucket, evaluated, state, params);
        }

        auto addCollisionDrawables = [&](const bool isText, const bool hasCollisionBox, const bool hasCollisionCircle) {
            if (!hasCollisionBox && !hasCollisionCircle) return;

            const auto& collisionGroup = getCollisionTileLayerGroup();
            if (!collisionGroup) {
                return;
            }

            const auto& bucketLayout = *bucket.layout;
            const auto values = isText ? textPropertyValues(evaluated, bucketLayout)
                                       : iconPropertyValues(evaluated, bucketLayout);
            const std::string suffix = isText ? "text/" : "icon/";

            const auto addVertices = [&collisionBuilder](const auto& vertices) {
                collisionBuilder->setRawVertices({}, vertices.size(), gfx::AttributeDataType::Short2);
            };

            if (hasCollisionBox) {
                const auto& collisionBox = isText ? bucket.textCollisionBox : bucket.iconCollisionBox;
                if (const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                        collisionBoxGroup->getOrCreateShader(context, {}))) {
                    collisionBuilder->setDrawableName(layerCollisionPrefix + suffix + "box");
                    collisionBuilder->setShader(shader);
                    addVertices(collisionBox->vertices().vector());
                    collisionBuilder->setVertexAttributes(getCollisionVertexAttributes(context, *collisionBox));
                    collisionBuilder->setSegments(gfx::Lines(1.0f),
                                                  collisionBox->sharedLines,
                                                  collisionBox->segments.data(),
                                                  collisionBox->segments.size());
                    collisionBuilder->flush(context);
                }
            }

            if (hasCollisionCircle) {
                const auto& collisionCircle = isText ? bucket.textCollisionCircle : bucket.iconCollisionCircle;
                if (const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                        collisionCircleGroup->getOrCreateShader(context, {}))) {
                    collisionBuilder->setDrawableName(layerCollisionPrefix + suffix + "circle");
                    collisionBuilder->setShader(shader);
                    addVertices(collisionCircle->vertices().vector());
                    collisionBuilder->setVertexAttributes(getCollisionVertexAttributes(context, *collisionCircle));
                    collisionBuilder->setSegments(gfx::Triangles(),
                                                  collisionCircle->sharedTriangles,
                                                  collisionCircle->segments.data(),
                                                  collisionCircle->segments.size());
                    collisionBuilder->flush(context);
                }
            }

            // add drawables to layer group
            for (auto& drawable : collisionBuilder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(collisionLayerTweaker);

                auto drawData = std::make_unique<gfx::CollisionDrawableData>(values.translate, values.translateAnchor);
                drawable->setData(std::move(drawData));
                collisionGroup->addDrawable(passes, tileID, std::move(drawable));
                ++stats.drawablesAdded;
            }
        };

        // If we already have drawables for this tile, update them.
        // Just update the drawables we already created
        auto updateExisting = [&](gfx::Drawable& drawable) {
            if (drawable.getLayerTweaker() != layerTweaker) {
                // This drawable was produced on a previous style/bucket, and should not be updated.
                return false;
            }

            propertiesAsUniforms.first.clear();
            propertiesAsUniforms.second.clear();

            updateTileDrawable(drawable, bucket, bucketPaintProperties, evaluated);
            return true;
        };
        if (updateTile(passes, tileID, std::move(updateExisting))) {
            // re-create collision drawables
            if (collisionTileLayerGroup) {
                collisionTileLayerGroup->removeDrawables(passes, tileID);
            }
            addCollisionDrawables(
                false /*isText*/, bucket.hasIconCollisionBoxData(), bucket.hasIconCollisionCircleData());
            addCollisionDrawables(
                true /*isText*/, bucket.hasTextCollisionBoxData(), bucket.hasTextCollisionCircleData());

            continue;
        }

        float serialKey = 1.0f;
        auto addRenderables = [&](const SymbolBucket::Buffer& buffer, const SymbolType type) mutable {
            if (sortFeaturesByKey) {
                // Features need to be rendered in a specific order, so we add each segment individually
                for (const auto& segment : buffer.segments) {
#if MLN_USE_SYMBOL_INSTANCING
                    assert(segment.baseInstance + segment.instanceCount <= buffer.attributeData().elements());
#else
                    assert(segment.vertexOffset + segment.vertexLength <= buffer.attributeData().elements());
#endif
                    renderableSegments.emplace(SegmentGroup{
                        .renderable = {segment, tile, renderData, bucketPaintProperties, segment.sortKey, type},
                        .segments = emptySegmentVector});
                }
            } else if (!buffer.segments.empty()) {
                // Features can be rendered in the order produced, and as grouped by the bucket
                const auto& firstSeg = buffer.segments.front();
                renderableSegments.emplace(
                    SegmentGroup{.renderable = {firstSeg, tile, renderData, bucketPaintProperties, serialKey, type},
                                 .segments = buffer.segments});
                serialKey += 1.0;
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
    using RawVertexVec = std::vector<std::uint8_t>; // raw buffer for vertexes of <int16_t, 4>
    struct TileInfo {
        RawVertexVec textVertices, iconVertices;
        gfx::DrawableTweakerPtr textTweaker, iconTweaker;
    };

    mln::unordered_map<UnwrappedTileID, TileInfo> tileCache;
    tileCache.reserve(renderTiles->size());

    for (auto& group : renderableSegments) {
        const auto& renderable = group.renderable;
        const auto& segments = group.segments.get();

        const auto isText = (renderable.type == SymbolType::Text);
        const auto sdfIcons = (renderable.type == SymbolType::IconSDF);

        const auto& tile = renderable.tile;
        const auto tileID = tile.id.overscaleTo(renderable.overscaledZ);
        auto& bucket = static_cast<SymbolBucket&>(*renderable.renderData.bucket);
        const auto& buffer = isText ? bucket.text : (sdfIcons ? bucket.sdfIcon : bucket.icon);

#if MLN_USE_SYMBOL_INSTANCING
        if (!buffer.attributeData().elements()) {
            continue;
        }
#else
        if (!buffer.sharedTriangles->elements()) {
            continue;
        }
#endif

        const auto& evaluated = getEvaluated<SymbolLayerProperties>(renderable.renderData.layerProperties);
        auto& bucketPaintProperties = bucket.paintProperties.at(getID());

        const auto& bucketLayout = *bucket.layout;
        const auto values = isText ? textPropertyValues(evaluated, bucketLayout)
                                   : iconPropertyValues(evaluated, bucketLayout);

        const auto& atlases = tile.getAtlasTextures();
        if (!atlases) {
            assert(false);
            continue;
        }

        auto& tileInfo = tileCache[tile.id];

        propertiesAsUniforms.first.clear();
        propertiesAsUniforms.second.clear();

#if MLN_USE_SYMBOL_INSTANCING
        auto vertexAttribs = context.createVertexAttributeArray();
        if (const auto& attr = vertexAttribs->set(idSymbolPosAttribute)) {
            attr->setSharedRawData(staticDataVertices,
                                   offsetof(SymbolStaticVertexAttributes, a1),
                                   /*vertexOffset=*/0,
                                   sizeof(SymbolStaticVertexAttributes),
                                   gfx::AttributeDataType::Short2);
        }

        auto instanceAttribs = context.createVertexAttributeArray();
        updateTileAttributes(buffer, isText, bucketPaintProperties, evaluated, *instanceAttribs, &propertiesAsUniforms);
#else
        auto vertexAttribs = context.createVertexAttributeArray();
        updateTileAttributes(buffer, isText, bucketPaintProperties, evaluated, *vertexAttribs, &propertiesAsUniforms);
#endif

        const auto textHalo = evaluated.get<style::TextHaloColor>().constantOr(Color::black()).a > 0.0f &&
                              evaluated.get<style::TextHaloWidth>().constantOr(1);
        const auto textFill = evaluated.get<style::TextColor>().constantOr(Color::black()).a > 0.0f;

        const auto iconHalo = evaluated.get<style::IconHaloColor>().constantOr(Color::black()).a > 0.0f &&
                              evaluated.get<style::IconHaloWidth>().constantOr(1);
        const auto iconFill = evaluated.get<style::IconColor>().constantOr(Color::black()).a > 0.0f;

        if (builder) {
            builder->clearTweakers();
        }

        const auto draw = [&](const gfx::ShaderGroupPtr& shaderGroup,
                              const bool isHalo,
                              const std::string_view suffix) {
            if (!shaderGroup) {
                return;
            }

            // We can use the same tweakers for all the segments in a tile
            if (isText && !tileInfo.textTweaker) {
                const bool textSizeIsZoomConstant = bucket.textSizeBinder->evaluateForZoom(currentZoom).isZoomConstant;
                tileInfo.textTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(atlases,
                                                                                     idSymbolImageIconTexture,
                                                                                     idSymbolImageTexture,
                                                                                     isText,
                                                                                     false,
                                                                                     values.rotationAlignment,
                                                                                     false,
                                                                                     textSizeIsZoomConstant);
            }
            if (!isText && !tileInfo.iconTweaker) {
                const bool iconScaled = bucketLayout.get<IconSize>().constantOr(1.0) != 1.0 || bucket.iconsNeedLinear;
                tileInfo.iconTweaker = std::make_shared<gfx::DrawableAtlasesTweaker>(atlases,
                                                                                     idSymbolImageIconTexture,
                                                                                     idSymbolImageTexture,
                                                                                     isText,
                                                                                     sdfIcons,
                                                                                     values.rotationAlignment,
                                                                                     iconScaled,
                                                                                     false);
            }

            if (!builder) {
                builder = context.createDrawableBuilder(layerPrefix);
                builder->setSubLayerIndex(0);
                builder->setRenderPass(passes);
                builder->setCullFaceMode(gfx::CullFaceMode::disabled());
                builder->setDepthType(gfx::DepthMaskType::ReadOnly);
                builder->setColorMode(
                    ((mln::underlying_type(passes) & mln::underlying_type(RenderPass::Translucent)) != 0)
                        ? gfx::ColorMode::alphaBlended()
                        : gfx::ColorMode::unblended());
            }

            const auto shader = std::static_pointer_cast<gfx::ShaderProgramBase>(
                shaderGroup->getOrCreateShader(context, propertiesAsUniforms, posOffsetAttribName));
            if (!shader) {
                return;
            }
            builder->setShader(shader);

            builder->clearTweakers();
            builder->addTweaker(isText ? tileInfo.textTweaker : tileInfo.iconTweaker);
            builder->setDrawableName(layerPrefix + std::string(suffix));
            builder->setVertexAttributes(vertexAttribs);

#if MLN_USE_SYMBOL_INSTANCING
            builder->setInstanceAttributes(instanceAttribs);
            builder->setRawVertices({}, staticDataVertices->elements(), gfx::AttributeDataType::Short2);

            auto& triangleIndices = staticDataIndices;
#else
            builder->setRawVertices({}, buffer.attributeData().elements(), gfx::AttributeDataType::Short4);

            auto& triangleIndices = buffer.sharedTriangles;
#endif

            if (segments.empty()) {
                builder->setSegments(gfx::Triangles(), triangleIndices, &renderable.segment.get(), 1);
            } else {
                builder->setSegments(gfx::Triangles(), triangleIndices, segments.data(), segments.size());
            }

            builder->flush(context);

            for (auto& drawable : builder->clearDrawables()) {
                drawable->setTileID(tileID);
                drawable->setLayerTweaker(layerTweaker);
                drawable->setRenderTile(renderTilesOwner, &tile);

                PaintPropertyBindersBase& textBinders = bucketPaintProperties.textBinders;
                PaintPropertyBindersBase& iconBinders = bucketPaintProperties.iconBinders;
                drawable->setBinders(renderable.renderData.bucket, isText ? &textBinders : &iconBinders);

                const bool hasVariablePlacement = bucket.hasVariablePlacement;
                drawable->setData(std::make_unique<gfx::SymbolDrawableData>(
                    /*.isHalo=*/isHalo,
                    /*.bucketVaraiblePlacement=*/hasVariablePlacement,
                    /*.symbolType=*/renderable.type,
                    /*.pitchAlignment=*/values.pitchAlignment,
                    /*.rotationAlignment=*/values.rotationAlignment,
                    /*.placement=*/bucketLayout.get<SymbolPlacement>(),
                    /*.textFit=*/bucketLayout.get<IconTextFit>(),
                    /*.isOffset=*/isOffset));

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
                    draw(symbolSDFGroup, /* isHalo = */ true, "halo");
                }

                if (textFill) {
                    draw(symbolSDFGroup, /* isHalo = */ false, "fill");
                }
            }
        } else { // icons
            if (sdfIcons) {
                if (iconHalo) {
                    draw(symbolSDFGroup, /* isHalo = */ true, "halo");
                }

                if (iconFill) {
                    draw(symbolSDFGroup, /* isHalo = */ false, "fill");
                }
            } else {
                draw(symbolIconGroup, /* isHalo = */ false, "icon");
            }
        }
    }
}

} // namespace mln
