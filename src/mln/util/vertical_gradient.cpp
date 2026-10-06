#include <mln/util/vertical_gradient.hpp>

#include <stdexcept>

namespace mln {

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

bool VerticalGradient::isInRange(const std::span<const float>& values) {
    if (!values.empty() && (values[0] < 0.0f || values[0] > 1.0f)) {
        return false;
    }
    if (values.size() > 1 && values[1] < 0.0f) {
        return false;
    }
    return true;
}

std::array<float, 2> VerticalGradient::toArray() const {
    return {{depth, referenceHeight}};
}

mln::Value VerticalGradient::serialize() const {
    return std::vector<mln::Value>{depth, referenceHeight};
}

} // namespace mln
