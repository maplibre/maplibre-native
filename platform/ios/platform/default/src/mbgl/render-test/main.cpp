#include <mbgl/render_test.hpp>

int main(int argc, char *argv[])
try {
    return mbgl::runRenderTests(argc, argv);
} catch (std::exception const& e) {
    std::cerr << "Caught an exception while running tests\n" << e.what() << '\n';
    return 1;
}
