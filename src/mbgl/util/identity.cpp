#include <mbgl/util/identity.hpp>

#include <atomic>

namespace {
// This is not in the class declaration to minimize include dependencies
std::atomic<std::int64_t> nextId = 1;
} // anonymous namespace

namespace mbgl {
namespace util {

const SimpleIdentity SimpleIdentity::Empty = {emptyID};

SimpleIdentity::SimpleIdentity() noexcept
    : uniqueID(nextId++) {}

} // namespace util
} // namespace mbgl
