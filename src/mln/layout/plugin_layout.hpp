#pragma once

#include <mln/layout/layout.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/style/layer_properties.hpp>

namespace mln {

class PluginLayout final : public Layout {
public:
    PluginLayout(const BucketParameters&,
                 std::vector<Immutable<style::LayerProperties>>,
                 std::unique_ptr<GeometryTileLayer>,
                 plugin::RegisteredLayerPtr,
                 LayoutResourceDependencies* resourceDependencies = nullptr);

    // A layer with RESOURCE properties waits for the resources its features name.
    bool hasDependencies() const override { return !resourceURLs.empty(); }
    void prepareResources(const LayoutResourceMap&) override;

    void createBucket(const ImagePositions&,
                      std::unique_ptr<FeatureIndex>&,
                      mln::unordered_map<std::string, LayerRenderData>&,
                      bool,
                      bool,
                      const CanonicalTileID&) override;

private:
    // The plugin's handle for a loaded resource, created on first use; null when it is not available.
    void* resourceHandle(const std::string& url);

    const float zoom;
    const std::shared_ptr<const GlobalStateMap> globalState;
    std::vector<Immutable<style::LayerProperties>> layers;
    std::unique_ptr<GeometryTileLayer> sourceLayer;
    plugin::RegisteredLayerPtr registration;
    std::set<std::string> resourceURLs;
    LayoutResourceMap resources;
};

} // namespace mln
