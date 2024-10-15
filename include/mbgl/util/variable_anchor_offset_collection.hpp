#pragma once

#include <mbgl/style/types.hpp>
#include <mbgl/util/feature.hpp>
#include <mbgl/util/geometry.hpp>

#include <array>
#include <string>
#include <vector>

namespace mbgl {

struct AnchorOffsetPair {
    style::SymbolAnchorType anchorType;
    std::array<float, 2> offset;

    AnchorOffsetPair(style::SymbolAnchorType anchorType_, std::array<float, 2> offset_)
        : anchorType(anchorType_),
          offset(offset_) {}

    bool operator==(const AnchorOffsetPair& other) const = default;
};

class VariableAnchorOffsetCollection {
private:
    using CollectionType = std::vector<AnchorOffsetPair>;
    CollectionType anchorOffsets;

public:
    VariableAnchorOffsetCollection() = default;

    VariableAnchorOffsetCollection(const VariableAnchorOffsetCollection& other) = default;

    VariableAnchorOffsetCollection(VariableAnchorOffsetCollection&& other) noexcept = default;

    VariableAnchorOffsetCollection(std::vector<AnchorOffsetPair>&& values) { anchorOffsets = std::move(values); }

    std::array<float, 2> getOffsetByAnchor(const style::SymbolAnchorType& anchorType) const;

    std::string toString() const;

    mbgl::Value serialize() const;

    bool empty() const { return anchorOffsets.size() == 0; }

    CollectionType::size_type size() const { return anchorOffsets.size(); }

    CollectionType::const_iterator begin() const { return anchorOffsets.begin(); }

    CollectionType::const_iterator end() const { return anchorOffsets.end(); }

    const AnchorOffsetPair& operator[](size_t index) const { return anchorOffsets[index]; }

    VariableAnchorOffsetCollection& operator=(const VariableAnchorOffsetCollection& other) = default;

    VariableAnchorOffsetCollection& operator=(VariableAnchorOffsetCollection&& other) noexcept = default;

    bool operator==(const VariableAnchorOffsetCollection& other) const = default;
};

} // namespace mbgl
