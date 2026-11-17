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
                 const LayoutParameters* = nullptr);

    bool hasDependencies() const override { return needsImages || !resourceURLs.empty(); }

    void prepareResources(const LayoutResourceMap&) override;

    void createBucket(const ImagePositions&,
                      std::unique_ptr<FeatureIndex>&,
                      mln::unordered_map<std::string, LayerRenderData>&,
                      bool,
                      bool,
                      const CanonicalTileID&) override;

private:
    // The handle of the resource that a RESOURCE value names, or null.
    void* resourceHandle(const plugin::PropertyDefinition&, const mln_plugin_value&) const;

    bool needsImages = false;
    // The URLs that the RESOURCE properties of the layer name in this tile, and the handles made from them.
    LayoutResourceDependencies resourceURLs;
    std::map<std::string, std::shared_ptr<void>> resourceHandles;
    const float zoom;
    const bool stillImage;
    const std::shared_ptr<const GlobalStateMap> globalState;
    std::vector<Immutable<style::LayerProperties>> layers;
    std::unique_ptr<GeometryTileLayer> sourceLayer;
    plugin::RegisteredLayerPtr registration;
};

} // namespace mln
