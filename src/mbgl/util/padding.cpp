#include <mbgl/util/padding.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {
std::array<float, 4> Padding::toArray() const {
    return {{top, right, bottom, left}};
}

mbgl::Value Padding::serialize() const {
    return std::vector<mbgl::Value>{top, right, bottom, left};
}
} // namespace mbgl
