#include <mbgl/render_test.hpp>

#ifdef __APPLE
#if defined(TARGET_IPHONE_SIMULATOR)
#define IS_IOS
#elif defined(TARGET_OS_IPHONE)
#define IS_IOS
#endif
#endif

int main(int argc, char* argv[]) {
#ifdef IS_IOS
    try {
#endif
        return mbgl::runRenderTests(argc, argv, []() {});
#ifdef IS_IOS
    } catch (std::exception const& e) {
        std::cerr << "Caught an exception while running tests\n" << e.what() << '\n';
        return 1;
    }
#endif
}