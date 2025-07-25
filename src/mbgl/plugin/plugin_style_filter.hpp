#pragma once

#include <mbgl/storage/response.hpp>

namespace mbgl {

namespace style {

class PluginStyleFilter {
public:
    using OnFilterStyle = std::function<const Response(const Response&)>;

    OnFilterStyle _filterStyleFunction;

    // This method
    const Response FilterResponse(const Response& res);
};

} // namespace style
} // namespace mbgl
