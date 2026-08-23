#pragma once

#include <mln/actor/actor_ref.hpp>

#include <memory>
#include <string>

namespace mln {

class RasterTile;

class RasterTileWorker {
public:
    RasterTileWorker(const ActorRef<RasterTileWorker>&, ActorRef<RasterTile>);

    void parse(const std::shared_ptr<const std::string>& data, uint64_t correlationID);

private:
    ActorRef<RasterTile> parent;
};

} // namespace mln
