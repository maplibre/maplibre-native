#pragma once

#ifdef __cplusplus

#if __has_include(<MapLibre/plugin.h>)
#include <MapLibre/plugin.h>
#else
#include "plugin.h"
#endif

#include <string>

namespace mbgl {
namespace plugin {

class __attribute__((visibility("default"))) StylePreprocessor : public Plugin {
public:
    virtual ~StylePreprocessor();
    virtual std::string processStyle(const std::string& data);
};

} // namespace plugin
} // namespace mbgl

#endif
