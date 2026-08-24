#pragma once

#include <functional>

namespace mln {

struct TestStatus {
    size_t completed;
    size_t total;
};

int runRenderTests(int argc, char* argv[], std::function<void(TestStatus)>);

} // namespace mln
