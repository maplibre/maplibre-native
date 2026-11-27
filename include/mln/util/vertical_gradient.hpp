#pragma once

#include <mln/util/feature.hpp>

#include <array>
#include <span>

namespace mln {

/// Configuration for the vertical shading applied to the sides of fill-extrusion
/// geometry.
///
/// A style supplies this either as a boolean or as an array of one or two
/// numbers, `[depth, referenceHeight]`.
///
/// There is also no curve-shape parameter. A curve would require per-fragment
/// evaluation or a subdivided wall mesh.
struct VerticalGradient {
    /// How dark the foot of a wall gets, as a multiple of the shading `true` applies:
    /// 0 is a no-op, 0.5 matches `true` exactly at every light intensity, 1 doubles it.
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

    /// Whether the given `[depth]` or `[depth, referenceHeight]` values are in range:
    /// depth within 0-1, referenceHeight non-negative.
    static bool isInRange(const std::span<const float>& values);

    /// The message the style-facing entry points report when `isInRange` fails.
    static constexpr auto rangeErrorMessage = "value must be [depth (0-1), referenceHeight (>= 0)]";

    float depth = defaultDepth;
    /// Selects whether the shading is scaled by building height. 0 shades every building
    /// equally; a positive value restores the height-scaled ramp above that height in
    /// meters, which is what `true` does at 150. `depth` applies either way.
    float referenceHeight = legacyReferenceHeight;

    bool operator==(const VerticalGradient&) const = default;

    std::array<float, 2> toArray() const;

    // Used by ValueFactory<VerticalGradient>::make()
    mln::Value serialize() const;
};

} // namespace mln
