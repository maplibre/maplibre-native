#include <mbgl/style/conversion/raster_dem_options.hpp>
#include <mbgl/style/conversion_impl.hpp>
#include <mbgl/style/expression/dsl.hpp>

#include <sstream>

namespace mbgl {
namespace style {
namespace conversion {

std::optional<RasterDEMOptions> Converter<RasterDEMOptions>::operator()(const Convertible& value, Error& error) const {
    RasterDEMOptions options;

    auto encodingValue = objectMember(value, "encoding");
    if (encodingValue) {
        std::optional<std::string> encoding = toString(*encodingValue);
        if (encoding && *encoding == "terrarium") {
            options.encoding = {Tileset::Encoding::Terrarium};
        } else if (encoding && (*encoding == "mapbox" || *encoding == "mvt")) {
            options.encoding = {Tileset::Encoding::Mapbox};
        } else if (encoding && *encoding == "mlt") {
            options.encoding = {Tileset::Encoding::MLT};
        } else {
            error.message =
                "invalid encoding - valid types are 'mapbox' and 'terrarium' for raster sources, 'mvt' and 'mlt' for "
                "vector sources";
            return std::nullopt;
        }
    }

    return {std::move(options)};
}

} // namespace conversion
} // namespace style
} // namespace mbgl
