#pragma once

#include <mln/layout/layout.hpp>
#include <mln/plugin/plugin_registry.hpp>
#include <mln/renderer/bucket_parameters.hpp>
#include <mln/style/layer_properties.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

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
    // The URL that a RESOURCE property's name maps to in the layer's RESOURCES property, or null.
    const std::string* resourceURL(const plugin::PropertyDefinition&, const std::string& name) const;
    // The handle of the resource that a RESOURCE property's name maps to, or null.
    void* resourceHandle(const plugin::PropertyDefinition&, const std::string& name) const;

    // A feature that passes the layer's filter, with the names its feature-dependent RESOURCE values evaluated to, in
    // the order of featureResourceDefinitions. The constructor finds them while it collects the resources, so that
    // createBucket evaluates neither the filter nor these values again.
    struct DrawnFeature {
        std::size_t index;
        std::vector<std::string> resourceNames;
    };
    std::vector<const plugin::PropertyDefinition*> featureResourceDefinitions;
    // Set only when the layer has a feature-dependent RESOURCE value.
    std::optional<std::vector<DrawnFeature>> drawnFeatures;

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
