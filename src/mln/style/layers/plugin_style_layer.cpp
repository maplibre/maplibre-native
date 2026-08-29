#include <mln/style/layers/plugin_style_layer.hpp>

namespace mln {
namespace style {

struct PluginStyleLayer::Impl::TypeInfoHolder {
    explicit TypeInfoHolder(const plugin::LayerType& registration)
        : name(registration.type),
          info{name.c_str(),
               registration.sourceKind == MLN_PLUGIN_SOURCE_GEOMETRY ? LayerTypeInfo::Source::Required
                                                                     : LayerTypeInfo::Source::NotRequired,
               registration.requires3D ? LayerTypeInfo::Pass3D::Required : LayerTypeInfo::Pass3D::NotRequired,
               registration.sourceKind == MLN_PLUGIN_SOURCE_GEOMETRY ? LayerTypeInfo::Layout::Required
                                                                     : LayerTypeInfo::Layout::NotRequired,
               LayerTypeInfo::FadingTiles::NotRequired,
               LayerTypeInfo::CrossTileIndex::NotRequired,
               registration.sourceKind == MLN_PLUGIN_SOURCE_GEOMETRY ? LayerTypeInfo::TileKind::Geometry
                                                                     : LayerTypeInfo::TileKind::NotRequired} {}

    std::string name;
    LayerTypeInfo info;
};

PluginStyleLayer::Impl::Impl(const std::string& id, const std::string& source, plugin::LayerType registration_)
    : Layer::Impl(id, source),
      registration(std::move(registration_)),
      typeInfo(std::make_shared<TypeInfoHolder>(registration)) {}

bool PluginStyleLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    return pluginProperties != other.pluginProperties || visibility != other.visibility;
}

void PluginStyleLayer::Impl::stringifyLayout(rapidjson::Writer<rapidjson::StringBuffer>& writer) const {
    writer.StartObject();
    writer.EndObject();
}

const LayerTypeInfo* PluginStyleLayer::Impl::getTypeInfo() const noexcept {
    return &typeInfo->info;
}

expression::Dependency PluginStyleLayer::Impl::getDependencies() const noexcept {
    expression::Dependency result = expression::Dependency::None;
    for (const auto& [name, value] : pluginProperties) {
        (void)name;
        result |= value.getDependencies();
    }
    return result;
}

PluginStyleLayer::PluginStyleLayer(const std::string& id, const std::string& source, plugin::LayerType registration)
    : Layer(makeMutable<Impl>(id, source, std::move(registration))) {}

PluginStyleLayer::PluginStyleLayer(Immutable<Impl> impl_)
    : Layer(std::move(impl_)) {}

PluginStyleLayer::~PluginStyleLayer() = default;

const PluginStyleLayer::Impl& PluginStyleLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<PluginStyleLayer::Impl> PluginStyleLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::optional<conversion::Error> PluginStyleLayer::setPropertyInternal(const std::string&,
                                                                       const conversion::Convertible&) {
    return conversion::Error{"layer doesn't support this property"};
}

StyleProperty PluginStyleLayer::getProperty(const std::string& name) const {
    return getPluginProperty(name);
}

std::unique_ptr<Layer> PluginStyleLayer::cloneRef(const std::string& id) const {
    auto copy = mutableImpl();
    copy->id = id;
    return std::unique_ptr<PluginStyleLayer>(new PluginStyleLayer(std::move(copy)));
}

Mutable<Layer::Impl> PluginStyleLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

} // namespace style
} // namespace mln
