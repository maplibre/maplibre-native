#include <mbgl/util/util.hpp>

#include <array>
#include <cstddef>
#include <type_traits>

namespace mbgl {
namespace util {

template <typename To, typename From, std::size_t Size, typename = std::enable_if_t<std::is_convertible_v<From, To>>>
MBGL_CONSTEXPR std::array<To, Size> convert(const std::array<From, Size>& from) {
    std::array<To, Size> to{{}};
    std::copy(std::begin(from), std::end(from), std::begin(to));
    return to;
}

} // namespace util
} // namespace mbgl
