#include <mln/style/conversion/sky.hpp>
#include <mln/style/conversion_impl.hpp>

#include <array>
#include <utility>

namespace mln {
namespace style {
namespace conversion {

std::optional<Sky> Converter<Sky>::operator()(const Convertible& value, Error& error) const {
    if (!isObject(value)) {
        error.message = "sky must be an object";
        return std::nullopt;
    }

    static constexpr std::array<const char*, 14> properties{
        "atmosphere-blend",
        "atmosphere-blend-transition",
        "fog-color",
        "fog-color-transition",
        "fog-ground-blend",
        "fog-ground-blend-transition",
        "horizon-color",
        "horizon-color-transition",
        "horizon-fog-blend",
        "horizon-fog-blend-transition",
        "sky-color",
        "sky-color-transition",
        "sky-horizon-blend",
        "sky-horizon-blend-transition",
    };

    Sky sky;
    for (const auto* name : properties) {
        const auto member = objectMember(value, name);
        if (!member) {
            continue;
        }

        if (auto conversionError = sky.setProperty(name, *member)) {
            error = std::move(*conversionError);
            return std::nullopt;
        }
    }

    return sky;
}

} // namespace conversion
} // namespace style
} // namespace mln
