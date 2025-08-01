#pragma once

#include <mbgl/storage/response.hpp>
#include <functional>

namespace mbgl {

namespace style {

class PluginStyleFilter {
public:
    using OnFilterStyle = std::function<const std::string(const std::string&)>;

    OnFilterStyle _filterStyleFunction;

    // This method
    const std::string filterResponse(const std::string& styleData);
};

} // namespace style
} // namespace mbgl
