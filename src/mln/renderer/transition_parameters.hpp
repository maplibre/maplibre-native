#pragma once

#include <mln/util/chrono.hpp>
#include <mln/style/transition_options.hpp>

#include <vector>

namespace mln {

class TransitionParameters {
public:
    TimePoint now;
    style::TransitionOptions transition;
};

} // namespace mln
