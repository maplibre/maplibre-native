#include <mbgl/renderer/bucket.hpp>

#include <atomic>

namespace mln {

namespace {
std::atomic<std::size_t> unidentifiedFeatureIndex = 0;
}

std::string Bucket::getRetainFeatureID(const GeometryTileFeature& feature) {
    if (auto idStr = featureIDtoString(feature.getID()); idStr && !idStr->empty()) {
        return std::move(*idStr);
    }
    // Assign a unique number to unidentified features
    return "maplibre:" + std::to_string(unidentifiedFeatureIndex++);
}

} // namespace mln
