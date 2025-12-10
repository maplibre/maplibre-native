#pragma once

#include <mbgl/plugin/cross_platform_plugin.hpp>

namespace plugin::ex {

/// A basic example cross-platform plugin that demonstrates the plugin architecture.
/// This plugin logs lifecycle events and provides a method to set the camera to San Francisco.
class BasicPluginExample : public mbgl::platform::XPlatformPlugin {
public:
    BasicPluginExample();
    ~BasicPluginExample() override;

    // XPlatformPlugin interface
    void onLoad(mbgl::Map* map, mbgl::Renderer* renderer) override;
    void onUnload() override;

    // MapObserver lifecycle overrides
    void onWillStartLoadingMap() override;
    void onDidFinishLoadingMap() override;
    void onDidFailLoadingMap(mbgl::MapLoadError error, const std::string& message) override;

    // Custom plugin methods
    void showSanFrancisco();

private:
    mbgl::Map* map_ = nullptr;
    mbgl::Renderer* renderer_ = nullptr;
};

} // namespace plugin::ex
