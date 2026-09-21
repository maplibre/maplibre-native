#pragma once

#include <mln/renderer/sources/render_tile_source.hpp>
#include <mln/style/sources/geojson_source_impl.hpp>

namespace mln {

namespace style {
class GeoJSONData;
} // namespace style

class RenderGeoJSONSource final : public RenderTileSource {
public:
    explicit RenderGeoJSONSource(Immutable<style::GeoJSONSource::Impl>, const TaggedScheduler&);
    ~RenderGeoJSONSource() override;

    void update(Immutable<style::Source::Impl>,
                const std::vector<Immutable<style::LayerProperties>>&,
                bool needsRendering,
                bool needsRelayout,
                const TileParameters&) override;

    FeatureExtensionValue queryFeatureExtensions(
        const Feature& feature,
        const std::string& extension,
        const std::string& extensionField,
        const std::optional<std::map<std::string, Value>>& args) const override;

private:
    const style::GeoJSONSource::Impl& impl() const;

    std::weak_ptr<style::GeoJSONData> data;
};

} // namespace mln
