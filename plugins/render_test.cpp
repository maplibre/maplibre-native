#include <mln/render_test.hpp>
#include <ngon_layer.hpp>

#include <iostream>

int main(int argc, char** argv) {
    char error[512]{};
    const auto status = mln_ngon_layer_register(mln_plugin_register_v1, error, sizeof(error));
    if (status != MLN_PLUGIN_STATUS_OK && status != MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
        std::cerr << "Plugin registration failed: " << error << '\n';
        return 1;
    }
    return mln::runRenderTests(argc, argv, {});
}
