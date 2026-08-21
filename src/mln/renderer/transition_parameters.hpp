#pragma once

#include <mbgl/util/chrono.hpp>
#include <mbgl/style/transition_options.hpp>

#include <vector>

namespace mln {

class TransitionParameters {
public:
    TimePoint now;
    style::TransitionOptions transition;
};

} // namespace mln
