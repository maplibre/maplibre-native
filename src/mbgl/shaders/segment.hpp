#pragma once

#include <mbgl/gfx/draw_scope.hpp>

#include <cstddef>
#include <vector>
#include <map>
#include <string>

namespace mbgl {

class SegmentBase {
public:
    SegmentBase(std::size_t vertexOffset_,
                std::size_t indexOffset_,
                std::size_t vertexLength_ = 0,
                std::size_t indexLength_ = 0,
                float sortKey_ = 0.0f)
        : vertexOffset(vertexOffset_),
          indexOffset(indexOffset_),
          vertexLength(vertexLength_),
          indexLength(indexLength_),
          sortKey(sortKey_) {}

    // FIXME: clang-tidy-8 still complains here and clang-cl
    // on Windows gives the error: "exception specification of
    // explicitly defaulted move constructor does not match
    // the calculated one" when marking this 'noexcept'.
    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    SegmentBase(SegmentBase&&) = default;

    const std::size_t vertexOffset;
    const std::size_t indexOffset;

    std::size_t vertexLength;
    std::size_t indexLength;

    // One DrawScope per layer ID. This minimizes rebinding in cases where
    // several layers share buckets but have different sets of active
    // attributes. This can happen:
    //   * when two layers have the same layout properties, but differing
    //     data-driven paint properties
    //   * when two fill layers have the same layout properties, but one
    //     uses fill-color and the other uses fill-pattern
    mutable std::map<std::string, gfx::DrawScope> drawScopes;

    float sortKey;
};

using SegmentVector = std::vector<SegmentBase>;

} // namespace mbgl
