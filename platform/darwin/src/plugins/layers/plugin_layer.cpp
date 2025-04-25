//
//  plugin_layer.cpp
//  App
//
//  Created by Malcolm Toon on 4/23/25.
//

#include "plugin_layer.hpp"
#include "plugin_layer_impl.hpp"

namespace mbgl {
namespace style {

namespace {
const LayerTypeInfo typeInfoCustom{"plugin",
                                   LayerTypeInfo::Source::NotRequired,
                                   LayerTypeInfo::Pass3D::NotRequired,
                                   LayerTypeInfo::Layout::NotRequired,
                                   LayerTypeInfo::FadingTiles::NotRequired,
                                   LayerTypeInfo::CrossTileIndex::NotRequired,
                                   LayerTypeInfo::TileKind::NotRequired};
} // namespace

PluginLayer::PluginLayer(const std::string& layerID, std::unique_ptr<PluginLayerHost> host)
    : Layer(makeMutable<Impl>(layerID, std::move(host))) {}

PluginLayer::~PluginLayer() = default;

const PluginLayer::Impl& PluginLayer::impl() const {
    return static_cast<const Impl&>(*baseImpl);
}

Mutable<PluginLayer::Impl> PluginLayer::mutableImpl() const {
    return makeMutable<Impl>(impl());
}

std::unique_ptr<Layer> PluginLayer::cloneRef(const std::string&) const {
    assert(false);
    return nullptr;
}

using namespace conversion;

std::optional<Error> PluginLayer::setPropertyInternal(const std::string&, const Convertible&) {
    return Error{"layer doesn't support this property"};
}

StyleProperty PluginLayer::getProperty(const std::string&) const {
    return {};
}

Mutable<Layer::Impl> PluginLayer::mutableBaseImpl() const {
    return staticMutableCast<Layer::Impl>(mutableImpl());
}

// static
const LayerTypeInfo* PluginLayer::Impl::staticTypeInfo() noexcept {
    return &typeInfoCustom;
}

} // namespace style
} // namespace mbgl
