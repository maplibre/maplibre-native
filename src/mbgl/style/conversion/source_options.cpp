#include <mbgl/style/conversion/source_options.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/dsl.hpp>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<SourceOptions> Converter<SourceOptions>::operator()(const Convertible& value, Error& error) const {
    const auto encodingValue = objectMember(value, "encoding");
    if (encodingValue) {
        const auto encoding = toString(*encodingValue);
        if (encoding && *encoding == "terrarium") {
            return {{.encoding = Tileset::Encoding::Terrarium}};
        } else if (encoding && (*encoding == "mapbox" || *encoding == "mvt")) {
            return {{.encoding = Tileset::Encoding::Mapbox}};
        } else if (encoding && *encoding == "mlt") {
            return {{.encoding = Tileset::Encoding::MLT}};
        } else {
            error.message =
                "invalid encoding - valid types are 'mapbox' and 'terrarium' for raster sources, 'mvt' and 'mlt' for "
                "vector sources";
            return std::nullopt;
        }
    }
    return {{.encoding = Tileset::Encoding::Mapbox}};
}

} // namespace conversion
} // namespace style
} // namespace mbgl
