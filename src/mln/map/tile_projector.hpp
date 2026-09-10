#pragma once

#include <mln/map/projection_base.hpp>
#include <mln/map/transform_state.hpp>
#include <mln/tile/tile_id.hpp>

namespace mln {

/// Projects the points of one tile through the current projection, the way the vertex shaders do it.
class TileProjector {
public:
    TileProjector(const TransformState& state_, const UnwrappedTileID& tileID_)
        : TileProjector(state_, tileID_, state_.getProjectionData(tileID_)) {}

    TileProjector(const TransformState& state_, const UnwrappedTileID& tileID_, ProjectionData data_)
        : state(&state_),
          tileID(tileID_),
          data(std::move(data_)) {}

    ProjectedTilePoint project(const Point<double>& point) const {
        return state->getProjection().projectTilePoint(data, tileID, point);
    }

    double circleRadiusCorrection() const { return state->getProjection().circleRadiusCorrection(*state); }

    double pitchedTextCorrection(const Point<double>& tileAnchor) const {
        return state->getProjection().pitchedTextCorrection(*state, tileAnchor, tileID);
    }

    const TransformState& getTransformState() const { return *state; }
    const UnwrappedTileID& getTileID() const { return tileID; }
    const ProjectionData& getProjectionData() const { return data; }

private:
    const TransformState* state;
    UnwrappedTileID tileID;
    ProjectionData data;
};

} // namespace mln
