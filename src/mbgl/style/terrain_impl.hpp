#pragma once

#include <mbgl/style/terrain.hpp>
#include <mbgl/util/immutable.hpp>
#include <string>

namespace mbgl {
namespace style {

class Terrain::Impl {
public:
    Impl()
        : sourceID(""),
          exaggeration(1.0f) {}
    Impl(const std::string& sourceID_, float exaggeration_)
        : sourceID(sourceID_),
          exaggeration(exaggeration_) {}

    std::string sourceID;
    float exaggeration;

    bool operator==(const Impl& other) const {
        return sourceID == other.sourceID && exaggeration == other.exaggeration;
    }
};

} // namespace style
} // namespace mbgl
