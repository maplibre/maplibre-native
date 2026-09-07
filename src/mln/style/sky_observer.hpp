#pragma once

namespace mln {
namespace style {

class Sky;

class SkyObserver {
public:
    virtual ~SkyObserver() = default;

    virtual void onSkyChanged(const Sky&) {}
};

} // namespace style
} // namespace mln
