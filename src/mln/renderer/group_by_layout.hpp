#pragma once

#include <mln/style/layer_impl.hpp>
#include <mln/util/containers.hpp>
#include <mln/style/layer_properties.hpp>

#include <memory>
#include <vector>

namespace mln {

struct LayoutGroupKey {
    // Direct comparable fields (fast path)
    const style::LayerTypeInfo* typeInfo;
    std::string source;
    std::string sourceLayer;
    float minZoom;
    float maxZoom;
    style::VisibilityType visibility;

    // Filter: the expression (compared by address first, then by contents) and the hash of its
    // contents, so layers with equal filters parsed separately still meet in one hash slot.
    std::shared_ptr<const style::expression::Expression> filterExpression;
    std::size_t filterHash;

    // Reference to impl for layout comparison
    const style::Layer::Impl* impl;

    // Comparison operator
    bool operator==(const LayoutGroupKey& other) const;
};

using GroupMap = mln::unordered_map<LayoutGroupKey, std::vector<Immutable<style::LayerProperties>>>;

GroupMap groupLayers(const std::vector<Immutable<style::LayerProperties>>&);

} // namespace mln

template <>
struct std::hash<mln::LayoutGroupKey> {
    std::size_t operator()(const mln::LayoutGroupKey& key) const noexcept;
};
