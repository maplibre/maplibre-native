#pragma once

#include <mln/style/conversion.hpp>
#include <mln/util/feature.hpp>

namespace mln {

class RenderTile;

class SourceFeatureState {
public:
    SourceFeatureState() = default;
    ~SourceFeatureState() = default;

    void updateState(const std::optional<std::string>& sourceLayerID,
                     const std::string& featureID,
                     const FeatureState& newState);
    void getState(FeatureState& result,
                  const std::optional<std::string>& sourceLayerID,
                  const std::string& featureID) const;
    void removeState(const std::optional<std::string>& sourceLayerID,
                     const std::optional<std::string>& featureID,
                     const std::optional<std::string>& stateKey);

    void coalesceChanges(std::vector<RenderTile>& tiles);

private:
    LayerFeatureStates currentStates;
    LayerFeatureStates stateChanges;
    LayerFeatureStates deletedStates;
};

} // namespace mln
