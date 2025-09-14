#include <mbgl/renderer/layers/render_symbol_layer.hpp>

#include <mbgl/gfx/cull_face_mode.hpp>
#include <mbgl/gfx/shader_registry.hpp>
#include <mbgl/layout/symbol_layout.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/renderer/buckets/symbol_bucket.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/renderer/property_evaluation_parameters.hpp>
#include <mbgl/renderer/render_source.hpp>
#include <mbgl/renderer/render_tile.hpp>
#include <mbgl/renderer/tile_render_data.hpp>
#include <mbgl/renderer/upload_parameters.hpp>
#include <mbgl/style/layers/symbol_layer_impl.hpp>
#include <mbgl/text/shaping.hpp>
#include <mbgl/tile/geometry_tile.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile.hpp>
#include <mbgl/util/convert.hpp>
#include <mbgl/util/math.hpp>

#include <mbgl/gfx/drawable_atlases_tweaker.hpp>
#include <mbgl/gfx/drawable_builder.hpp>
#include <mbgl/gfx/symbol_drawable_data.hpp>
#include <mbgl/gfx/collision_drawable_data.hpp>
#include <mbgl/renderer/layer_group.hpp>
#include <mbgl/renderer/layers/symbol_layer_tweaker.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/shaders/shader_program_base.hpp>
#include <mbgl/shaders/symbol_layer_ubo.hpp>
#include <mbgl/renderer/layers/collision_layer_tweaker.hpp>

#include <set>

namespace mbgl {

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
                placementData.push_back({*bucket, renderTile, featureIndex, baseImpl->source, std::nullopt});
            } else {
                for (const auto& sortKeyRange : bucket->sortKeyRanges) {
                    BucketPlacementData layerData{.bucket = *bucket,
                                                  .tile = renderTile,
                                                  .featureIndex = featureIndex,
                                                  .sourceId = baseImpl->source,
                                                  .sortKeyRange = sortKeyRange};
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

namespace {
const SegmentVector emptySegmentVector;
constexpr auto posOffsetAttribName = "a_pos_offset";

void updateTileAttributes(const SymbolBucket::Buffer& buffer,
                          const bool isText,
                          const SymbolBucket::PaintProperties& paintProps,
                          const SymbolPaintProperties::PossiblyEvaluated& evaluated,
                          gfx::VertexAttributeArray& attribs,
                          StringIDSetsPair* propertiesAsUniforms) {
    if (const auto& attr = attribs.set(idSymbolPosOffsetVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(SymbolLayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutVertex),
                               gfx::AttributeDataType::Short4);
    }
    if (const auto& attr = attribs.set(idSymbolDataVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(SymbolLayoutVertex, a2),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutVertex),
                               gfx::AttributeDataType::UShort4);
    }
    if (const auto& attr = attribs.set(idSymbolPixelOffsetVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(SymbolLayoutVertex, a3),
                               /*vertexOffset=*/0,
                               sizeof(SymbolLayoutVertex),
                               gfx::AttributeDataType::Short4);
    }

    if (const auto& attr = attribs.set(idSymbolProjectedPosVertexAttribute)) {
        using Vertex = gfx::Vertex<SymbolDynamicLayoutAttributes>;
        attr->setSharedRawData(buffer.sharedDynamicVertices,
                               offsetof(Vertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(Vertex),
                               gfx::AttributeDataType::Float3);
    }
    if (const auto& attr = attribs.set(idSymbolFadeOpacityVertexAttribute)) {
        using Vertex = gfx::Vertex<SymbolOpacityAttributes>;
        attr->setSharedRawData(buffer.sharedOpacityVertices,
                               offsetof(Vertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(Vertex),
                               gfx::AttributeDataType::Float);
    }

    if (isText) {
        attribs.readDataDrivenPaintProperties<TextOpacity, TextColor, TextHaloColor, TextHaloWidth, TextHaloBlur>(
            paintProps.textBinders, evaluated, propertiesAsUniforms, idSymbolOpacityVertexAttribute);
    } else {
        attribs.readDataDrivenPaintProperties<IconOpacity, IconColor, IconHaloColor, IconHaloWidth, IconHaloBlur>(
            paintProps.iconBinders, evaluated, propertiesAsUniforms, idSymbolOpacityVertexAttribute);
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
    const auto vertexCount = buffer.vertices().elements();

    drawable.setVertices({}, vertexCount, gfx::AttributeDataType::Short4);

    // TODO: detect whether anything has actually changed
    // See `Placement::updateBucketDynamicVertices`

    if (auto& attribs = drawable.getVertexAttributes()) {
        updateTileAttributes(buffer, isText, paintProps, evaluated, *attribs, nullptr);
    }
}

gfx::VertexAttributeArrayPtr getCollisionVertexAttributes(gfx::Context& context,
                                                          const SymbolBucket::CollisionBuffer& buffer) {
    auto vertexAttrs = context.createVertexAttributeArray();
    using LayoutVertex = gfx::Vertex<CollisionBoxLayoutAttributes>;

    if (const auto& attr = vertexAttrs->set(idCollisionPosVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(LayoutVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(LayoutVertex),
                               gfx::AttributeDataType::Short2);
    }
    if (const auto& attr = vertexAttrs->set(idCollisionAnchorPosVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(LayoutVertex, a2),
                               /*vertexOffset=*/0,
                               sizeof(LayoutVertex),
                               gfx::AttributeDataType::Short2);
    }
    if (const auto& attr = vertexAttrs->set(idCollisionExtrudeVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedVertices,
                               offsetof(LayoutVertex, a3),
                               /*vertexOffset=*/0,
                               sizeof(LayoutVertex),
                               gfx::AttributeDataType::Short2);
    }

    using DynamicVertex = gfx::Vertex<CollisionBoxDynamicAttributes>;

    if (const auto& attr = vertexAttrs->set(idCollisionPlacedVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedDynamicVertices,
                               offsetof(DynamicVertex, a1),
                               /*vertexOffset=*/0,
                               sizeof(DynamicVertex),
                               gfx::AttributeDataType::UShort2);
    }
    if (const auto& attr = vertexAttrs->set(idCollisionShiftVertexAttribute)) {
        attr->setSharedRawData(buffer.sharedDynamicVertices,
                               offsetof(DynamicVertex, a2),
                               /*vertexOffset=*/0,
                               sizeof(DynamicVertex),
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

void RenderSymbolLayer::update(gfx::ShaderRegistry& shaders,
                               gfx::Context& context,
                               const TransformState& state,
                               const std::shared_ptr<UpdateParameters>&,
                               const RenderTree&,
                               UniqueChangeRequestVec& changes) {
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

        assert(bucket.paintProperties.find(getID()) != bucket.paintProperties.end());
        const auto& bucketPaintProperties = bucket.paintProperties.at(getID());

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
                    assert(segment.vertexOffset + segment.vertexLength <= buffer.vertices().elements());
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

    mbgl::unordered_map<UnwrappedTileID, TileInfo> tileCache;
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

        if (!buffer.sharedTriangles->elements()) {
            continue;
        }

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

        const auto vertexCount = buffer.vertices().elements();

        propertiesAsUniforms.first.clear();
        propertiesAsUniforms.second.clear();

        auto attribs = context.createVertexAttributeArray();
        updateTileAttributes(buffer, isText, bucketPaintProperties, evaluated, *attribs, &propertiesAsUniforms);

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
                    ((mbgl::underlying_type(passes) & mbgl::underlying_type(RenderPass::Translucent)) != 0)
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
            builder->setRawVertices({}, vertexCount, gfx::AttributeDataType::Short4);
            builder->setDrawableName(layerPrefix + std::string(suffix));
            builder->setVertexAttributes(attribs);

            if (segments.empty()) {
                builder->setSegments(gfx::Triangles(), buffer.sharedTriangles, &renderable.segment.get(), 1);
            } else {
                builder->setSegments(gfx::Triangles(), buffer.sharedTriangles, segments.data(), segments.size());
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

} // namespace mbgl
