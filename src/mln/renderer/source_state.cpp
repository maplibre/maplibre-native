#include <mln/renderer/render_tile.hpp>
#include <mln/renderer/source_state.hpp>
#include <mln/style/conversion_impl.hpp>
#include <mln/util/instrumentation.hpp>
#include <mln/util/logging.hpp>

namespace mln {

void SourceFeatureState::updateState(const std::optional<std::string>& sourceLayerID,
                                     const std::string& featureID,
                                     const FeatureState& newState) {
    std::string sourceLayer = sourceLayerID.value_or(std::string());
    for (const auto& state : newState) {
        auto& layerStates = stateChanges[sourceLayer];
        auto& featureStates = layerStates[featureID];
        featureStates[state.first] = state.second;
    }
    // coalesceChanges applies removals after changes, so a removal queued before this change would undo it.
    cancelQueuedRemovals(sourceLayer, featureID, newState);
}

void SourceFeatureState::cancelQueuedRemovals(const std::string& sourceLayer,
                                              const std::string& featureID,
                                              const FeatureState& newState) {
    const auto layerDeletions = deletedStates.find(sourceLayer);
    if (layerDeletions == deletedStates.end()) {
        return;
    }
    const auto removalOfKeysNotInNewState = [&](const std::string& id) {
        FeatureState removal;
        if (const auto layer = currentStates.find(sourceLayer); layer != currentStates.end()) {
            if (const auto feature = layer->second.find(id); feature != layer->second.end()) {
                for (const auto& state : feature->second) {
                    if (!newState.contains(state.first)) {
                        removal[state.first] = {};
                    }
                }
            }
        }
        return removal;
    };
    FeatureStates& featureDeletions = layerDeletions->second;
    if (featureDeletions.empty()) {
        if (const auto layer = currentStates.find(sourceLayer); layer != currentStates.end()) {
            for (const auto& feature : layer->second) {
                if (feature.first != featureID) {
                    featureDeletions[feature.first] = {};
                }
            }
        }
        if (auto removal = removalOfKeysNotInNewState(featureID); !removal.empty()) {
            featureDeletions[featureID] = std::move(removal);
        }
    } else if (const auto feature = featureDeletions.find(featureID); feature != featureDeletions.end()) {
        if (feature->second.empty()) {
            feature->second = removalOfKeysNotInNewState(featureID);
        } else {
            for (const auto& state : newState) {
                feature->second.erase(state.first);
            }
        }
        if (feature->second.empty()) {
            featureDeletions.erase(feature);
        }
    }
    if (featureDeletions.empty()) {
        deletedStates.erase(layerDeletions);
    }
}

void SourceFeatureState::getState(FeatureState& result,
                                  const std::optional<std::string>& sourceLayerID,
                                  const std::string& featureID) const {
    std::string sourceLayer = sourceLayerID.value_or(std::string());
    FeatureState current;
    FeatureState changes;
    auto layerStates = currentStates.find(sourceLayer);
    if (layerStates != currentStates.end()) {
        const auto currentStateEntry = layerStates->second.find(featureID);
        if (currentStateEntry != layerStates->second.end()) {
            current = currentStateEntry->second;
        }
    }

    layerStates = stateChanges.find(sourceLayer);
    if (layerStates != stateChanges.end()) {
        const auto stateChangesEntry = layerStates->second.find(featureID);
        if (stateChangesEntry != layerStates->second.end()) {
            changes = stateChangesEntry->second;
        }
    }
    result = std::move(changes);
    result.insert(current.begin(), current.end());
}

void SourceFeatureState::coalesceChanges(std::vector<RenderTile>& tiles) {
    MLN_TRACE_FUNC();

    LayerFeatureStates changes;
    for (const auto& layerStatesEntry : stateChanges) {
        const auto& sourceLayer = layerStatesEntry.first;
        FeatureStates layerStates;
        for (const auto& featureStatesEntry : stateChanges[sourceLayer]) {
            const auto& featureID = featureStatesEntry.first;
            for (const auto& stateEntry : stateChanges[sourceLayer][featureID]) {
                const auto& stateKey = stateEntry.first;
                const auto& stateVal = stateEntry.second;

                auto currentState = currentStates[sourceLayer][featureID].find(stateKey);
                if (currentState != currentStates[sourceLayer][featureID].end()) {
                    currentState->second = stateVal;
                } else {
                    currentStates[sourceLayer][featureID].insert(std::make_pair(stateKey, stateVal));
                }
            }
            layerStates[featureID] = currentStates[sourceLayer][featureID];
        }
        changes[sourceLayer] = std::move(layerStates);
    }

    for (const auto& layerStatesEntry : deletedStates) {
        const auto& sourceLayer = layerStatesEntry.first;
        FeatureStates& layerStates = changes[sourceLayer];

        if (deletedStates[sourceLayer].empty()) {
            for (const auto& featureStatesEntry : currentStates[sourceLayer]) {
                const auto& featureID = featureStatesEntry.first;
                layerStates[featureID] = {};
                currentStates[sourceLayer][featureID] = {};
            }
        } else {
            for (const auto& feature : deletedStates[sourceLayer]) {
                const auto& featureID = feature.first;
                bool deleteWholeFeatureState = deletedStates[sourceLayer][featureID].empty();
                if (deleteWholeFeatureState) {
                    currentStates[sourceLayer][featureID] = {};
                } else {
                    for (const auto& stateEntry : deletedStates[sourceLayer][featureID]) {
                        currentStates[sourceLayer][featureID].erase(stateEntry.first);
                    }
                }
                layerStates[featureID] = currentStates[sourceLayer][featureID];
            }
        }
    }

    stateChanges.clear();
    deletedStates.clear();

    changes.insert(currentStates.begin(), currentStates.end());

    if (changes.empty()) {
        return;
    }

    for (auto& tile : tiles) {
        tile.setFeatureState(changes);
    }
}

void SourceFeatureState::removeState(const std::optional<std::string>& sourceLayerID,
                                     const std::optional<std::string>& featureID,
                                     const std::optional<std::string>& stateKey) {
    std::string sourceLayer = sourceLayerID.value_or(std::string());

    const auto layerDeletions = deletedStates.find(sourceLayer);
    bool sourceLayerDeleted = layerDeletions != deletedStates.end() && layerDeletions->second.empty();
    if (sourceLayerDeleted) {
        return;
    }

    if (stateKey && featureID) {
        bool featureDeleted = layerDeletions != deletedStates.end() && layerDeletions->second.contains(*featureID) &&
                              layerDeletions->second.at(*featureID).empty();
        if (featureDeleted) {
            return;
        }
        deletedStates[sourceLayer][*featureID][*stateKey] = {};
    } else if (featureID) {
        deletedStates[sourceLayer][*featureID] = {};
    } else {
        deletedStates[sourceLayer] = {};
    }
    // A later change turns this removal into removals of the state in currentStates, which don't cover queued changes.
    cancelQueuedChanges(sourceLayer, featureID, stateKey);
}

void SourceFeatureState::cancelQueuedChanges(const std::string& sourceLayer,
                                             const std::optional<std::string>& featureID,
                                             const std::optional<std::string>& stateKey) {
    const auto layerChanges = stateChanges.find(sourceLayer);
    if (layerChanges == stateChanges.end()) {
        return;
    }
    if (!featureID) {
        stateChanges.erase(layerChanges);
        return;
    }
    const auto featureChanges = layerChanges->second.find(*featureID);
    if (featureChanges == layerChanges->second.end()) {
        return;
    }
    if (stateKey) {
        featureChanges->second.erase(*stateKey);
    }
    if (!stateKey || featureChanges->second.empty()) {
        layerChanges->second.erase(featureChanges);
    }
}

} // namespace mln
