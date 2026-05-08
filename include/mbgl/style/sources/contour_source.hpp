#pragma once

#include <mbgl/algorithm/contour/intervals.hpp>
#include <mbgl/algorithm/contour/units.hpp>
#include <mbgl/style/source.hpp>

#include <cstdint>
#include <string>

namespace mbgl {
namespace style {

// Configuration for a `style::ContourSource`. Mirrors the `contour` source
// type from the MapLibre style spec proposal (maplibre-style-spec#583):
//
//   {
//     "type": "contour",
//     "source": "dem",
//     "intervals": [200, 12, 100, 14, 50, 15, 20],
//     "unit": "feet",
//     "majorMultiplier": 5,
//     "overzoom": 1
//   }
struct ContourSourceOptions {
    // ID of the upstream `raster-dem` source to derive contours from.
    std::string sourceID;
    // Per-zoom contour interval schedule (see algorithm/contour/intervals.hpp).
    algorithm::contour::IntervalSchedule intervals;
    // Display unit for emitted `ele` / `interval` feature attributes. Defaults
    // to metres; the underlying DEM is always interpreted as metres.
    algorithm::contour::UnitConfig unit;
    // Every Nth contour level is tagged `major: true` on emitted features.
    // Defaults to 5 (i.e. major lines every 5 × interval).
    std::uint32_t majorMultiplier = 5;
    // Number of zoom levels to overzoom from the upstream DEM source. 0 means
    // sample the DEM at the contour-tile zoom directly.
    std::uint8_t overzoom = 0;
};

// Source-on-source: reads decoded DEM data from an upstream `raster-dem`
// source's tile pyramid and emits contour-line features. This class is the
// style-spec-side handle that the parser produces; the render-side
// integration lives in `RenderContourSource`.
class ContourSource final : public Source {
public:
    ContourSource(std::string id, ContourSourceOptions options);
    ~ContourSource() override;

    const std::string& getDEMSourceID() const;
    const algorithm::contour::IntervalSchedule& getIntervals() const;
    const algorithm::contour::UnitConfig& getUnit() const;
    std::uint32_t getMajorMultiplier() const;
    std::uint8_t getOverzoom() const;

    class Impl;
    const Impl& impl() const;

    void loadDescription(FileSource&) final;

    bool supportsLayerType(const mbgl::style::LayerTypeInfo*) const override;

    mapbox::base::WeakPtr<Source> makeWeakPtr() override { return weakFactory.makeWeakPtr(); }

protected:
    Mutable<Source::Impl> createMutable() const noexcept final;

private:
    mapbox::base::WeakPtrFactory<Source> weakFactory{this};
    // Do not add members here, see `WeakPtrFactory`.
};

template <>
inline bool Source::is<ContourSource>() const {
    return getType() == SourceType::Contour;
}

} // namespace style
} // namespace mbgl
