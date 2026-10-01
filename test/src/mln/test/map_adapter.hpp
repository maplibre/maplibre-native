#pragma once

#include <mln/map/map.hpp>
#include <mln/map/map_impl.hpp>

namespace mln {

class FileSource;

// Non-public version of mln::Map that accepts a file source as parameter.
class MapAdapter : public Map {
public:
    explicit MapAdapter(RendererFrontend& frontend,
                        MapObserver& observer,
                        std::shared_ptr<FileSource> fileSource,
                        const MapOptions& options)
        : Map(std::make_unique<Map::Impl>(frontend, observer, std::move(fileSource), options)) {}
};

} // namespace mln
