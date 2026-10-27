// The render-test runner of a plugin. A build defines MLN_PLUGIN_HEADER and MLN_PLUGIN_REGISTER for the plugin it
// tests.
//
// With `--layer-type <type>` it only runs the tests in the manifest's test directory whose style has a layer of that
// type, so a new test of the layer needs no change to the plugin. `--ignore <file>` leaves out the tests named in a
// JSON file, as the ignore files of the platforms do: {"render-tests/<test>": "reason"}.
#include <mln/render_test.hpp>

#include MLN_PLUGIN_HEADER

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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

std::vector<std::pair<std::string, std::string>> ignoredTests(const std::string& path) {
    std::vector<std::pair<std::string, std::string>> ignored;
    if (path.empty()) return ignored;
    const auto text = readFile(path);
    const std::regex entry("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");
    for (auto it = std::sregex_iterator(text.begin(), text.end(), entry); it != std::sregex_iterator(); ++it) {
        ignored.emplace_back((*it)[1].str(), (*it)[2].str());
    }
    return ignored;
}

// A `--filter` regex that matches the style of every test with a layer of `layerType`, except the ignored ones.
std::string filterForLayerType(const fs::path& manifestPath,
                               const std::string& layerType,
                               const std::vector<std::pair<std::string, std::string>>& ignored) {
    const auto base =
        (manifestPath.parent_path() / manifestValue(readFile(manifestPath), "base_test_path")).lexically_normal();
    const std::regex layer("\"type\"\\s*:\\s*\"" + layerType + "\"");
    std::vector<std::string> tests;
    for (const auto& entry : fs::recursive_directory_iterator(base)) {
        if (entry.path().filename() != "style.json") continue;
        if (!std::regex_search(readFile(entry.path()), layer)) continue;
        const auto test = entry.path().parent_path().lexically_relative(base).generic_string();
        const auto skip = std::find_if(ignored.begin(), ignored.end(), [&](const auto& i) { return i.first == test; });
        if (skip != ignored.end()) {
            std::cout << "* ignore " << test << " (" << skip->second << ")\n";
            continue;
        }
        tests.push_back(escape(test));
    }
    if (tests.empty()) return {};
    std::sort(tests.begin(), tests.end());
    std::string filter = "/(";
    for (const auto& test : tests) filter += (filter.size() > 2 ? "|" : "") + test;
    return filter + ")/style\\.json$";
}

} // namespace

int main(int argc, char** argv) {
    char error[512]{};
    const auto status = MLN_PLUGIN_REGISTER(mln_plugin_register_v1, error, sizeof(error));
    if (status != MLN_PLUGIN_STATUS_OK && status != MLN_PLUGIN_STATUS_ALREADY_REGISTERED) {
        std::cerr << "Plugin registration failed: " << error << '\n';
        return 1;
    }

    std::vector<std::string> args(argv, argv + argc);
    const auto layerType = flag(args, "--layer-type", "", true);
    if (!layerType.empty()) {
        const auto manifest = flag(args, "--manifestPath", "-p", false);
        const auto filter = filterForLayerType(manifest, layerType, ignoredTests(flag(args, "--ignore", "", true)));
        if (filter.empty()) {
            std::cerr << "No test in the directory of " << manifest << " has a layer of type " << layerType << '\n';
            return 1;
        }
        args.push_back("--filter=" + filter);
    }
    std::vector<char*> pointers;
    for (auto& arg : args) pointers.push_back(arg.data());
    pointers.push_back(nullptr);
    return mln::runRenderTests(static_cast<int>(args.size()), pointers.data(), {});
}
