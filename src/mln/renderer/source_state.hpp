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
    void cancelQueuedRemovals(const std::string& sourceLayer,
                              const std::string& featureID,
                              const FeatureState& newState);
    void cancelQueuedChanges(const std::string& sourceLayer,
                             const std::optional<std::string>& featureID,
                             const std::optional<std::string>& stateKey);

    LayerFeatureStates currentStates;
    LayerFeatureStates stateChanges;
    LayerFeatureStates deletedStates;
};

} // namespace mln
