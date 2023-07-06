#pragma once

#include <mbgl/renderer/renderer_observer.hpp>
#include <functional>
#include <string>
#include <vector>

namespace mbgl {

class ImageManagerObserver {
public:
    virtual ~ImageManagerObserver() = default;

    virtual void onStyleImageMissing(const std::string&, const std::function<void()>& done) { done(); }
    virtual void onRemoveUnusedStyleImages(const std::vector<std::string>&) {}
};

} // namespace mbgl
