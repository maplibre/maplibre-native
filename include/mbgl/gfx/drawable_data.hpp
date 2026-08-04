#pragma once

#include <memory>

namespace mbgl {

namespace gfx {

class PluginDrawableData;

class DrawableData {
public:
    virtual ~DrawableData() = default;
    virtual PluginDrawableData* getPluginData() { return nullptr; }
    virtual const PluginDrawableData* getPluginData() const { return nullptr; }
};

using UniqueDrawableData = std::unique_ptr<DrawableData>;

} // namespace gfx
} // namespace mbgl
