#pragma once

#include <mbgl/tile/geometry_tile_data.hpp>

#include <unordered_map>
#include <string>
#include <vector>

namespace mbgl {

class SymbolFeature;

namespace util {

void mergeLines(std::vector<SymbolFeature> &features);

} // end namespace util
} // end namespace mbgl
