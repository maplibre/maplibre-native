#include <mbgl/plugin/plugin_style_filter.hpp>

using namespace mbgl::style;

// This method will call the lambda if it exists
const std::string PluginStyleFilter::filterResponse(const std::string& res) {
    if (_filterStyleFunction) {
        auto tempResult = _filterStyleFunction(res);
        return tempResult;
    }
    return res;
}
