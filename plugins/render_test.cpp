// The render-test runner of a plugin. A build defines MLN_PLUGIN_HEADER and MLN_PLUGIN_REGISTER for the plugin it
// tests.
//
// With `--plugin-layers` it only runs the tests in the manifest's test directory whose style has a layer of a type that
// the plugin registers, so a new test of the layer needs no change to the plugin. `--ignore <file>` leaves out the
// tests named in a JSON file, as the ignore files of the platforms do: {"render-tests/<test>": "reason"}.
#include <mln/render_test.hpp>

#include MLN_PLUGIN_HEADER

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

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

std::map<std::string, std::string> ignoredTests(const std::string& path) {
    std::map<std::string, std::string> ignored;
    const auto text = readFile(path);
    const std::regex entry("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), entry); it != std::sregex_iterator(); ++it) {
        ignored.emplace((*it)[1].str(), (*it)[2].str());
    }
    return ignored;
}

// A `--filter` regex that matches the style of every test with a layer of one of `layerTypes`, except the ignored ones.
std::string filterForLayerTypes(const fs::path& manifestPath,
                                const std::vector<std::string>& layerTypes,
                                const std::map<std::string, std::string>& ignored) {
    const auto base =
        (manifestPath.parent_path() / manifestValue(readFile(manifestPath), "base_test_path")).lexically_normal();
    std::string alternatives;
    for (size_t i = 0; i < layerTypes.size(); ++i) alternatives += (i ? "|" : "") + layerTypes[i];
    const std::regex layer("\"type\"\\s*:\\s*\"(" + alternatives + ")\"");
    std::vector<std::string> tests;
    for (const auto& entry : fs::recursive_directory_iterator(base)) {
        if (entry.path().filename() != "style.json") continue;
        if (!std::regex_search(readFile(entry.path()), layer)) continue;
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

// Collects the layer types of the plugin instead of registering it.
mln_plugin_status collectLayerTypes(const mln_plugin_descriptor_v1* descriptor, char*, size_t) {
    for (size_t i = 0; i < descriptor->layer_type_count; ++i) {
        const auto& type = descriptor->layer_types[i].layer_type;
        registeredLayerTypes.emplace_back(type.data, type.size);
    }
    return MLN_PLUGIN_STATUS_OK;
}

// Whether `args` has the switch `name`, which is removed.
bool takeSwitch(std::vector<std::string>& args, const std::string& name) {
    const auto found = std::find(args.begin(), args.end(), name);
    if (found == args.end()) return false;
    args.erase(found);
    return true;
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
    const auto ignored = ignoredTests(flag(args, "--ignore", "", true));
    if (takeSwitch(args, "--plugin-layers")) {
        const auto manifest = flag(args, "--manifestPath", "-p", false);
        const auto filter = filterForLayerTypes(manifest, registeredLayerTypes, ignored);
        if (filter.empty()) {
            std::cerr << "No test in the directory of " << manifest << " has a layer of a type of the plugin\n";
            return 1;
        }
        args.push_back("--filter=" + filter);
    }
    std::vector<char*> pointers;
    for (auto& arg : args) pointers.push_back(arg.data());
    pointers.push_back(nullptr);
    return mln::runRenderTests(static_cast<int>(args.size()), pointers.data(), {});
}
