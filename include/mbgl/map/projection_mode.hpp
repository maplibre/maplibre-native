#pragma once

#include <functional>
#include <optional>

namespace mbgl {

/**
 * @brief Holds values for Axonometric rendering. All fields are
 * optional.
 */
struct ProjectionMode {
    ProjectionMode& withAxonometric(bool o) {
        axonometric = o;
        return *this;
    }
    ProjectionMode& withXSkew(double o) {
        xSkew = o;
        return *this;
    }
    ProjectionMode& withYSkew(double o) {
        ySkew = o;
        return *this;
    }

    /**
     * @brief Set to True to enable axonometric rendering, false otherwise.
     */
    std::optional<bool> axonometric;

    /**
     * @brief The X skew value represents how much to skew on the x-axis.
     */
    std::optional<double> xSkew;

    /**
     * @brief The Y skew value represents how much to skew on the y-axis.
     */
    std::optional<double> ySkew;
};

} // namespace mbgl
