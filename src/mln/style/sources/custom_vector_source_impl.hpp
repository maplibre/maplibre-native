#pragma once

#include <mln/style/source_impl.hpp>
#include <mln/style/sources/custom_vector_source.hpp>
#include <mln/style/custom_vector_tile_loader.hpp>
#include <mln/actor/actor_ref.hpp>

namespace mln {
namespace style {

class CustomVectorSource::Impl : public Source::Impl {
public:
    Impl(std::string id, const CustomVectorSource::Options& options);
    Impl(const Impl&, const ActorRef<CustomVectorTileLoader>&);

    std::optional<std::string> getAttribution() const final;

    Range<uint8_t> getZoomRange() const;
    std::optional<ActorRef<CustomVectorTileLoader>> getTileLoader() const;
    bool operator!=(const Impl&) const noexcept;

private:
    Range<uint8_t> zoomRange;
    std::optional<ActorRef<CustomVectorTileLoader>> loaderRef;
};

} // namespace style
} // namespace mln
