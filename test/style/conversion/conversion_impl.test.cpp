#include <mln/test/util.hpp>
#include <mln/style/conversion_impl.hpp>

namespace mln {
namespace style {
namespace conversion {

class MockConvertible {
public:
    MockConvertible() = default;
    MockConvertible(int* counter_)
        : counter(counter_) {}

    MockConvertible(MockConvertible&& other) noexcept
        : counter(other.counter) {}

    ~MockConvertible() { ++*counter; }

    int* counter = nullptr;
};

template <>
class ConversionTraits<MockConvertible> {
public:
    static bool isUndefined(const MockConvertible&) { return false; }

    static bool isArray(const MockConvertible&) { return false; }

    static bool isObject(const MockConvertible&) { return false; }

    static std::size_t arrayLength(const MockConvertible&) { return 0u; }

    static MockConvertible arrayMember(const MockConvertible&, std::size_t) { return {}; }

    static std::optional<MockConvertible> objectMember(const MockConvertible&, const char*) { return std::nullopt; }

    template <class Fn>
    static std::optional<Error> eachMember(const MockConvertible&, Fn&&) {
        return std::nullopt;
    }

    static std::optional<bool> toBool(const MockConvertible&) { return std::nullopt; }

    static std::optional<float> toNumber(const MockConvertible&) { return std::nullopt; }

    static std::optional<double> toDouble(const MockConvertible&) { return std::nullopt; }

    static std::optional<std::string> toString(const MockConvertible&) { return std::nullopt; }

    static std::optional<mln::Value> toValue(const MockConvertible&) { return std::nullopt; }

    static std::optional<GeoJSON> toGeoJSON(const MockConvertible&, Error&) { return std::nullopt; }
};

} // namespace conversion
} // namespace style
} // namespace mln

using namespace mln;
using namespace mln::style;
using namespace mln::style::conversion;

TEST(Conversion, Move) {
    int dtorCounter = 0;

    {
        MockConvertible mock(&dtorCounter);
        Convertible a(std::move(mock));
        Convertible b(std::move(a));
        a = std::move(b);
        Convertible* c = &a;
        a = std::move(*c);
    }

    ASSERT_EQ(dtorCounter, 4);
}
