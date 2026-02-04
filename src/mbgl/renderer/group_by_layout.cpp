#include <mbgl/renderer/group_by_layout.hpp>
#include <mbgl/util/hash.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/style/filter.hpp>

#include <mbgl/style/conversion/stringify.hpp>
#include <mbgl/util/rapidjson.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace mbgl {

bool LayoutGroupKey::operator==(const LayoutGroupKey& other) const {
    // Fast path: compare pointers and scalars first
    if (typeInfo != other.typeInfo) return false;
    if (minZoom != other.minZoom) return false;
    if (maxZoom != other.maxZoom) return false;
    if (visibility != other.visibility) return false;

    // Compare source strings
    if (source != other.source) return false;
    if (sourceLayer != other.sourceLayer) return false;

    // Filter comparison - prefer pointer equality
    if (filterExpression != other.filterExpression) {
        if (!filterExpression || !other.filterExpression) return false;
        // Deep comparison if pointers differ
        if (*filterExpression != *other.filterExpression) return false;
    }

    return !impl->hasLayoutDifference(*other.impl);
}

namespace {

LayoutGroupKey createGroupKey(const style::Layer::Impl& impl) {
    LayoutGroupKey key;
    key.typeInfo = impl.getTypeInfo();
    key.source = impl.source;
    key.sourceLayer = impl.sourceLayer;
    key.minZoom = impl.minZoom;
    key.maxZoom = impl.maxZoom;
    key.visibility = impl.visibility;
    key.filterExpression = impl.filter.expression.value_or(nullptr);
    key.impl = &impl;
    return key;
}

} // anonymous namespace

GroupMap groupLayers(const std::vector<Immutable<style::LayerProperties>>& layers) {
    MLN_TRACE_FUNC();

    GroupMap groupMap;
    groupMap.reserve(layers.size());

    for (auto layer : layers) {
        groupMap[createGroupKey(*layer->baseImpl)].push_back(std::move(layer));
    }
    return groupMap;
}

} // namespace mbgl

// std::hash specialization (outside namespace)
std::size_t std::hash<mbgl::LayoutGroupKey>::operator()(const mbgl::LayoutGroupKey& key) const noexcept {
    using mbgl::util::hash_combine;

    std::size_t seed = 0;

    // Hash stable pointers and scalars only (NOT layout)
    hash_combine(seed, reinterpret_cast<std::size_t>(key.typeInfo));
    hash_combine(seed, std::hash<std::string>{}(key.source));
    hash_combine(seed, std::hash<std::string>{}(key.sourceLayer));
    hash_combine(seed, std::hash<float>{}(key.minZoom));
    hash_combine(seed, std::hash<float>{}(key.maxZoom));
    hash_combine(seed, static_cast<bool>(key.visibility));
    hash_combine(seed, reinterpret_cast<std::size_t>(key.filterExpression.get()));

    // Note: We intentionally DON'T hash the layout here
    // This means keys with same source/zoom/type/filter but different layouts
    // will hash to the same bucket, then operator== distinguishes them

    return seed;
}
