#include <mln/render_test.hpp>

#include <iostream>

// The plugin a test binary registers is chosen by its build target, since the manifest it runs only has styles for
// that plugin.
#if MLN_RENDER_TEST_GLTF
#include <gltf_layer.hpp>
#define MLN_REGISTER_PLUGIN mln_gltf_layer_register
#elif MLN_RENDER_TEST_FILL_EXTRUSION
#include <fill_extrusion.hpp>
#define MLN_REGISTER_PLUGIN mln_fill_extrusion_register
#else
#include <ngon_layer.hpp>
#define MLN_REGISTER_PLUGIN mln_ngon_layer_register
#endif

int main(int argc, char** argv) {
    char error[512]{};
    const auto status = MLN_REGISTER_PLUGIN(mln_plugin_register_v1, error, sizeof(error));
    if (status != MLN_PLUGIN_STATUS_OK && status != MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
        std::cerr << "Plugin registration failed: " << error << '\n';
        return 1;
    }
    return mln::runRenderTests(argc, argv, {});
}
