#pragma once

#include <functional>
#include <string>

namespace mln {

struct TestStatus {
    size_t completed;
    size_t total;
};

/// Rewrites the style of a test, as JSON, before the test reads its metadata and operations from it.
using TestStyleTransform = std::function<std::string(const std::string& style)>;

int runRenderTests(int argc, char* argv[], std::function<void(TestStatus)>, TestStyleTransform = {});

} // namespace mln
