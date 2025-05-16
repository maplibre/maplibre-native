#include <mbgl/tile/tile_operation.hpp>
#include <mbgl/util/enum.hpp>

namespace mbgl {

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
} // namespace mbgl
