#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <cstdint>
#include <mbgl/util/convert.hpp>

namespace mbgl {
namespace util {

template std::array<float, 2> convert(const std::array<int32_t, 2>&);

} // namespace util
} // namespace mbgl

#ifdef _MSC_VER
#pragma warning(pop)
#endif
