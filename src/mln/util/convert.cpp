#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

#include <cstdint>
#include <mln/util/convert.hpp>

namespace mln {
namespace util {

template std::array<float, 2> convert(const std::array<int32_t, 2>&);

} // namespace util
} // namespace mln

#ifdef _MSC_VER
#pragma warning(pop)
#endif
