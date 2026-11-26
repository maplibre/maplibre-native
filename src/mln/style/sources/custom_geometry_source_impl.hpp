#pragma once

#include <mln/style/source_impl.hpp>
#include <mln/style/sources/custom_geometry_source.hpp>
#include <mln/style/custom_tile_loader.hpp>
#include <mln/actor/actor_ref.hpp>

namespace mln {
namespace style {

class CustomGeometrySource::Impl : public Source::Impl {
public:
    Impl(std::string id, const CustomGeometrySource::Options& options);
    Impl(const Impl&, const ActorRef<CustomTileLoader>&);

    std::optional<std::string> getAttribution() const final;

    Immutable<CustomGeometrySource::TileOptions> getTileOptions() const;
    Range<uint8_t> getZoomRange() const;
    std::optional<ActorRef<CustomTileLoader>> getTileLoader() const;
    bool operator!=(const Impl&) const noexcept;

private:
    Immutable<CustomGeometrySource::TileOptions> tileOptions;
    Range<uint8_t> zoomRange;
    std::optional<ActorRef<CustomTileLoader>> loaderRef;
};

} // namespace style
} // namespace mln
