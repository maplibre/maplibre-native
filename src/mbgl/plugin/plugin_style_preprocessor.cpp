//
//  StyleFilterPlugin.cpp
//  App
//
//  Created by Malcolm Toon on 11/26/25.
//

#include "plugin_style_preprocessor.hpp"

using namespace mbgl::plugin;


StylePreprocessor::~StylePreprocessor() {

}

std::string StylePreprocessor::processStyle(const std::string & data) {
    // Base class just returns whatever was passed to it
    return data;
}
