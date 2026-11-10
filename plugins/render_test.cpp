// The render-test runner of a plugin. A build defines MLN_PLUGIN_HEADER and MLN_PLUGIN_REGISTER for the plugin it
// tests, and `--plugin-directory <directory>` names the plugin in plugins/plugins. The tests are
//
//   - the plugin's own, when it has render-tests/manifest.json, or else
//   - the tests of the manifest of the repository for the renderer of this build whose style has a layer of a built-in
//     type that the plugin implements, run with the plugin's layer type in place of the built-in one, so that they
//     check that the plugin renders like the built-in layer. A layer type org.maplibre.<type> implements the built-in
//     layer type <type>. A new test of the layer needs no change to the plugin. render-tests/ignores.json leaves out
//     tests, as the ignore files of the platforms do: {"render-tests/<test>": "reason"}.
//
// The runner does nothing when the `backends` of the plugin.json do not have the renderer of this build.
#include <mln/render_test.hpp>

#include MLN_PLUGIN_HEADER

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace {

namespace fs = std::filesystem;

#if MLN_RENDER_BACKEND_METAL
constexpr auto backend = "metal";
constexpr auto repositoryManifest = "metrics/ios-metal-render-test-runner-style.json";
#elif MLN_RENDER_BACKEND_VULKAN
constexpr auto backend = "vulkan";
constexpr auto repositoryManifest = "metrics/linux-vulkan.json";
#else
constexpr auto backend = "opengl";
constexpr auto repositoryManifest = "metrics/linux-opengl.json";
#endif

std::string readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

std::string escape(const std::string& text) {
    std::string result;
    for (const char c : text) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '/') result += '\\';
        result += c;
    }
    return result;
}

// The value of `--name value`, `--name=value` or `-p value` in `args`; the flag is removed from `args` when `remove`.
std::string flag(std::vector<std::string>& args, const std::string& name, const std::string& shortName, bool remove) {
    for (size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];
        if (arg == name || (!shortName.empty() && arg == shortName)) {
            if (i + 1 == args.size()) return {};
            auto value = args[i + 1];
            if (remove) args.erase(args.begin() + i, args.begin() + i + 2);
            return value;
        }
        if (arg.rfind(name + "=", 0) == 0) {
            auto value = arg.substr(name.size() + 1);
            if (remove) args.erase(args.begin() + i);
            return value;
        }
    }
    return {};
}

std::string manifestValue(const std::string& manifest, const std::string& key) {
    std::smatch match;
    std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
    return std::regex_search(manifest, match, pattern) ? match[1].str() : std::string{};
}

std::map<std::string, std::string> ignoredTests(const fs::path& path) {
    std::map<std::string, std::string> ignored;
    const auto text = readFile(path);
    const std::regex entry("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), entry); it != std::sregex_iterator(); ++it) {
        ignored.emplace((*it)[1].str(), (*it)[2].str());
    }
    return ignored;
}

// Gives every layer in `value` with a type that is a key of `layerTypes` the type it maps to, and returns how many it
// changed. An object with an `id` and a `type` is a layer: those of the style, and those that the operations of a test
// add or set with a style.
size_t replaceLayerTypes(rapidjson::Value& value,
                         const std::map<std::string, std::string>& layerTypes,
                         rapidjson::Document::AllocatorType& allocator) {
    size_t replaced = 0;
    if (value.IsArray()) {
        for (auto& element : value.GetArray()) replaced += replaceLayerTypes(element, layerTypes, allocator);
    } else if (value.IsObject()) {
        const auto type = value.FindMember("type");
        if (value.HasMember("id") && type != value.MemberEnd() && type->value.IsString()) {
            const auto replacement = layerTypes.find(type->value.GetString());
            if (replacement != layerTypes.end()) {
                type->value.SetString(replacement->second.c_str(), allocator);
                ++replaced;
            }
        }
        for (auto& member : value.GetObject()) replaced += replaceLayerTypes(member.value, layerTypes, allocator);
    }
    return replaced;
}

std::string replaceLayerTypes(const std::string& style, const std::map<std::string, std::string>& layerTypes) {
    rapidjson::Document document;
    document.Parse(style.c_str());
    replaceLayerTypes(document, layerTypes, document.GetAllocator());
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return buffer.GetString();
}

// A `--filter` regex that matches the style of every test with a layer of one of the keys of `layerTypes`, except the
// ignored ones. The layers are found as replaceLayerTypes finds them, so that every test it matches runs the plugin.
std::string filterForLayerTypes(const fs::path& manifestPath,
                                const std::map<std::string, std::string>& layerTypes,
                                const std::map<std::string, std::string>& ignored) {
    const auto base =
        (manifestPath.parent_path() / manifestValue(readFile(manifestPath), "base_test_path")).lexically_normal();
    std::vector<std::string> tests;
    for (const auto& entry : fs::recursive_directory_iterator(base)) {
        if (entry.path().filename() != "style.json") continue;
        rapidjson::Document style;
        style.Parse(readFile(entry.path()).c_str());
        if (style.HasParseError() || !replaceLayerTypes(style, layerTypes, style.GetAllocator())) continue;
        const auto test = entry.path().parent_path().lexically_relative(base).generic_string();
        const auto skip = ignored.find(test);
        if (skip != ignored.end()) {
            std::cout << "* ignore " << test << " (" << skip->second << ")\n";
            continue;
        }
        tests.push_back(escape(test));
    }
    if (tests.empty()) return {};
    std::string filter = "/(";
    for (size_t i = 0; i < tests.size(); ++i) filter += (i ? "|" : "") + tests[i];
    return filter + ")/style\\.json$";
}

std::vector<std::string> registeredLayerTypes;

// The built-in layer types that the plugin implements, each with the layer type of the plugin that implements it.
std::map<std::string, std::string> implementedLayerTypes() {
    constexpr std::string_view prefix = "org.maplibre.";
    std::map<std::string, std::string> types;
    for (const auto& type : registeredLayerTypes) {
        if (type.starts_with(prefix)) types.emplace(type.substr(prefix.size()), type);
    }
    return types;
}

// Collects the layer types of the plugin instead of registering it.
mln_plugin_status collectLayerTypes(const mln_plugin_descriptor_v1* descriptor, char*, size_t) {
    for (size_t i = 0; i < descriptor->layer_type_count; ++i) {
        const auto& type = descriptor->layer_types[i].layer_type;
        registeredLayerTypes.emplace_back(type.data, type.size);
    }
    return MLN_PLUGIN_STATUS_OK;
}

// Whether the plugin.json in `directory` has no `backends`, or the backend of this build among them.
bool hasBackend(const fs::path& directory) {
    const auto json = readFile(directory / "plugin.json");
    std::smatch backends;
    if (!std::regex_search(json, backends, std::regex("\"backends\"\\s*:\\s*\\[([^\\]]*)\\]"))) return true;
    return backends[1].str().find(std::string("\"") + backend + "\"") != std::string::npos;
}

} // namespace

int main(int argc, char** argv) {
    char error[512]{};
    MLN_PLUGIN_REGISTER(collectLayerTypes, error, sizeof(error));
    const auto status = MLN_PLUGIN_REGISTER(mln_plugin_register_v1, error, sizeof(error));
    if (status != MLN_PLUGIN_STATUS_OK && status != MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
        std::cerr << "Plugin registration failed: " << error << '\n';
        return 1;
    }

    std::vector<std::string> args(argv, argv + argc);
    mln::TestStyleTransform transformStyle;
    const fs::path directory = flag(args, "--plugin-directory", "", true);
    if (!directory.empty()) {
        if (!hasBackend(directory)) {
            std::cout << "* skipped: " << directory << " has no " << backend << " backend\n";
            return 0;
        }
        if (fs::exists(directory / "render-tests/manifest.json")) {
            args.insert(args.end(), {"--manifestPath", (directory / "render-tests/manifest.json").string()});
        } else {
            const auto layerTypes = implementedLayerTypes();
            const auto filter = filterForLayerTypes(
                repositoryManifest, layerTypes, ignoredTests(directory / "render-tests/ignores.json"));
            if (filter.empty()) {
                std::cerr << "No test of " << repositoryManifest
                          << " has a layer of a built-in type that the plugin implements\n";
                return 1;
            }
            args.insert(args.end(), {"--manifestPath", repositoryManifest, "--filter=" + filter});
            transformStyle = [layerTypes](const std::string& style) {
                return replaceLayerTypes(style, layerTypes);
            };
        }
    }
    std::vector<char*> pointers;
    for (auto& arg : args) pointers.push_back(arg.data());
    pointers.push_back(nullptr);
    return mln::runRenderTests(static_cast<int>(args.size()), pointers.data(), {}, transformStyle);
}
