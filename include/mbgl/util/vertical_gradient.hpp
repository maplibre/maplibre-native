#pragma once

#include <mbgl/util/feature.hpp>

#include <array>
#include <span>

namespace mbgl {

/// Configuration for the vertical shading applied to the sides of fill-extrusion
/// geometry.
///
/// A style supplies this either as a boolean or as an array of one or two
/// numbers, `[depth, referenceHeight]`.
///
/// There is also no curve-shape parameter. A curve would require per-fragment
/// evaluation or a subdivided wall mesh.
struct VerticalGradient {
    /// How dark the foot of a wall gets, scaled by the light intensity. 0 is a no-op.
    static constexpr float defaultDepth = 0.5f;
    /// Height at which the legacy gradient reaches full strength. Shorter buildings
    /// are shaded proportionally less.
    static constexpr float legacyReferenceHeight = 150.0f;

    /// Defaults to the legacy behaviour, matching the property's `true` default.
    VerticalGradient() = default;

    /// `true` selects the legacy height-scaled gradient, `false` disables shading.
    explicit VerticalGradient(bool enabled);

    /// `[depth]` or `[depth, referenceHeight]`.
    explicit VerticalGradient(const std::span<const float>& values);

    float depth = defaultDepth;
    /// 0 disables height scaling, so every building is shaded regardless of height.
    /// A positive value runs the legacy gradient against it.
    float referenceHeight = legacyReferenceHeight;

    bool operator==(const VerticalGradient&) const = default;

    std::array<float, 2> toArray() const;

    // Used by ValueFactory<VerticalGradient>::make()
    mbgl::Value serialize() const;
};

} // namespace mbgl
