#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geometry.hpp>

#include <string>
#include <map>
#include <vector>

namespace mbgl {

using AnchorOffsetMap = std::map<style::SymbolAnchorType, std::array<float, 2>>;

class VariableAnchorOffsetCollection {
    
public:
    VariableAnchorOffsetCollection() = default;

    // Copy constructor
    VariableAnchorOffsetCollection(const VariableAnchorOffsetCollection& other) {
        anchorOffsets = other.anchorOffsets;
    }

    // Move constructor
    VariableAnchorOffsetCollection(VariableAnchorOffsetCollection&& other) noexcept {
        anchorOffsets = std::move(other.anchorOffsets);
    }
  
    VariableAnchorOffsetCollection(const AnchorOffsetMap& values) {
        anchorOffsets = std::move(values);
    }

    std::string toString() const;
    mbgl::Value serialize() const;
    bool empty() const;
    AnchorOffsetMap getOffsets() const;

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

    bool operator==(const VariableAnchorOffsetCollection& other) const {
        return anchorOffsets == other.anchorOffsets;
    }

private:
    AnchorOffsetMap anchorOffsets;
};

} // namespace mbgl
