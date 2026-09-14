#include <circle_layer.hpp>
#include <mln/render_test.hpp>
#include <iostream>
#include <string_view>

int main(int argc, char** argv) {
    if (argc > 1 && std::string_view(argv[1]) == "--builtin") {
        --argc;
        ++argv;
    } else {
        char error[512]{};
        if (mln_circle_layer_register(mln_plugin_register_v1, error, sizeof(error)) != MLN_PLUGIN_STATUS_OK) {
            std::cerr << error << '\n';
            return 1;
        }
    }
    return mln::runRenderTests(argc, argv, {});
}
