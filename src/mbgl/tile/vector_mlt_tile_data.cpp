#include <mbgl/tile/vector_mlt_tile_data.hpp>

#include <mbgl/util/constants.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#if ANDROID
#include <mlt/decoder.hpp>
#include <mlt/metadata/tileset_protozero.hpp>
#endif

namespace mbgl {

VectorMLTTileFeature::VectorMLTTileFeature(/*const mapbox::vector_tile::layer& layer, const protozero::data_view& view*/)
    //: feature(view/*, layer*/)
{
}

FeatureType VectorMLTTileFeature::getType() const {
    /*
    switch (feature.getType()) {
        case mapbox::vector_tile::GeomType::POINT:
            return FeatureType::Point;
        case mapbox::vector_tile::GeomType::LINESTRING:
            return FeatureType::LineString;
        case mapbox::vector_tile::GeomType::POLYGON:
            return FeatureType::Polygon;
        default:
        */
            return FeatureType::Unknown;
    //}
}

std::optional<Value> VectorMLTTileFeature::getValue(const std::string& /*key*/) const {
    //const std::optional<Value> value(feature.getValue(key));
    //return value->is<NullValue>() ? std::nullopt : value;
    return std::nullopt;
}

const PropertyMap& VectorMLTTileFeature::getProperties() const {
//    if (!properties) {
//        properties = feature.getProperties();
//    }
    return *properties;
}

FeatureIdentifier VectorMLTTileFeature::getID() const {
    return mapbox::feature::null_value; //feature.getID();
}

const GeometryCollection& VectorMLTTileFeature::getGeometries() const {
    MLN_TRACE_FUNC();

//    if (!lines) {
//        const auto scale = static_cast<float>(util::EXTENT) / feature.getExtent();
//
//        try {
//            lines = feature.getGeometries<GeometryCollection>(scale);
//        } catch (const std::runtime_error& ex) {
//            Log::Error(Event::ParseTile, "Could not get geometries: " + std::string(ex.what()));
//            lines = GeometryCollection();
//        }
//
//        if (feature.getVersion() < 2 && feature.getType() == mapbox::vector_tile::GeomType::POLYGON) {
//            lines = fixupPolygons(*lines);
//        }
//    }
    return *lines;
}

VectorMLTTileLayer::VectorMLTTileLayer(std::shared_ptr<const std::string> data_, const protozero::data_view& /*view*/)
    : data(std::move(data_))
    //,layer(view)
    {}

std::size_t VectorMLTTileLayer::featureCount() const {
    return 0;//layer.featureCount();
}

std::unique_ptr<GeometryTileFeature> VectorMLTTileLayer::getFeature(std::size_t /*i*/) const {
    return std::make_unique<VectorMLTTileFeature>(/*layer, layer.getFeature(i)*/);
}

std::string VectorMLTTileLayer::getName() const {
    return {};//layer.getName();
}

VectorMLTTileData::VectorMLTTileData(std::shared_ptr<const std::string> data_)
    : data(std::move(data_)) {}

std::unique_ptr<GeometryTileData> VectorMLTTileData::clone() const {
    return std::make_unique<VectorMLTTileData>(data);
}

std::unique_ptr<GeometryTileLayer> VectorMLTTileData::getLayer(const std::string& name) const {
    MLN_TRACE_FUNC();

    if (!parsed) {
#if ANDROID
        Log::Warning(Event::General, "Parse " + name + " / " + util::toString(data->size()));
        if (data->size() > 4) {
            const auto metadataSize = *reinterpret_cast<const std::uint32_t *>(data->data());
            Log::Warning(Event::General, "Parse " + name + " / " + util::toString(data->size()) + " / " + util::toString(metadataSize) + " / " +
             util::toString(*reinterpret_cast<const std::uint32_t *>(data->data() + 4)));
            if (metadataSize + 4 < data->size()) {
                try {
                    auto metadata = mlt::metadata::tileset::read({data->data() + 4, metadataSize});
                    if (metadata) {
                        auto tile = mlt::Decoder().decode({data->data() + 4 + metadataSize, data->size() - metadataSize - 4}, *metadata);

                        std::string tables;
                        for (const auto& table : metadata->featureTables) {
                            tables += table.name + ",";
                        }

                        Log::Warning(Event::General, "MLT decoded: " + tables);
                    } else {
                        Log::Warning(Event::General, "Metadata parse failed");
                    }
                } catch (const std::exception &ex) {
                    Log::Warning(Event::General, "Metadata parse failed: " + std::string(ex.what()));
                }
            }
        }
#endif

        // We're parsing this lazily so that we can construct VectorTileData
        // objects on the main thread without incurring the overhead of parsing
        // immediately.
    }

    auto it = layers.find(name);
    if (it != layers.end()) {
        return std::make_unique<VectorMLTTileLayer>(data, it->second);
    }
    return nullptr;
}

std::vector<std::string> VectorMLTTileData::layerNames() const {
    return {};//mapbox::vector_tile::buffer(*data).layerNames();
}

} // namespace mbgl
