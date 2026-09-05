#pragma once

#include <mln/gfx/index_buffer.hpp>
#include <mln/gfx/vertex_buffer.hpp>
#include <mln/layout/symbol_feature.hpp>
#include <mln/layout/symbol_instance.hpp>
#include <mln/map/mode.hpp>
#include <mln/shaders/segment.hpp>
#include <mln/renderer/bucket.hpp>
#include <mln/renderer/paint_property_binder.hpp>
#include <mln/style/layers/symbol_layer_properties.hpp>
#include <mln/text/glyph_range.hpp>

#include <memory>
#include <vector>

namespace mln {
class RenderTerrain;

namespace style {

// {icon,text}-specific paint-property packs for use in the symbol Programs.
// Since each program deals either with icons or text, using a smaller property set
// lets us avoid unnecessarily binding attributes for properties the program wouldn't use.
class IconPaintProperties : public Properties<IconOpacity,
                                              IconColor,
                                              IconHaloColor,
                                              IconHaloWidth,
                                              IconHaloBlur,
                                              IconTranslate,
                                              IconTranslateAnchor> {};

class TextPaintProperties : public Properties<TextOpacity,
                                              TextColor,
                                              TextHaloColor,
                                              TextHaloWidth,
                                              TextHaloBlur,
                                              TextTranslate,
                                              TextTranslateAnchor> {};

} // namespace style

class CrossTileSymbolLayerIndex;

using SymbolIconBinders = PaintPropertyBinders<style::IconPaintProperties::DataDrivenProperties>;
using SymbolTextBinders = PaintPropertyBinders<style::TextPaintProperties::DataDrivenProperties>;
using SymbolStaticVertexAttributes = gfx::Vertex<TypeList<attributes::pos>>;

#if MLN_USE_SYMBOL_INSTANCING
using SymbolSortedInstance = gfx::Vertex<TypeList<attributes::sorted_instance>>;
using SymbolLayoutAttributes = gfx::Vertex<TypeList<attributes::pos_scale,
                                                    attributes::offset_tltr,
                                                    attributes::offset_blbr,
                                                    attributes::texture_rect,
                                                    attributes::pixeloffset,
                                                    attributes::size_sdf>>;
#else
using SymbolLayoutAttributes =
    gfx::Vertex<TypeList<attributes::pos_offset, attributes::data<uint16_t, 4>, attributes::pixeloffset>>;
#endif

using SymbolDynamicLayoutAttributes = gfx::Vertex<TypeList<attributes::projected_pos>>;
using SymbolOpacityAttributes = gfx::Vertex<TypeList<attributes::fade_opacity>>;

using CollisionBoxLayoutVertexAttributes =
    gfx::Vertex<TypeList<attributes::pos, attributes::anchor_pos, attributes::extrude>>;
using CollisionBoxDynamicVertexAttributes = gfx::Vertex<TypeList<attributes::placed, attributes::shift>>;

const uint16_t MAX_GLYPH_ICON_SIZE = 255;
const uint16_t SIZE_PACK_FACTOR = 128;
const uint16_t MAX_PACKED_SIZE = MAX_GLYPH_ICON_SIZE * SIZE_PACK_FACTOR;

struct ZoomEvaluatedSize {
    bool isZoomConstant;
    bool isFeatureConstant;
    float sizeT;
    float size;
    float layoutSize;
};
// Mimic the PaintPropertyBinder technique specifically for the {text,icon}-size
// layout properties in order to provide a 'custom' scheme for encoding the
// necessary attribute data.  As with PaintPropertyBinder, SymbolSizeBinder is
// an abstract class whose implementations handle the particular attribute &
// uniform logic needed by each possible type of the {Text,Icon}Size properties.
class SymbolSizeBinder {
public:
    virtual ~SymbolSizeBinder() = default;

    static std::unique_ptr<SymbolSizeBinder> create(float tileZoom,
                                                    const style::PropertyValue<float>& sizeProperty,
                                                    float defaultValue);

    virtual Range<float> getVertexSizeData(const GeometryTileFeature& feature) = 0;
    virtual ZoomEvaluatedSize evaluateForZoom(float currentZoom) const = 0;
};

class ConstantSymbolSizeBinder final : public SymbolSizeBinder {
public:
    ConstantSymbolSizeBinder(const float /*tileZoom*/, const float& size, const float /*defaultValue*/) noexcept
        : layoutSize(size) {}

    ConstantSymbolSizeBinder(const float /*tileZoom*/, const style::Undefined&, const float defaultValue) noexcept
        : layoutSize(defaultValue) {}

    ConstantSymbolSizeBinder(const float tileZoom,
                             const style::PropertyExpression<float>& expression_,
                             const float /*defaultValue*/
                             )
        : layoutSize(expression_.evaluate(tileZoom + 1)),
          expression(expression_) {
        const Range<float> zoomLevels = expression_.getCoveringStops(tileZoom, tileZoom + 1);
        coveringRanges = std::make_tuple(
            zoomLevels, Range<float>{expression_.evaluate(zoomLevels.min), expression_.evaluate(zoomLevels.max)});
    }

    Range<float> getVertexSizeData(const GeometryTileFeature&) noexcept override { return {0.0f, 0.0f}; };

    ZoomEvaluatedSize evaluateForZoom(float currentZoom) const override {
        float size = layoutSize;
        bool isZoomConstant = !(coveringRanges || expression);
        if (coveringRanges) {
            // Even though we could get the exact value of the camera function
            // at z = currentZoom, we intentionally do not: instead, we
            // interpolate between the camera function values at a pair of zoom
            // stops covering [tileZoom, tileZoom + 1] in order to be consistent
            // with this restriction on composite functions.
            const Range<float>& zoomLevels = std::get<0>(*coveringRanges);
            const Range<float>& sizeLevels = std::get<1>(*coveringRanges);
            float t = util::clamp(expression->interpolationFactor(zoomLevels, currentZoom), 0.0f, 1.0f);
            size = sizeLevels.min + t * (sizeLevels.max - sizeLevels.min);
        } else if (expression) {
            size = expression->evaluate(currentZoom);
        }

        const float unused = 0.0f;
        return {.isZoomConstant = isZoomConstant,
                .isFeatureConstant = true,
                .sizeT = unused,
                .size = size,
                .layoutSize = layoutSize};
    }

    float layoutSize;
    std::optional<std::tuple<Range<float>, Range<float>>> coveringRanges;
    std::optional<style::PropertyExpression<float>> expression;
};

class SourceFunctionSymbolSizeBinder final : public SymbolSizeBinder {
public:
    SourceFunctionSymbolSizeBinder(const float /*tileZoom*/,
                                   style::PropertyExpression<float> expression_,
                                   const float defaultValue_) noexcept
        : expression(std::move(expression_)),
          defaultValue(defaultValue_) {}

    Range<float> getVertexSizeData(const GeometryTileFeature& feature) override {
        const float size = expression.evaluate(feature, defaultValue);
        return {size, size};
    };

    ZoomEvaluatedSize evaluateForZoom(float) const noexcept override {
        const float unused = 0.0f;
        return {
            .isZoomConstant = true, .isFeatureConstant = false, .sizeT = unused, .size = unused, .layoutSize = unused};
    }

    style::PropertyExpression<float> expression;
    const float defaultValue;
};

class CompositeFunctionSymbolSizeBinder final : public SymbolSizeBinder {
public:
    CompositeFunctionSymbolSizeBinder(const float tileZoom,
                                      style::PropertyExpression<float> expression_,
                                      const float defaultValue_) noexcept
        : expression(std::move(expression_)),
          defaultValue(defaultValue_),
          layoutZoom(tileZoom + 1),
          coveringZoomStops(expression.getCoveringStops(tileZoom, tileZoom + 1)) {}

    Range<float> getVertexSizeData(const GeometryTileFeature& feature) override {
        return {expression.evaluate(coveringZoomStops.min, feature, defaultValue),
                expression.evaluate(coveringZoomStops.max, feature, defaultValue)};
    };

    ZoomEvaluatedSize evaluateForZoom(float currentZoom) const override {
        float sizeInterpolationT = util::clamp(
            expression.interpolationFactor(coveringZoomStops, currentZoom), 0.0f, 1.0f);

        const float unused = 0.0f;
        return {.isZoomConstant = false,
                .isFeatureConstant = false,
                .sizeT = sizeInterpolationT,
                .size = unused,
                .layoutSize = unused};
    }

    style::PropertyExpression<float> expression;
    const float defaultValue;
    float layoutZoom;
    Range<float> coveringZoomStops;
};

class PlacedSymbol {
public:
    PlacedSymbol(Point<float> anchorPoint_,
                 std::size_t segment_,
                 float lowerSize_,
                 float upperSize_,
                 std::array<float, 2> lineOffset_,
                 WritingModeType writingModes_,
                 GeometryCoordinates line_,
                 std::vector<float> tileDistances_,
                 std::optional<size_t> placedIconIndex_ = std::nullopt)
        : anchorPoint(anchorPoint_),
          segment(segment_),
          lowerSize(lowerSize_),
          upperSize(upperSize_),
          lineOffset(lineOffset_),
          writingModes(writingModes_),
          line(std::move(line_)),
          tileDistances(std::move(tileDistances_)),
          hidden(false),
          startIndex(0),
          placedIconIndex(std::move(placedIconIndex_)) {}
    Point<float> anchorPoint;
    std::size_t segment;
    float lowerSize;
    float upperSize;
    std::array<float, 2> lineOffset;
    WritingModeType writingModes;
    GeometryCoordinates line;
    std::vector<float> tileDistances;
    std::vector<float> glyphOffsets;
    bool hidden;
    size_t startIndex;
    // The crossTileID is only filled/used on the foreground for variable text anchors
    uint32_t crossTileID = 0u;
    // The placedOrientation is only used when symbol layer's property is set to
    // support placement for orientation variants.
    std::optional<style::TextWritingModeType> placedOrientation;
    float angle = 0;

    // Reference to placed icon, only applicable for text symbols.
    std::optional<size_t> placedIconIndex;
};

class SymbolBucket final : public Bucket {
public:
    SymbolBucket(Immutable<style::SymbolLayoutProperties::PossiblyEvaluated>,
                 const std::map<std::string, Immutable<style::LayerProperties>>&,
                 const style::PropertyValue<float>& textSize,
                 const style::PropertyValue<float>& iconSize,
                 float zoom,
                 bool iconsNeedLinear,
                 bool sortFeaturesByY,
                 std::string bucketName_,
                 const std::vector<SymbolInstance>&&,
                 const std::vector<SortKeyRange>&&,
                 float tilePixelRatio,
                 bool allowVerticalPlacement,
                 std::vector<style::TextWritingModeType> placementModes,
                 bool iconsInText);
    ~SymbolBucket() override;

    static style::IconPaintProperties::PossiblyEvaluated iconPaintProperties(
        const style::SymbolPaintProperties::PossiblyEvaluated&);
    static style::TextPaintProperties::PossiblyEvaluated textPaintProperties(
        const style::SymbolPaintProperties::PossiblyEvaluated&);

    void upload(gfx::UploadPass&) override;
    bool hasData() const override;
    void update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) override;
    std::pair<uint32_t, bool> registerAtCrossTileIndex(CrossTileSymbolLayerIndex&, const RenderTile&) override;
    void place(Placement&, const BucketPlacementData&, std::set<uint32_t>&) override;
    void updateVertices(const Placement&,
                        bool updateOpacities,
                        const TransformState&,
                        const RenderTile&,
                        std::set<uint32_t>&,
                        const RenderTerrain* terrain = nullptr) override;
    bool hasTextData() const;
    bool hasIconData() const;
    bool hasSdfIconData() const;
    bool hasIconCollisionBoxData() const;
    bool hasIconCollisionCircleData() const;
    bool hasTextCollisionBoxData() const;
    bool hasTextCollisionCircleData() const;
    bool hasFormatSectionOverrides() const;
    bool hasVariableTextAnchors() const;

    void sortFeatures(float angle);
    // Returns references to the `symbolInstances` items, sorted by viewport Y.
    SymbolInstanceReferences getSortedSymbols(float angle) const;
    // Returns references to the `symbolInstances` items, which belong to the
    // `sortKeyRange` range; returns references to all the symbols if
    // |sortKeyRange| is `std::nullopt`.
    SymbolInstanceReferences getSymbols(const std::optional<SortKeyRange>& sortKeyRange = std::nullopt) const;

#if MLN_SYMBOL_GUARDS
    bool check(source_location) override;
#endif

#if MLN_USE_SYMBOL_INSTANCING
    static SymbolLayoutAttributes layoutAttributes(const SymbolQuad& symbol,
                                                   const Anchor& labelAnchor,
                                                   const Range<float>& sizeData) {
        const uint16_t aSizeMin = (std::min(MAX_PACKED_SIZE, static_cast<uint16_t>(sizeData.min * SIZE_PACK_FACTOR))
                                   << 1) +
                                  uint16_t(symbol.isSDF);
        const uint16_t aSizeMax = std::min(MAX_PACKED_SIZE, static_cast<uint16_t>(sizeData.max * SIZE_PACK_FACTOR));

        return {{static_cast<int16_t>(labelAnchor.point.x),
                 static_cast<int16_t>(labelAnchor.point.y),
                 static_cast<int16_t>(symbol.minFontScale.x * 256),
                 static_cast<int16_t>(symbol.minFontScale.y * 256)},

                {static_cast<int16_t>(std::round(symbol.tl.x * 32)),
                 static_cast<int16_t>(std::round((symbol.tl.y + symbol.glyphOffset.y) * 32)),
                 static_cast<int16_t>(std::round(symbol.tr.x * 32)),
                 static_cast<int16_t>(std::round((symbol.tr.y + symbol.glyphOffset.y) * 32))},

                {static_cast<int16_t>(std::round(symbol.bl.x * 32)),
                 static_cast<int16_t>(std::round((symbol.bl.y + symbol.glyphOffset.y) * 32)),
                 static_cast<int16_t>(std::round(symbol.br.x * 32)),
                 static_cast<int16_t>(std::round((symbol.br.y + symbol.glyphOffset.y) * 32))},

                {symbol.tex.x, symbol.tex.y, symbol.tex.w, symbol.tex.h},

                {static_cast<int16_t>(symbol.pixelOffsetTL.x * 16),
                 static_cast<int16_t>(symbol.pixelOffsetTL.y * 16),
                 static_cast<int16_t>(symbol.pixelOffsetBR.x * 16),
                 static_cast<int16_t>(symbol.pixelOffsetBR.y * 16)},

                { aSizeMin,
                  aSizeMax }};
    }
#else
    static SymbolLayoutAttributes layoutAttributes(Point<float> labelAnchor,
                                                   Point<float> o,
                                                   float glyphOffsetY,
                                                   uint16_t tx,
                                                   uint16_t ty,
                                                   const Range<float>& sizeData,
                                                   bool isSDF,
                                                   Point<float> pixelOffset,
                                                   Point<float> minFontScale) {
        const uint16_t aSizeMin = (std::min(MAX_PACKED_SIZE, static_cast<uint16_t>(sizeData.min * SIZE_PACK_FACTOR))
                                   << 1) +
                                  uint16_t(isSDF);
        const uint16_t aSizeMax = std::min(MAX_PACKED_SIZE, static_cast<uint16_t>(sizeData.max * SIZE_PACK_FACTOR));
        return {
            // combining pos and offset to reduce number of vertex attributes
            // passed to shader (8 max for some devices)
            {{static_cast<int16_t>(labelAnchor.x),
              static_cast<int16_t>(labelAnchor.y),
              static_cast<int16_t>(std::round(o.x * 32)), // use 1/32 pixels for placement
              static_cast<int16_t>(std::round((o.y + glyphOffsetY) * 32))}},
            {{tx, ty, aSizeMin, aSizeMax}},
            {{static_cast<int16_t>(pixelOffset.x * 16),
              static_cast<int16_t>(pixelOffset.y * 16),
              static_cast<int16_t>(minFontScale.x * 256),
              static_cast<int16_t>(minFontScale.y * 256)}},
        };
    }
#endif

    static SymbolDynamicLayoutAttributes dynamicLayoutAttributes(Point<float> anchorPoint, float labelAngle) {
        return {{{anchorPoint.x, anchorPoint.y, labelAngle}}};
    }

    static SymbolOpacityAttributes opacityAttributes(bool placed, float opacity) {
        return {{{static_cast<float>((static_cast<uint8_t>(opacity * 127) << 1) | static_cast<uint8_t>(placed))}}};
    }

    Immutable<style::SymbolLayoutProperties::PossiblyEvaluated> layout;
    const std::string bucketLeaderID;
    float sortedAngle = std::numeric_limits<float>::max();

    // Flags
    const bool iconsNeedLinear : 1;
    const bool sortFeaturesByY : 1;
    bool staticUploaded : 1;
    bool placementChangesUploaded : 1;
    bool dynamicUploaded : 1;
    bool sortUploaded : 1;
    bool iconsInText : 1;
    // Set and used by placement.
    mutable bool justReloaded : 1;
    bool hasVariablePlacement : 1;
    bool hasUninitializedSymbols : 1;

    std::vector<SymbolInstance> symbolInstances;
    const std::vector<SortKeyRange> sortKeyRanges;

    struct PaintProperties {
        SymbolIconBinders iconBinders;
        SymbolTextBinders textBinders;
    };
    std::map<std::string, PaintProperties> paintProperties;

    std::unique_ptr<SymbolSizeBinder> textSizeBinder;

    using AttributeVector = gfx::VertexVector<SymbolLayoutAttributes>;
    using DynamicAttributeVector = gfx::VertexVector<SymbolDynamicLayoutAttributes>;
    using OpacityAttributeVector = gfx::VertexVector<SymbolOpacityAttributes>;

#if MLN_USE_SYMBOL_INSTANCING
    using SortedInstanceVector = gfx::VertexVector<SymbolSortedInstance>;
#else
    using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
#endif

    struct Buffer final {
        ~Buffer() {
            sharedAttributeData->release();
            sharedDynamicAttributeData->release();
            sharedOpacityAttributeData->release();
#if MLN_USE_SYMBOL_INSTANCING
            sharedSortedInstances->release();
#endif
        }

        void updateModified() {
            if (sharedAttributeData) {
                sharedAttributeData->updateModified();
            }
            if (sharedDynamicAttributeData) {
                sharedDynamicAttributeData->updateModified();
            }
            if (sharedOpacityAttributeData) {
                sharedOpacityAttributeData->updateModified();
            }
#if MLN_USE_SYMBOL_INSTANCING
            if (sharedSortedInstances) {
                sharedSortedInstances->updateModified();
            }
#endif
        }

        std::shared_ptr<AttributeVector> sharedAttributeData = std::make_shared<AttributeVector>();
        AttributeVector& attributeData() { return *sharedAttributeData; }
        const AttributeVector& attributeData() const { return *sharedAttributeData; }

        std::shared_ptr<DynamicAttributeVector> sharedDynamicAttributeData = std::make_shared<DynamicAttributeVector>();
        DynamicAttributeVector& dynamicAttributeData() { return *sharedDynamicAttributeData; }
        const DynamicAttributeVector& dynamicAttributeData() const { return *sharedDynamicAttributeData; }

        std::shared_ptr<OpacityAttributeVector> sharedOpacityAttributeData = std::make_shared<OpacityAttributeVector>();
        OpacityAttributeVector& opacityAttributeData() { return *sharedOpacityAttributeData; }
        const OpacityAttributeVector& opacityAttributeData() const { return *sharedOpacityAttributeData; }

#if MLN_USE_SYMBOL_INSTANCING
        std::shared_ptr<SortedInstanceVector> sharedSortedInstances = std::make_shared<SortedInstanceVector>();
        SortedInstanceVector& sortedInstances() { return *sharedSortedInstances; }
        const SortedInstanceVector& sortedInstances() const { return *sharedSortedInstances; }
#else
        const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
        TriangleIndexVector& triangles = *sharedTriangles;
#endif

        SegmentVector segments;
        std::vector<PlacedSymbol> placedSymbols;
    } text;

    std::unique_ptr<SymbolSizeBinder> iconSizeBinder;

    Buffer icon;
    Buffer sdfIcon;

    using CollisionVertexVector = gfx::VertexVector<CollisionBoxLayoutVertexAttributes>;
    using CollisionDynamicVertexVector = gfx::VertexVector<CollisionBoxDynamicVertexAttributes>;

    struct CollisionBuffer {
        std::shared_ptr<CollisionVertexVector> sharedVertices = std::make_shared<CollisionVertexVector>();
        CollisionVertexVector& vertices() { return *sharedVertices; }
        const CollisionVertexVector& vertices() const { return *sharedVertices; }

        std::shared_ptr<CollisionDynamicVertexVector> sharedDynamicVertices =
            std::make_shared<CollisionDynamicVertexVector>();
        CollisionDynamicVertexVector& dynamicVertices() { return *sharedDynamicVertices; }
        const CollisionDynamicVertexVector& dynamicVertices() const { return *sharedDynamicVertices; }

        SegmentVector segments;
    };

    struct CollisionBoxBuffer : public CollisionBuffer {
        using LineIndexVector = gfx::IndexVector<gfx::Lines>;
        const std::shared_ptr<LineIndexVector> sharedLines = std::make_shared<LineIndexVector>();
        LineIndexVector& lines = *sharedLines;
    };
    std::unique_ptr<CollisionBoxBuffer> iconCollisionBox;
    std::unique_ptr<CollisionBoxBuffer> textCollisionBox;

    CollisionBoxBuffer& getOrCreateIconCollisionBox() {
        if (!iconCollisionBox) iconCollisionBox = std::make_unique<CollisionBoxBuffer>();
        return *iconCollisionBox;
    }

    CollisionBoxBuffer& getOrCreateTextCollisionBox() {
        if (!textCollisionBox) textCollisionBox = std::make_unique<CollisionBoxBuffer>();
        return *textCollisionBox;
    }

    struct CollisionCircleBuffer : public CollisionBuffer {
        using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
        const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
        TriangleIndexVector& triangles = *sharedTriangles;
    };
    std::unique_ptr<CollisionCircleBuffer> iconCollisionCircle;
    std::unique_ptr<CollisionCircleBuffer> textCollisionCircle;

    CollisionCircleBuffer& getOrCreateIconCollisionCircleBuffer() {
        if (!iconCollisionCircle) iconCollisionCircle = std::make_unique<CollisionCircleBuffer>();
        return *iconCollisionCircle;
    }

    CollisionCircleBuffer& getOrCreateTextCollisionCircleBuffer() {
        if (!textCollisionCircle) textCollisionCircle = std::make_unique<CollisionCircleBuffer>();
        return *textCollisionCircle;
    }

    static CollisionBoxLayoutVertexAttributes collisionLayoutVertex(Point<float> a,
                                                                    Point<float> anchor,
                                                                    Point<float> o) {
        return {{{static_cast<int16_t>(a.x), static_cast<int16_t>(a.y)}},
                {{static_cast<int16_t>(anchor.x), static_cast<int16_t>(anchor.y)}},
                {{static_cast<int16_t>(::round(o.x)), static_cast<int16_t>(::round(o.y))}}};
    }

    static CollisionBoxDynamicVertexAttributes collisionDynamicVertex(bool placed, bool notUsed, Point<float> shift) {
        return {{{static_cast<uint16_t>(placed), static_cast<uint16_t>(notUsed)}}, {{shift.x, shift.y}}};
    }

    const float tilePixelRatio;
    uint32_t bucketInstanceId;
    const bool allowVerticalPlacement;
    const std::vector<style::TextWritingModeType> placementModes;
    mutable std::optional<bool> hasFormatSectionOverrides_;

    FeatureSortOrder featureSortOrder;
};

} // namespace mln
