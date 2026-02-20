#pragma once

#include <functional>

namespace mbgl {

struct TestStatus {
    size_t completed;
    size_t total;
};

int runRenderTests(int argc, char* argv[], std::function<void(TestStatus)>);

} // namespace mbgl
