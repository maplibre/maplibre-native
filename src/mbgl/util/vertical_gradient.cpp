#include <mbgl/util/vertical_gradient.hpp>

#include <stdexcept>

namespace mbgl {

VerticalGradient::VerticalGradient(bool enabled) {
    if (!enabled) {
        depth = 0.0f;
        referenceHeight = 0.0f;
    }
}

VerticalGradient::VerticalGradient(const std::span<const float>& values) {
    if (values.empty() || values.size() > 2) {
        throw std::invalid_argument("VerticalGradient must have between 1 and 2 elements");
    }

    depth = values[0];
    referenceHeight = values.size() > 1 ? values[1] : 0.0f;
}

std::array<float, 2> VerticalGradient::toArray() const {
    return {{depth, referenceHeight}};
}

mbgl::Value VerticalGradient::serialize() const {
    return std::vector<mbgl::Value>{depth, referenceHeight};
}

} // namespace mbgl
