#pragma once

#include <mbgl/gfx/index_buffer.hpp>
#include <mbgl/gfx/vertex_buffer.hpp>
#include <mbgl/layout/symbol_feature.hpp>
#include <mbgl/layout/symbol_instance.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/programs/symbol_program.hpp>
#include <mbgl/renderer/bucket.hpp>
#include <mbgl/style/layers/symbol_layer_properties.hpp>
#include <mbgl/text/glyph_range.hpp>
#include <mbgl/text/placement.hpp>

#include <memory>
#include <vector>

namespace mbgl {

class CrossTileSymbolLayerIndex;

using CollisionBoxLayoutAttributes = TypeList<attributes::pos, attributes::anchor_pos, attributes::extrude>;
using CollisionBoxDynamicAttributes = TypeList<attributes::placed, attributes::shift>;

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
          vertexStartIndex(0),
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
    size_t vertexStartIndex;
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

    void upload(gfx::UploadPass&) override;
    bool hasData() const override;
    std::pair<uint32_t, bool> registerAtCrossTileIndex(CrossTileSymbolLayerIndex&, const RenderTile&) override;
    void place(Placement&, const BucketPlacementData&, std::set<uint32_t>&) override;
    void updateVertices(
        const Placement&, bool updateOpacities, const TransformState&, const RenderTile&, std::set<uint32_t>&) override;
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
    bool check(std::source_location) override;
#endif

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
        SymbolIconProgram::Binders iconBinders;
        SymbolSDFTextProgram::Binders textBinders;
    };
    std::map<std::string, PaintProperties> paintProperties;

    std::unique_ptr<SymbolSizeBinder> textSizeBinder;

    using VertexVector = gfx::VertexVector<SymbolLayoutVertex>;
    using VertexBuffer = gfx::VertexBuffer<SymbolLayoutVertex>;
    using DynamicVertexVector = gfx::VertexVector<gfx::Vertex<SymbolDynamicLayoutAttributes>>;
    using DynamicVertexBuffer = gfx::VertexBuffer<gfx::Vertex<SymbolDynamicLayoutAttributes>>;
    using OpacityVertexVector = gfx::VertexVector<gfx::Vertex<SymbolOpacityAttributes>>;
    using OpacityVertexBuffer = gfx::VertexBuffer<gfx::Vertex<SymbolOpacityAttributes>>;

    struct Buffer final {
        ~Buffer() {
            sharedVertices->release();
            sharedDynamicVertices->release();
            sharedOpacityVertices->release();
        }

        void updateModified() {
            if (sharedVertices) {
                sharedVertices->updateModified();
            }
            if (sharedDynamicVertices) {
                sharedDynamicVertices->updateModified();
            }
            if (sharedOpacityVertices) {
                sharedOpacityVertices->updateModified();
            }
        }

        std::shared_ptr<VertexVector> sharedVertices = std::make_shared<VertexVector>();
        VertexVector& vertices() { return *sharedVertices; }
        const VertexVector& vertices() const { return *sharedVertices; }

        std::shared_ptr<DynamicVertexVector> sharedDynamicVertices = std::make_shared<DynamicVertexVector>();
        DynamicVertexVector& dynamicVertices() { return *sharedDynamicVertices; }
        const DynamicVertexVector& dynamicVertices() const { return *sharedDynamicVertices; }

        std::shared_ptr<OpacityVertexVector> sharedOpacityVertices = std::make_shared<OpacityVertexVector>();
        OpacityVertexVector& opacityVertices() { return *sharedOpacityVertices; }
        const OpacityVertexVector& opacityVertices() const { return *sharedOpacityVertices; }

        using TriangleIndexVector = gfx::IndexVector<gfx::Triangles>;
        const std::shared_ptr<TriangleIndexVector> sharedTriangles = std::make_shared<TriangleIndexVector>();
        TriangleIndexVector& triangles = *sharedTriangles;

        SegmentVector segments;
        std::vector<PlacedSymbol> placedSymbols;
    } text;

    std::unique_ptr<SymbolSizeBinder> iconSizeBinder;

    Buffer icon;
    Buffer sdfIcon;

    using CollisionVertexVector = gfx::VertexVector<gfx::Vertex<CollisionBoxLayoutAttributes>>;
    using CollisionDynamicVertexVector = gfx::VertexVector<gfx::Vertex<CollisionBoxDynamicAttributes>>;

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
    
    static gfx::Vertex<CollisionBoxLayoutAttributes> collisionLayoutVertex(Point<float> a, Point<float> anchor, Point<float> o) {
        return {{{static_cast<int16_t>(a.x), static_cast<int16_t>(a.y)}},
            {{static_cast<int16_t>(anchor.x), static_cast<int16_t>(anchor.y)}},
            {{static_cast<int16_t>(::round(o.x)), static_cast<int16_t>(::round(o.y))}}};
    }

    static gfx::Vertex<CollisionBoxDynamicAttributes> collisionDynamicVertex(bool placed, bool notUsed, Point<float> shift) {
        return {{{static_cast<uint16_t>(placed), static_cast<uint16_t>(notUsed)}}, {{shift.x, shift.y}}};
    }

    const float tilePixelRatio;
    uint32_t bucketInstanceId;
    const bool allowVerticalPlacement;
    const std::vector<style::TextWritingModeType> placementModes;
    mutable std::optional<bool> hasFormatSectionOverrides_;

    FeatureSortOrder featureSortOrder;
};

} // namespace mbgl
