#pragma once

#include <mbgl/layout/symbol_instance.hpp>
#include <mbgl/style/image_impl.hpp>
#include <mbgl/style/layer_impl.hpp>
#include <mbgl/tile/geometry_tile_data.hpp>

#include <mbgl/util/identity.hpp>

#include <atomic>

namespace mbgl {

namespace gfx {
class UploadPass;
} // namespace gfx

class RenderLayer;
class CrossTileSymbolLayerIndex;
class OverscaledTileID;
class PatternDependency;
using PatternLayerMap = std::map<std::string, PatternDependency>;
class Placement;
class TransformState;
class BucketPlacementData;
class RenderTile;

class Bucket {
public:
    Bucket(const Bucket&) = delete;
    Bucket& operator=(const Bucket&) = delete;

    virtual ~Bucket() = default;

    // Feature geometries are also used to populate the feature index.
    // Obtaining these is a costly operation, so we do it only once, and
    // pass-by-const-ref the geometries as a second parameter.
    virtual void addFeature(const GeometryTileFeature&,
                            const GeometryCollection&,
                            const ImagePositions&,
                            const PatternLayerMap&,
                            std::size_t,
                            const CanonicalTileID&) {};

    virtual void update(const FeatureStates&, const GeometryTileLayer&, const std::string&, const ImagePositions&) {}

    // As long as this bucket has a Prepare render pass, this function is
    // getting called. Typically, this only happens once when the bucket is
    // being rendered for the first time.
    virtual void upload(gfx::UploadPass&) = 0;

    virtual bool hasData() const = 0;

    virtual float getQueryRadius(const RenderLayer&) const { return 0; };

    bool needsUpload() const { return hasData() && !uploaded; }

    // The following methods are implemented by buckets that require cross-tile indexing and placement.

    // Returns a pair, the first element of which is a bucket cross-tile id
    // on success call; `0` otherwise. The second element is `true` if
    // the bucket was originally registered; `false` otherwise.
    virtual std::pair<uint32_t, bool> registerAtCrossTileIndex(CrossTileSymbolLayerIndex&, const RenderTile&) {
        return std::make_pair(0u, false);
    }
    // Places this bucket to the given placement.
    virtual void place(Placement&, const BucketPlacementData&, std::set<uint32_t>&) {}
    virtual void updateVertices(
        const Placement&, bool /*updateOpacities*/, const TransformState&, const RenderTile&, std::set<uint32_t>&) {}

    const util::SimpleIdentity& getID() const { return bucketID; }

#if MLN_SYMBOL_GUARDS
    virtual bool check(std::source_location) { return true; }
#else
    bool check(std::string_view = {}) { return true; }
#endif

    const std::optional<std::thread::id>& getRenderThreadID() const { return renderThreadID; }
    void setRenderThreadID(std::optional<std::thread::id> id) { renderThreadID = id; }

protected:
    Bucket() = default;
    std::atomic<bool> uploaded{false};

    util::SimpleIdentity bucketID;

    std::optional<std::thread::id> renderThreadID;
};

} // namespace mbgl
