#pragma once

#include <mbgl/layout/layout.hpp>
#include <mbgl/plugin/plugin_registry.hpp>
#include <mbgl/renderer/bucket_parameters.hpp>
#include <mbgl/style/layer_properties.hpp>

namespace mbgl {

class PluginLayout final : public Layout {
public:
    PluginLayout(const BucketParameters&,
                 std::vector<Immutable<style::LayerProperties>>,
                 std::unique_ptr<GeometryTileLayer>,
                 plugin::LayerType);

    bool hasDependencies() const override { return false; }

    void createBucket(const ImagePositions&,
                      std::unique_ptr<FeatureIndex>&,
                      mbgl::unordered_map<std::string, LayerRenderData>&,
                      bool,
                      bool,
                      const CanonicalTileID&) override;

private:
    const OverscaledTileID tileID;
    const float zoom;
    std::shared_ptr<FileSource> fileSource;
    std::vector<Immutable<style::LayerProperties>> layers;
    std::unique_ptr<GeometryTileLayer> sourceLayer;
    plugin::LayerType registration;
};

} // namespace mbgl
