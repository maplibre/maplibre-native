#include "manifest_parser.hpp"
#include "filesystem.hpp"
#include "parser.hpp"

#include <mln/util/logging.hpp>

#include <algorithm>
#include <random>

#if defined(WIN32) && defined(GetObject)
#undef GetObject
#endif

Manifest::Manifest() {
    const char* envApiKey = getenv("MLN_API_KEY");
    if (envApiKey != nullptr) {
        apiKey = envApiKey;
    }
}

Manifest::~Manifest() = default;

const std::vector<TestPaths>& Manifest::getTestPaths() const {
    return testPaths;
}
const std::vector<std::pair<std::string, std::string>>& Manifest::getIgnores() const {
    return ignores;
}
const std::string& Manifest::getTestRootPath() const {
    return testRootPath;
}
const std::string& Manifest::getManifestPath() const {
    return manifestPath;
}
const std::string& Manifest::getResultPath() const {
    return resultPath;
}

const std::string& Manifest::getCachePath() const {
    return cachePath;
}

const std::string& Manifest::getApiKey() const {
    return apiKey;
}

const std::set<std::string>& Manifest::getProbes() const {
    return probes;
}

void Manifest::doShuffle(uint32_t seed) {
    std::seed_seq sequence{seed};
    std::mt19937 shuffler(sequence);
    std::shuffle(testPaths.begin(), testPaths.end(), shuffler);
}

namespace {
std::vector<std::pair<std::string, std::string>> parseIgnores(const std::vector<mln::filesystem::path>& ignoresPaths) {
    std::vector<std::pair<std::string, std::string>> ignores;
    for (const auto& path : ignoresPaths) {
        auto maybeIgnores = readJson(path);
        if (!maybeIgnores.is<mln::JSDocument>()) {
            continue;
        }
        for (const auto& property : maybeIgnores.get<mln::JSDocument>().GetObject()) {
            const std::string ignore = {property.name.GetString(), property.name.GetStringLength()};
            const std::string reason = {property.value.GetString(), property.value.GetStringLength()};
            ignores.emplace_back(std::make_pair(ignore, reason));
        }
    }

    return ignores;
}

// defaultExpectationPath: absolute path that contains the style.json file for testing
// testId: Test case id that used for composing expectation path
// expectatedPaths: absolute paths that contain possible expected.png/metrics.json files for result checking
std::vector<mln::filesystem::path> getTestExpectations(const mln::filesystem::path& defaultExpectationPath,
                                                       const std::string& testId,
                                                       std::vector<mln::filesystem::path> expectatedPaths) {
    std::vector<mln::filesystem::path> expectations{defaultExpectationPath};
    for (const auto& expectedPath : expectatedPaths) {
        expectations.emplace_back(expectedPath / testId);
    }
    return expectations;
}

mln::filesystem::path getValidPath(const std::string& manifestPath, const std::string& path) {
    const static mln::filesystem::path BasePath{manifestPath};
    mln::filesystem::path result{path};
    if (result.is_relative()) {
        result = BasePath / result;
    }
    if (mln::filesystem::exists(result)) {
        return result.lexically_normal();
    }
    mln::Log::Warning(mln::Event::General, "Invalid path is provided inside the manifest file: " + path);
    return mln::filesystem::path{};
}

} // namespace

std::optional<Manifest> ManifestParser::parseManifest(const std::string& manifestPath, std::string testFilter) {
    Manifest manifest;
    const auto filePath = mln::filesystem::path(manifestPath);
    manifest.manifestPath = manifestPath.substr(0, manifestPath.find(filePath.filename().generic_string()));

    auto contents = readJson(filePath);
    if (!contents.is<mln::JSDocument>()) {
        mln::Log::Error(mln::Event::General,
                        "Provided manifest file: " + filePath.generic_string() + " is not a valid json");
        return std::nullopt;
    }

    auto document = std::move(contents.get<mln::JSDocument>());
    if (document.HasMember("result_path")) {
        const auto& resultPathValue = document["result_path"];
        if (!resultPathValue.IsString()) {
            mln::Log::Warning(mln::Event::General,
                              "Invalid result_path is provided inside the manifest file: " + filePath.generic_string());
            return std::nullopt;
        }
        manifest.resultPath = (getValidPath(manifest.manifestPath, resultPathValue.GetString()) / "").generic_string();
        if (manifest.resultPath.empty()) {
            return std::nullopt;
        }
    }
    if (document.HasMember("cache_path")) {
        const auto& cachePathValue = document["cache_path"];
        if (!cachePathValue.IsString()) {
            mln::Log::Warning(mln::Event::General,
                              "Invalid cache_path is provided inside the manifest file: " + filePath.generic_string());
            return std::nullopt;
        }
        manifest.cachePath = (getValidPath(manifest.manifestPath, ".") / cachePathValue.GetString()).generic_string();
        if (manifest.cachePath.empty()) {
            return std::nullopt;
        }
    }
    // TODO:PP
    if (document.HasMember("access_token")) {
        const auto& apiKeyValue = document["access_token"];
        if (!apiKeyValue.IsString()) {
            mln::Log::Warning(
                mln::Event::General,
                "Invalid access_token is provided inside the manifest file: " + filePath.generic_string());
            return std::nullopt;
        }
        manifest.apiKey = apiKeyValue.GetString();
        if (manifest.apiKey.empty()) {
            return std::nullopt;
        }
    }
    mln::filesystem::path baseTestPath;
    if (document.HasMember("base_test_path")) {
        const auto& testPathValue = document["base_test_path"];
        if (!testPathValue.IsString()) {
            mln::Log::Warning(mln::Event::General,
                              "Invalid base_test_path is provided inside the manifest "
                              "file: " +
                                  filePath.generic_string());
            return std::nullopt;
        }
        baseTestPath = getValidPath(manifest.manifestPath, testPathValue.GetString());
        if (baseTestPath.empty()) {
            return std::nullopt;
        }
    }
    mln::filesystem::path expectedMetricPath;
    if (document.HasMember("metric_path")) {
        const auto& metricPathValue = document["metric_path"];
        if (!metricPathValue.IsString()) {
            mln::Log::Warning(mln::Event::General,
                              "Invalid metric_path is provided inside the manifest file: " + filePath.generic_string());
            return std::nullopt;
        }
        expectedMetricPath = getValidPath(manifest.manifestPath, metricPathValue.GetString());
        if (expectedMetricPath.empty()) {
            return std::nullopt;
        }
    }
    std::vector<mln::filesystem::path> expectationPaths{};
    if (document.HasMember("expectation_paths")) {
        const auto& expectationPathValue = document["expectation_paths"];
        if (!expectationPathValue.IsArray()) {
            mln::Log::Warning(mln::Event::General,
                              "Provided expectation_paths inside the manifest file: %s is "
                              "not a valid array" +
                                  filePath.generic_string());
            return std::nullopt;
        }
        for (const auto& value : expectationPathValue.GetArray()) {
            if (!value.IsString()) {
                mln::Log::Warning(mln::Event::General,
                                  "Invalid expectation path item is provided inside the "
                                  "manifest file: " +
                                      filePath.generic_string());
                return std::nullopt;
            }
            expectationPaths.emplace_back(getValidPath(manifest.manifestPath, value.GetString()));
            if (expectationPaths.back().empty()) {
                return std::nullopt;
            }
        }
    }
    std::vector<mln::filesystem::path> ignorePaths{};
    if (document.HasMember("ignore_paths")) {
        const auto& ignorePathValue = document["ignore_paths"];
        if (!ignorePathValue.IsArray()) {
            mln::Log::Warning(mln::Event::General,
                              "Provided ignore_paths inside the manifest file: " + filePath.generic_string() +
                                  " is not a valid array");
            return std::nullopt;
        }
        for (const auto& value : ignorePathValue.GetArray()) {
            if (!value.IsString()) {
                mln::Log::Warning(mln::Event::General,
                                  "Invalid ignore path item is provided inside the manifest "
                                  "file: " +
                                      filePath.generic_string());
                return std::nullopt;
            }
            ignorePaths.emplace_back(getValidPath(manifest.manifestPath, value.GetString()));
            if (ignorePaths.back().empty()) {
                return std::nullopt;
            }
        }
        manifest.ignores = parseIgnores(ignorePaths);
    }

    if (document.HasMember("probes")) {
        const auto& probesValue = document["probes"];
        if (!probesValue.IsArray()) {
            mln::Log::Warning(
                mln::Event::General,
                "Provided probes inside the manifest file: " + filePath.generic_string() + "is not a valid array");
            return std::nullopt;
        }
        for (const auto& value : probesValue.GetArray()) {
            if (!value.IsString()) {
                mln::Log::Warning(mln::Event::General,
                                  "Invalid probe type is provided inside the manifest "
                                  "file: " +
                                      filePath.generic_string());
                return std::nullopt;
            }
            manifest.probes.emplace(value.GetString());
        }
    }

    if (testFilter.empty() && document.HasMember("filter")) {
        const auto& filterValue = document["filter"];
        if (!filterValue.IsString()) {
            mln::Log::Warning(mln::Event::General,
                              "Invalid filter is provided inside the manifest file: " + filePath.generic_string());
            return std::nullopt;
        }

        testFilter = filterValue.GetString();
    }

    manifest.testRootPath = baseTestPath.string();
    if (manifest.testRootPath.back() == '/') {
        manifest.testRootPath.pop_back();
    }
    if (manifest.manifestPath.back() == '/') {
        manifest.manifestPath.pop_back();
    }
    if (manifest.resultPath.empty()) {
        manifest.resultPath = manifest.manifestPath;
    } else if (manifest.resultPath.back() == '/') {
        manifest.resultPath.pop_back();
    }

    auto& path = manifest.testRootPath;
    auto& testPaths = manifest.testPaths;

    for (auto& testPath : mln::filesystem::recursive_directory_iterator(path)) {
        // Skip paths that fail regexp search.
        if (!testFilter.empty() && !std::regex_search(testPath.path().generic_string(), std::regex(testFilter))) {
            continue;
        }

        if (testPath.path().filename() == "style.json") {
            const auto defaultExpectationPath{std::move(mln::filesystem::path(testPath).remove_filename())};
            const auto rootLength = manifest.testRootPath.length();
            auto testId = defaultExpectationPath.generic_string();
            testId = testId.substr(rootLength + 1, testId.length() - rootLength - 1);

            std::vector<mln::filesystem::path> expectedMetricPaths{expectedMetricPath};
#if defined(__ANDROID__)
            // todo: use `Context.getExternalFilesDir()` or similar via JNI to select an appropriate destination
            const auto locations = std::vector<std::string>{
                "/sdcard",
                "/storage/emulated/0",
                "/storage/self/primary",
            };
            static bool reportedOnce = false;
            for (const auto& location : locations) {
                // Checking `mln::filesystem::status` doesn't accurately reflect whether we can create subdirectories,
                // so just try it. (See `TestRunner::checkProbingResults`)
                try {
                    const auto baselinesPath = location + "/baselines";
                    mln::filesystem::create_directories(baselinesPath);
                    expectedMetricPaths.emplace_back(baselinesPath);
                    break;
                } catch (mln::filesystem::filesystem_error& ex) {
                    if (!reportedOnce) {
                        mln::Log::Warning(mln::Event::Android, "Not a writable directory: " + std::string(ex.what()));
                    }
                }
            }
            // Only log on the first case
            reportedOnce = true;
#elif defined(__APPLE__)
            expectedMetricPaths.emplace_back(manifest.manifestPath + "/baselines/");
#endif
            testPaths.emplace_back(testPath,
                                   getTestExpectations(defaultExpectationPath, testId, expectationPaths),
                                   getTestExpectations(defaultExpectationPath, testId, expectedMetricPaths));
        }
    }

    return std::optional<Manifest>(manifest);
}
