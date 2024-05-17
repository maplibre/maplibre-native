#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>
#include <mbgl/tile/tile_id.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/grid_index.hpp>
#include <mbgl/util/mat4.hpp>

#include <vector>
#include <string>
#include <unordered_map>

namespace mbgl {

class RenderedQueryOptions;
class RenderLayer;
class TransformState;
class SourceFeatureState;

class CollisionIndex;

/// An indexed element within a feature index.
/// This version holds a reference to the strings within the index itself to avoid making tens of thousands of
/// unnecessary copies of them. As a result, its lifetime cannot exceed that of the index.
class RefIndexedSubfeature {
public:
    RefIndexedSubfeature() = delete;
    RefIndexedSubfeature(RefIndexedSubfeature&&) = default;
    RefIndexedSubfeature(const RefIndexedSubfeature&) = default;
    RefIndexedSubfeature(std::size_t index_,
                         const std::string& sourceLayerName_,
                         const std::string& bucketName_,
                         size_t sortIndex_,
                         uint32_t bucketInstanceId_ = 0,
                         uint16_t collisionGroupId_ = 0);

    RefIndexedSubfeature& operator=(const RefIndexedSubfeature&) = default;
    RefIndexedSubfeature& operator=(RefIndexedSubfeature&&) = default;

    size_t getIndex() const { return index; }
    size_t getSortIndex() const { return sortIndex; }

    const std::string& getSourceLayerName() const { return sourceLayerName.get(); }
    const std::string& getBucketLeaderID() const { return bucketLeaderID.get(); }

    uint32_t getBucketInstanceId() const { return bucketInstanceId; }
    uint16_t getCollisionGroupId() const { return collisionGroupId; }

protected:
    size_t index;
    size_t sortIndex;

    std::reference_wrapper<const std::string> sourceLayerName;
    std::reference_wrapper<const std::string> bucketLeaderID;

    // Only used for symbol features
    uint32_t bucketInstanceId;
    uint16_t collisionGroupId;
};

/// This version of an indexed feature has copies of the necessary strings, so that it can be used outside the index.
class IndexedSubfeature : public RefIndexedSubfeature {
public:
    IndexedSubfeature(const RefIndexedSubfeature&);
    IndexedSubfeature(const RefIndexedSubfeature&, uint32_t bucketInstanceId_, uint16_t collisionGroupId_);

    IndexedSubfeature(const IndexedSubfeature&);
    IndexedSubfeature(IndexedSubfeature&&);

    IndexedSubfeature(std::size_t index_,
                      std::string sourceLayerName_,
                      std::string bucketName_,
                      size_t sortIndex_,
                      uint32_t bucketInstanceId_ = 0,
                      uint16_t collisionGroupId_ = 0);

    IndexedSubfeature& operator=(const IndexedSubfeature&);
    IndexedSubfeature& operator=(IndexedSubfeature&&);

private:
    std::string sourceLayerNameCopy;
    std::string bucketLeaderIDCopy;
};

using FeatureSortOrder = std::shared_ptr<const std::vector<size_t>>;

class DynamicFeatureIndex {
public:
    ~DynamicFeatureIndex();
    void query(std::unordered_map<std::string, std::vector<Feature>>& result,
               const mbgl::ScreenLineString& queryGeometry,
               const TransformState& state) const;

    void insert(std::shared_ptr<Feature> feature, std::shared_ptr<mapbox::geometry::polygon<int64_t>> envelope);

protected:
    struct FeatureRecord {
        std::shared_ptr<Feature> feature;
        std::shared_ptr<mapbox::geometry::polygon<int64_t>> envelope;
    };

    std::vector<FeatureRecord> features;
};

class FeatureIndex {
public:
    FeatureIndex(std::unique_ptr<const GeometryTileData> tileData_);

    const GeometryTileData* getData() { return tileData.get(); }

    /// Set the expected number of elements per cell to avoid small re-allocations for populated cells
    void reserve(std::size_t value) { grid.reserve(value); }

    void insert(const GeometryCollection&,
                std::size_t index,
                const std::string& sourceLayerName,
                const std::string& bucketLeaderID);

    void query(std::unordered_map<std::string, std::vector<Feature>>& result,
               const GeometryCoordinates& queryGeometry,
               const TransformState&,
               const mat4& posMatrix,
               double tileSize,
               double scale,
               const RenderedQueryOptions& options,
               const UnwrappedTileID&,
               const std::unordered_map<std::string, const RenderLayer*>&,
               float additionalQueryPadding,
               const SourceFeatureState& sourceFeatureState) const;

    static std::optional<GeometryCoordinates> translateQueryGeometry(const GeometryCoordinates& queryGeometry,
                                                                     const std::array<float, 2>& translate,
                                                                     style::TranslateAnchorType,
                                                                     float bearing,
                                                                     float pixelsToTileUnits);

    void setBucketLayerIDs(const std::string& bucketLeaderID, const std::vector<std::string>& layerIDs);

    std::unordered_map<std::string, std::vector<Feature>> lookupSymbolFeatures(
        const std::vector<IndexedSubfeature>& symbolFeatures,
        const RenderedQueryOptions& options,
        const std::unordered_map<std::string, const RenderLayer*>& layers,
        const OverscaledTileID& tileID,
        const FeatureSortOrder& featureSortOrder) const;

private:
    void addFeature(std::unordered_map<std::string, std::vector<Feature>>& result,
                    const RefIndexedSubfeature&,
                    const RenderedQueryOptions& options,
                    const CanonicalTileID&,
                    const std::unordered_map<std::string, const RenderLayer*>&,
                    const GeometryCoordinates& queryGeometry,
                    const TransformState& transformState,
                    float pixelsToTileUnits,
                    const mat4& posMatrix,
                    const SourceFeatureState* sourceFeatureState) const;

    GridIndex<RefIndexedSubfeature> grid;
    unsigned int sortIndex = 0;

    // mbgl::unordered_* cannot be used here, as we rely on holding references to elements:
    //     22.2.7 Unordered associative containers [unord.req]
    //     The insert and emplace members shall not affect the validity of references to
    //     container elements, but may invalidate all iterators to the container.
    std::unordered_map<std::string, std::vector<std::string>> bucketLayerIDs;
    std::unordered_set<std::string> uniqueLayerIDs;
    std::unique_ptr<const GeometryTileData> tileData;
};
} // namespace mbgl
