#include <mln/tile/tile_operation.hpp>
#include <mln/util/enum.hpp>

namespace mln {

MBGL_DEFINE_ENUM(TileOperation,
                 {
                     {TileOperation::RequestedFromCache, "RequestedFromCache"},
                     {TileOperation::RequestedFromNetwork, "RequestedFromNetwork"},
                     {TileOperation::LoadFromNetwork, "LoadFromNetwork"},
                     {TileOperation::LoadFromCache, "LoadFromCache"},
                     {TileOperation::StartParse, "StartParse"},
                     {TileOperation::EndParse, "EndParse"},
                     {TileOperation::Error, "Error"},
                     {TileOperation::Cancelled, "Cancelled"},
                     {TileOperation::NullOp, "NullOp"},
                 });
} // namespace mln
