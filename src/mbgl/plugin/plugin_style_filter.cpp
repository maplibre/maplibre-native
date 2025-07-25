#include <mbgl/plugin/plugin_style_filter.hpp>

using namespace mbgl::style;

// This method
const mbgl::Response PluginStyleFilter::FilterResponse(const Response& res) {
    if (_filterStyleFunction) {
        auto tempResult = _filterStyleFunction(res);
        return tempResult;
    }
    return res;
}
