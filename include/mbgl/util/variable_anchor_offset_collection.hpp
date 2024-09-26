#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geometry.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace mbgl {

using AnchorOffsetPair = std::pair<style::SymbolAnchorType, std::array<float, 2>>;

// BUGBUG consider changing data types here. Current impl using extra
// heap allocs due to vector
class VariableAnchorOffsetCollection {
public:
    VariableAnchorOffsetCollection() = default;

    // Copy constructor
    VariableAnchorOffsetCollection(const VariableAnchorOffsetCollection& other) { anchorOffsets = other.anchorOffsets; }

    // Move constructor
    VariableAnchorOffsetCollection(VariableAnchorOffsetCollection&& other) noexcept {
        anchorOffsets = std::move(other.anchorOffsets);
    }

    // BUGBUG add overload to take advantage of std::move.
    VariableAnchorOffsetCollection(const std::vector<AnchorOffsetPair>& values) { anchorOffsets = values; }

    std::string toString() const;
    mbgl::Value serialize() const;
    bool empty() const;
    std::vector<AnchorOffsetPair> getOffsets() const;
    std::array<float, 2> getOffsetByAnchor(const style::SymbolAnchorType& anchorType) const;

    // Copy assignment operator
    VariableAnchorOffsetCollection& operator=(VariableAnchorOffsetCollection& other) {
        if (this != &other) {
            anchorOffsets = other.anchorOffsets;
        }

        return *this;
    }

    // Move assignment operator
    VariableAnchorOffsetCollection& operator=(VariableAnchorOffsetCollection&& other) noexcept {
        if (this != &other) {
            anchorOffsets = std::move(other.anchorOffsets);
        }

        return *this;
    }

    bool operator==(const VariableAnchorOffsetCollection& other) const { return anchorOffsets == other.anchorOffsets; }

private:
    std::vector<AnchorOffsetPair> anchorOffsets;
};

} // namespace mbgl
