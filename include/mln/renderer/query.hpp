#pragma once

#include <mln/style/filter.hpp>
#include <mln/util/feature.hpp>

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace mln {

/**
 * Options for query rendered features.
 */
class RenderedQueryOptions {
public:
    RenderedQueryOptions(std::optional<std::vector<std::string>> layerIDs_ = std::nullopt,
                         std::optional<style::Filter> filter_ = std::nullopt)
        : layerIDs(std::move(layerIDs_)),
          filter(std::move(filter_)) {}

    /** layerIDs to include in the query */
    std::optional<std::vector<std::string>> layerIDs;

    std::optional<style::Filter> filter;

    /// The style's current global state, injected by the renderer so that
    /// `global-state` expressions in the filter can be evaluated.
    std::shared_ptr<const GlobalStateMap> globalState;
};

/**
 * Options for query source features
 */
class SourceQueryOptions {
public:
    SourceQueryOptions(std::optional<std::vector<std::string>> sourceLayers_ = std::nullopt,
                       std::optional<style::Filter> filter_ = std::nullopt)
        : sourceLayers(std::move(sourceLayers_)),
          filter(std::move(filter_)) {}

    /// Required for VectorSource, ignored for GeoJSONSource
    std::optional<std::vector<std::string>> sourceLayers;

    std::optional<style::Filter> filter;

    /// The style's current global state, injected by the renderer so that
    /// `global-state` expressions in the filter can be evaluated.
    std::shared_ptr<const GlobalStateMap> globalState;
};

} // namespace mln
