#include <mbgl/style/conversion/terrain.hpp>
#include <mbgl/style/conversion/constant.hpp>
#include <mbgl/style/conversion_impl.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<Terrain> Converter<Terrain>::operator()(const Convertible& value, Error& error) const {
    if (!isObject(value)) {
        error.message = "terrain must be an object";
        return std::nullopt;
    }

    // Parse source (required)
    std::optional<Convertible> sourceValue = objectMember(value, "source");
    if (!sourceValue) {
        error.message = "terrain must have a source";
        return std::nullopt;
    }

    std::optional<std::string> source = toString(*sourceValue);
    if (!source) {
        error.message = "terrain source must be a string";
        return std::nullopt;
    }

    // Parse exaggeration (optional, default: 1.0)
    float exaggeration = 1.0f;
    std::optional<Convertible> exaggerationValue = objectMember(value, "exaggeration");
    if (exaggerationValue) {
        std::optional<float> converted = toNumber(*exaggerationValue);
        if (!converted) {
            error.message = "terrain exaggeration must be a number";
            return std::nullopt;
        }
        exaggeration = *converted;
    }

    return Terrain(*source, exaggeration);
}

} // namespace conversion
} // namespace style
} // namespace mbgl
