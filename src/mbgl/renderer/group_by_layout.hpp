#pragma once

#include <mbgl/style/layer_impl.hpp>
#include <mbgl/util/containers.hpp>
#include <mbgl/style/layer_properties.hpp>

#include <vector>

namespace mbgl {

using GroupMap = mbgl::unordered_map<std::string, std::vector<Immutable<style::LayerProperties>>>;

GroupMap groupLayers(const std::vector<Immutable<style::LayerProperties>> &);

} // namespace mbgl
