#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/style/layer_properties.hpp>

#include <memory>
#include <vector>

namespace mbgl {

struct LayoutGroupKey {
    // Direct comparable fields (fast path)
    const style::LayerTypeInfo* typeInfo;
    std::string source;
    std::string sourceLayer;
    float minZoom;
    float maxZoom;
    style::VisibilityType visibility;

    // Filter - use shared_ptr for identity comparison
    std::shared_ptr<const style::expression::Expression> filterExpression;

    // Reference to impl for layout comparison
    const style::Layer::Impl* impl;

    // Comparison operator
    bool operator==(const LayoutGroupKey& other) const;
};

using GroupMap = mbgl::unordered_map<LayoutGroupKey, std::vector<Immutable<style::LayerProperties>>>;

GroupMap groupLayers(const std::vector<Immutable<style::LayerProperties>>&);

} // namespace mbgl

template <>
struct std::hash<mbgl::LayoutGroupKey> {
    std::size_t operator()(const mbgl::LayoutGroupKey& key) const noexcept;
};
