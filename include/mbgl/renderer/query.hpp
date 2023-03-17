#pragma once

#include <mbgl/style/filter.hpp>

#include <string>
#include <vector>
#include <optional>

namespace mbgl {

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
};

} // namespace mbgl
