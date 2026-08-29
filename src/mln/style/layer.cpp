#include <mln/plugin/plugin_registry.hpp>
#include <mln/style/conversion/constant.hpp>
#include <mln/style/conversion/filter.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/layer_impl.hpp>
#include <mln/style/layer_observer.hpp>
#include <mln/tile/tile.hpp>

#include <mln/renderer/render_layer.hpp>
#include <mln/util/logging.hpp>

namespace mln {
namespace style {

// Added this to support plugins and that their LayerTypeInfo isn't the same point
// across the board
bool layerTypeInfoEquals(const mln::style::LayerTypeInfo* one, const mln::style::LayerTypeInfo* other) {
    return ((strcmp(one->type, other->type) == 0) && (one->source == other->source) && (one->pass3d == other->pass3d) &&
            (one->layout == other->layout) && (one->fadingTiles == other->fadingTiles) &&
            (one->crossTileIndex == other->crossTileIndex) && (one->tileKind == other->tileKind));
};

static_assert(mln::underlying_type(Tile::Kind::Geometry) == mln::underlying_type(LayerTypeInfo::TileKind::Geometry),
              "tile kind error");
static_assert(mln::underlying_type(Tile::Kind::Raster) == mln::underlying_type(LayerTypeInfo::TileKind::Raster),
              "tile kind error");
static_assert(mln::underlying_type(Tile::Kind::RasterDEM) == mln::underlying_type(LayerTypeInfo::TileKind::RasterDEM),
              "tile kind error");

namespace {
LayerObserver nullObserver;
}

Layer::Layer(Immutable<Impl> impl)
    : baseImpl(std::move(impl)),
      observer(&nullObserver) {}

Layer::~Layer() = default;

std::string Layer::getID() const {
    return baseImpl->id;
}

std::string Layer::getSourceID() const {
    return baseImpl->source;
}

std::string Layer::getSourceLayer() const {
    return baseImpl->sourceLayer;
}

void Layer::setSourceLayer(const std::string& sourceLayer) {
    if (getSourceLayer() == sourceLayer) return;
    auto impl_ = mutableBaseImpl();
    impl_->sourceLayer = sourceLayer;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void Layer::setSourceID(const std::string& sourceID) {
    if (getSourceID() == sourceID) return;
    auto impl_ = mutableBaseImpl();
    impl_->source = sourceID;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
};

const Filter& Layer::getFilter() const {
    return baseImpl->filter;
}

void Layer::setFilter(const Filter& filter) {
    if (getFilter() == filter) return;
    auto impl_ = mutableBaseImpl();
    impl_->filter = filter;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

VisibilityType Layer::getVisibility() const {
    return baseImpl->visibility;
}

void Layer::setVisibility(VisibilityType value) {
    if (value == getVisibility()) return;
    auto impl_ = mutableBaseImpl();
    impl_->visibility = value;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

float Layer::getMinZoom() const {
    return baseImpl->minZoom;
}

float Layer::getMaxZoom() const {
    return baseImpl->maxZoom;
}

void Layer::setMinZoom(float minZoom) {
    if (getMinZoom() == minZoom) return;
    auto impl_ = mutableBaseImpl();
    impl_->minZoom = minZoom;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

void Layer::setMaxZoom(float maxZoom) {
    if (getMaxZoom() == maxZoom) return;
    auto impl_ = mutableBaseImpl();
    impl_->maxZoom = maxZoom;
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
}

Value Layer::serialize() const {
    mapbox::base::ValueObject result;
    result.emplace(std::make_pair("id", getID()));
    result.emplace(std::make_pair("type", Layer::getTypeInfo()->type));

    auto source = getSourceID();
    if (!source.empty()) {
        result.emplace(std::make_pair("source", std::move(source)));
    }

    auto sourceLayer = getSourceLayer();
    if (!sourceLayer.empty()) {
        result.emplace(std::make_pair("source-layer", std::move(sourceLayer)));
    }

    if (getFilter()) {
        result.emplace(std::make_pair("filter", getFilter().serialize()));
    }

    if (getMinZoom() != -std::numeric_limits<float>::infinity()) {
        result.emplace(std::make_pair("minzoom", getMinZoom()));
    }

    if (getMaxZoom() != std::numeric_limits<float>::infinity()) {
        result.emplace(std::make_pair("maxzoom", getMaxZoom()));
    }

    if (getVisibility() == VisibilityType::None) {
        result["layout"] = mapbox::base::ValueObject{std::make_pair("visibility", "none")};
    }

    for (const auto& [name, value] : baseImpl->pluginProperties) {
        const auto definition = plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name);
        if (!definition) continue;
        const auto section = definition->scope == MLN_PLUGIN_PROPERTY_PAINT ? "paint" : "layout";
        auto& object = result[section];
        if (!object.getObject()) object = mapbox::base::ValueObject{};
        const auto property = value.toStyleProperty();
        if (property.getKind() != StyleProperty::Kind::Undefined) {
            object.getObject()->insert_or_assign(name, property.getValue());
        }
    }

    return result;
}

StyleProperty Layer::getPluginProperty(const std::string& name) const {
    const auto value = baseImpl->pluginProperties.find(name);
    if (value != baseImpl->pluginProperties.end()) {
        return value->second.toStyleProperty();
    }
    const auto definition = plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name);
    return definition ? defaultPluginPropertyValue(*definition).toStyleProperty() : StyleProperty{};
}

bool Layer::getPluginBoolean(const std::string& name, bool defaultValue) const {
    const auto property = getPluginProperty(name);
    if (const auto* value = property.getValue().getBool()) return *value;
    return defaultValue;
}

void Layer::serializeProperty(Value& out, const StyleProperty& property, const char* propertyName, bool isPaint) const {
    assert(out.getObject());
    auto& object = *(out.getObject());
    std::string propertyType = isPaint ? "paint" : "layout";
    auto it = object.find(propertyType);
    auto pair = std::make_pair(std::string(propertyName), Value{property.getValue()});
    if (it != object.end()) {
        assert(it->second.getObject());
        it->second.getObject()->emplace(std::move(pair));
    } else {
        object[propertyType] = mapbox::base::ValueObject{{std::move(pair)}};
    }
}

void Layer::setObserver(LayerObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

std::optional<conversion::Error> Layer::setProperty(const std::string& name, const conversion::Convertible& value) {
    using namespace conversion;
    std::optional<Error> error = setPropertyInternal(name, value);
    if (!error) return error; // Successfully set by the derived class implementation.
    if (plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name)) {
        return setPluginProperty(name, value);
    }
    if (name == "visibility") return setVisibility(value);
    if (name == "minzoom") {
        if (auto zoom = convert<float>(value, *error)) {
            setMinZoom(*zoom);
            return std::nullopt;
        }
    } else if (name == "maxzoom") {
        if (auto zoom = convert<float>(value, *error)) {
            setMaxZoom(*zoom);
            return std::nullopt;
        }
    } else if (name == "filter") {
        if (auto filter = convert<Filter>(value, *error)) {
            setFilter(*filter);
            return std::nullopt;
        }
    } else if (name == "source-layer") {
        if (auto sourceLayer = convert<std::string>(value, *error)) {
            if (getTypeInfo()->source != LayerTypeInfo::Source::Required) {
                Log::Warning(mln::Event::General,
                             "'source-layer' property cannot be set to"
                             "the layer " +
                                 baseImpl->id);
                return std::nullopt;
            }
            setSourceLayer(*sourceLayer);
            return std::nullopt;
        }
    } else if (name == "source") {
        if (auto sourceID = convert<std::string>(value, *error)) {
            if (getTypeInfo()->source != LayerTypeInfo::Source::Required) {
                Log::Warning(mln::Event::General,
                             "'source' property cannot be set to"
                             "the layer " +
                                 baseImpl->id);
                return std::nullopt;
            }
            setSourceID(*sourceID);
            return std::nullopt;
        }
    }
    return error;
}

std::optional<conversion::Error> Layer::setProperty(const std::string& name,
                                                    const conversion::Convertible& value,
                                                    PropertyScope scope) {
    using namespace conversion;
    std::optional<Error> error = setPropertyInternal(name, value);
    if (!error) return error;
    if (plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name)) {
        return setPluginProperty(name, value, scope);
    }
    return setProperty(name, value);
}

std::optional<conversion::Error> Layer::setPluginProperty(const std::string& name,
                                                          const conversion::Convertible& value,
                                                          std::optional<PropertyScope> scope) {
    using namespace conversion;
    const auto definition = plugin::PluginRegistry::get().findProperty(getTypeInfo()->type, name);
    if (!definition) return Error{"layer doesn't support this property"};
    if (scope) {
        const auto expected = definition->scope == MLN_PLUGIN_PROPERTY_PAINT ? PropertyScope::Paint
                                                                             : PropertyScope::Layout;
        if (*scope != expected) {
            return Error{"plugin property '" + name + "' is in the wrong style section"};
        }
    }

    auto impl_ = mutableBaseImpl();
    if (isUndefined(value)) {
        if (impl_->pluginProperties.erase(name) != 0) {
            baseImpl = std::move(impl_);
            observer->onLayerChanged(*this);
        }
        return std::nullopt;
    }

    Error conversionError;
    auto converted = convertPluginPropertyValue(*definition, value, conversionError);
    if (!converted) return conversionError;
    const auto existing = impl_->pluginProperties.find(name);
    if (existing != impl_->pluginProperties.end() && existing->second == *converted) return std::nullopt;
    impl_->pluginProperties.insert_or_assign(name, std::move(*converted));
    baseImpl = std::move(impl_);
    observer->onLayerChanged(*this);
    return std::nullopt;
}

std::optional<conversion::Error> Layer::setVisibility(const conversion::Convertible& value) {
    using namespace conversion;

    if (isUndefined(value)) {
        setVisibility(VisibilityType::Visible);
        return std::nullopt;
    }

    Error error;
    std::optional<VisibilityType> visibility = convert<VisibilityType>(value, error);
    if (!visibility) {
        return error;
    }

    setVisibility(*visibility);
    return std::nullopt;
}

const LayerTypeInfo* Layer::getTypeInfo() const noexcept {
    return baseImpl->getTypeInfo();
}

/// Collect dependencies
expression::Dependency Layer::getDependencies() const noexcept {
    return baseImpl->getDependencies();
}

} // namespace style
} // namespace mln
