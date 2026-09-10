#include <ngon_layer.hpp>
#include <mln/layermanager/layer_manager.hpp>
#include <mln/style/conversion/json.hpp>
#include <mln/style/conversion/layer.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/layers/plugin_style_layer.hpp>
#include <mln/util/run_loop.hpp>
#include <gtest/gtest.h>

using namespace mln;

namespace {
const mln_plugin_descriptor_v1* descriptor = nullptr;
mln_plugin_status capture(const mln_plugin_descriptor_v1* value, char*, size_t) {
    descriptor = value;
    return MLN_PLUGIN_STATUS_OK;
}

TEST(PluginRegistry, ValidatesAndCopiesDescriptors) {
    style::conversion::Error parsingError;
    EXPECT_FALSE(style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"unregistered-plugin","source":"points"})", parsingError));
    ASSERT_EQ(MLN_PLUGIN_STATUS_OK, mln_ngon_layer_register(capture, nullptr, 0));
    ASSERT_NE(nullptr, descriptor);
    char error[256]{};
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(nullptr, error, sizeof(error)));
    EXPECT_NE('\0', error[0]);

    auto input = *descriptor;
    input.abi_version = 2;
    EXPECT_EQ(MLN_PLUGIN_STATUS_UNSUPPORTED_ABI, mln_plugin_register_v1(&input, error, sizeof(error)));
    input = *descriptor;
    input.struct_size = 0;
    EXPECT_EQ(MLN_PLUGIN_STATUS_UNSUPPORTED_ABI, mln_plugin_register_v1(&input, error, sizeof(error)));
    input = *descriptor;
    auto layer = input.layer_types[0];
    input.layer_types = &layer;
    layer.create_layout = nullptr;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&input, error, sizeof(error)));
    layer = descriptor->layer_types[0];
    auto property = layer.properties[0];
    property.default_value.type = MLN_PLUGIN_VALUE_STRING;
    layer.properties = &property;
    layer.property_count = 1;
    EXPECT_EQ(MLN_PLUGIN_STATUS_INVALID_ARGUMENT, mln_plugin_register_v1(&input, error, sizeof(error)));

    EXPECT_EQ(MLN_PLUGIN_STATUS_OK, mln_ngon_layer_register(mln_plugin_register_v1, error, sizeof(error))) << error;
    EXPECT_EQ(MLN_PLUGIN_STATUS_ALREADY_REGISTERED,
              mln_ngon_layer_register(mln_plugin_register_v1, error, sizeof(error)))
        << error;
    EXPECT_TRUE(LayerManager::get()->hasLayerType("ngon"));
    input = *descriptor;
    input.plugin_version = {"different", 9};
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&input, error, sizeof(error)));
    input = *descriptor;
    input.plugin_id = {"another-plugin", 14};
    EXPECT_EQ(MLN_PLUGIN_STATUS_CONFLICT, mln_plugin_register_v1(&input, error, sizeof(error)));
}

TEST(PluginStyle, ParsingDefaultsExpressionsAndClone) {
    char diagnostic[256]{};
    mln_ngon_layer_register(mln_plugin_register_v1, diagnostic, sizeof(diagnostic));
    style::conversion::Error error;
    auto parsed = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"points","type":"ngon","source":"points","paint":{"ngon-radius":12,"ngon-opacity":0}})", error);
    ASSERT_TRUE(parsed) << error.message;
    auto& layer = **parsed;
    EXPECT_EQ(12.0, *layer.getProperty("ngon-radius").getValue().getDouble());
    EXPECT_EQ(0.0, *layer.getProperty("ngon-opacity").getValue().getDouble());
    auto cloned = layer.cloneRef("clone");
    EXPECT_EQ(layer.getProperty("ngon-opacity").getValue(), cloned->getProperty("ngon-opacity").getValue());
    const auto serialized = layer.serialize();
    ASSERT_NE(nullptr, serialized.getObject());
    EXPECT_NE(serialized.getObject()->end(), serialized.getObject()->find("paint"));

    auto wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"ngon","source":"points","paint":{"ngon-radius":"wide"}})", error);
    EXPECT_FALSE(wrong);
    wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"circle","source":"points","paint":{"ngon-radius":12}})", error);
    EXPECT_FALSE(wrong); // Plugins cannot add properties to built-in layers.
    wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"ngon","source":"points","layout":{"ngon-radius":12}})", error);
    EXPECT_FALSE(wrong);
    wrong = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(R"({"id":"p","type":"ngon"})", error);
    EXPECT_FALSE(wrong);
    auto dynamic = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"p","type":"ngon","source":"points","paint":{"ngon-radius":["interpolate",["linear"],["zoom"],10,["get","small"],11,["get","large"]]}})",
        error);
    EXPECT_TRUE(dynamic) << error.message;
    auto filtered = style::conversion::convertJSON<std::unique_ptr<style::Layer>>(
        R"({"id":"points","type":"ngon","source":"points","filter":["==","n",3]})", error);
    ASSERT_TRUE(filtered) << error.message;
    EXPECT_TRUE(static_cast<const style::PluginStyleLayer&>(layer).impl().hasLayoutDifference(
        static_cast<const style::PluginStyleLayer&>(**filtered).impl()));
}
} // namespace

int main(int argc, char** argv) {
    util::RunLoop loop;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
