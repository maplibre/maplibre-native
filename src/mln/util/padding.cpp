#include <mln/util/padding.hpp>
#include <mln/util/string.hpp>

namespace mln {
std::array<float, 4> Padding::toArray() const {
    return {{top, right, bottom, left}};
}

mln::Value Padding::serialize() const {
    return std::vector<mln::Value>{top, right, bottom, left};
}
} // namespace mln
