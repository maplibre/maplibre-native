#include <mbgl/util/action_journal_impl.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/map/map.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <regex>

namespace mbgl {
namespace util {

constexpr auto ACTION_JOURNAL_FOLDER_NAME = "action_journal";
constexpr auto ACTION_JOURNAL_FILE_NAME = "action_journal";
constexpr auto ACTION_JOURNAL_FILE_EXTENSION = "log";

class MapEnvironmentSnapshot {
public:
    MapEnvironmentSnapshot(const ActionJournal::Impl& journal) {
        const auto& map = journal.getMap();
        const auto& clientOptions = map.getClientOptions();
        const auto& style = map.getStyle();

        time = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

        clientName = clientOptions.name();
        clientVersion = clientOptions.version();

        styleName = style.getName();
        styleURL = style.getURL();
    }

public:
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> time;
    std::string clientName;
    std::string clientVersion;
    std::string styleName;
    std::string styleURL;
};

class ActionJournalEvent {
public:
    ActionJournalEvent(const rapidjson::GenericStringRef<char>& name, const MapEnvironmentSnapshot& env)
        : json(rapidjson::kObjectType),
          eventJson(rapidjson::kObjectType) {
        json.AddMember("name", name, json.GetAllocator());
        json.AddMember("time", iso8601(env.time), json.GetAllocator());

        if (!env.clientName.empty()) {
            json.AddMember("clientName", env.clientName, json.GetAllocator());
        }

        if (!env.clientVersion.empty()) {
            json.AddMember("clientVersion", env.clientVersion, json.GetAllocator());
        }

        if (!env.styleName.empty()) {
            json.AddMember("styleName", env.styleName, json.GetAllocator());
        }

        if (!env.styleURL.empty()) {
            json.AddMember("styleURL", env.styleURL, json.GetAllocator());
        }
    }

    ~ActionJournalEvent() = default;

    template <typename T>
    ActionJournalEvent& addEvent(const rapidjson::GenericStringRef<char>& key, const T& value) {
        eventJson.AddMember(key, value, json.GetAllocator());
        return *this;
    }

    ActionJournalEvent& addEventStringArray(const rapidjson::GenericStringRef<char>& key,
                                            const std::vector<std::string>& value) {
        rapidjson::Value arrayJson(rapidjson::kArrayType);

        for (const auto& elem : value) {
            rapidjson::Value elemJson(rapidjson::kStringType);
            elemJson.SetString(elem.c_str(), elem.size());
            arrayJson.PushBack(elemJson, json.GetAllocator());
        }

        eventJson.AddMember(key, arrayJson, json.GetAllocator());
        return *this;
    }

    template <typename T>
    ActionJournalEvent& addEventEnum(const rapidjson::GenericStringRef<char>& key, const T& value) {
        eventJson.AddMember(key, rapidjson::StringRef(Enum<T>::toString(value)), json.GetAllocator());
        return *this;
    }

    std::string toString() {
        if (!eventJson.ObjectEmpty()) {
            json.AddMember("event", eventJson, json.GetAllocator());
        }

        rapidjson::StringBuffer jsonBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> jsonWriter(jsonBuffer);

        json.Accept(jsonWriter);

        return jsonBuffer.GetString();
    }

    rapidjson::Document json;
    rapidjson::Value eventJson;
};

ActionJournal::Impl::Impl(const Map& map_, const ActionJournalOptions& options_)
    : map(map_),
      options(options_),
      scheduler(Scheduler::GetSequenced()) {
    assert(!options.path().empty());
    assert(options.logFileSize() > 0);
    assert(options.logFileCount() > 1);

    options.withPath(options.path() + "/" + ACTION_JOURNAL_FOLDER_NAME);

    openFile(detectFiles(), false);
}

std::vector<std::string> ActionJournal::Impl::getLog() {
    std::lock_guard lock(fileMutex);
    std::vector<std::string> logEvents;

    const auto readFile = [&](std::fstream& file) {
        if (!file) {
            return;
        }

        for (std::string line; std::getline(file, line);) {
            logEvents.emplace_back(line);
        }
    };

    // read current file
    if (currentFile && currentFileIndex == 0) {
        currentFile.seekg(std::ios::beg);
        readFile(currentFile);
        return logEvents;
    }

    const uint32_t maxFileIndex = currentFile ? currentFileIndex - 1 : detectFiles();

    // read previous files
    for (uint32_t fileIndex = 0; fileIndex <= maxFileIndex; ++fileIndex) {
        const auto& filepath = getFilepath(fileIndex);
        std::fstream file(filepath, std::fstream::in);
        readFile(file);
    }

    if (currentFile) {
        currentFile.seekg(std::ios::beg);
        readFile(currentFile);
    }

    return logEvents;
}

void ActionJournal::Impl::clearLog() {
    std::lock_guard lock(fileMutex);

    // close current file
    currentFile.close();

    currentFileIndex = 0;
    currentFileSize = 0;

    if (!std::filesystem::remove_all(options.path())) {
        Log::Error(Event::General, "Failed to clear ActionJournal");
    }

    openFile(0, false);
}

void ActionJournal::Impl::onCameraWillChange(CameraChangeMode mode) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onCameraWillChange", env).addEvent("cameraMode", static_cast<int>(mode)));
    });
}

void ActionJournal::Impl::onCameraIsChanging() {}

void ActionJournal::Impl::onCameraDidChange(CameraChangeMode mode) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onCameraDidChange", env).addEvent("cameraMode", static_cast<int>(mode)));
    });
}

void ActionJournal::Impl::onWillStartLoadingMap() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onWillStartLoadingMap", env)); });
}

void ActionJournal::Impl::onDidFinishLoadingMap() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onDidFinishLoadingMap", env)); });
}

void ActionJournal::Impl::onDidFailLoadingMap(MapLoadError error, const std::string& errorStr) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onDidFailLoadingMap", env)
                .addEvent("error", errorStr)
                .addEvent("code", static_cast<int>(error)));
    });
}

void ActionJournal::Impl::onWillStartRenderingFrame() {}

void ActionJournal::Impl::onDidFinishRenderingFrame(RenderFrameStatus) {}

void ActionJournal::Impl::onWillStartRenderingMap() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onWillStartRenderingMap", env)); });
}

void ActionJournal::Impl::onDidFinishRenderingMap(RenderMode) {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onDidFinishRenderingMap", env)); });
}

void ActionJournal::Impl::onDidFinishLoadingStyle() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onDidFinishLoadingStyle", env)); });
}

void ActionJournal::Impl::onSourceChanged(style::Source& source) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this), type = source.getType()]() {
        log(ActionJournalEvent("onSourceChanged", env).addEventEnum("type", type));
    });
}

void ActionJournal::Impl::onDidBecomeIdle() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onDidBecomeIdle", env)); });
}

void ActionJournal::Impl::onStyleImageMissing(const std::string& id) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onStyleImageMissing", env).addEvent("id", id));
    });
}

void ActionJournal::Impl::onRegisterShaders(gfx::ShaderRegistry&) {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onRegisterShaders", env)); });
}

void ActionJournal::Impl::onPreCompileShader(shaders::BuiltIn id, gfx::Backend::Type backend, const std::string&) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onPreCompileShader", env)
                .addEventEnum("shader", id)
                .addEvent("backend", static_cast<int>(backend)));
    });
}

void ActionJournal::Impl::onPostCompileShader(shaders::BuiltIn id, gfx::Backend::Type backend, const std::string&) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onPostCompileShader", env)
                .addEventEnum("shader", id)
                .addEvent("backend", static_cast<int>(backend)));
    });
}

void ActionJournal::Impl::onShaderCompileFailed(shaders::BuiltIn id,
                                                gfx::Backend::Type backend,
                                                const std::string& defines) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onShaderCompileFailed", env)
                .addEventEnum("shader", id)
                .addEvent("backend", static_cast<int>(backend))
                .addEvent("defines", defines));
    });
}

void ActionJournal::Impl::onGlyphsLoaded(const FontStack& fonts, const GlyphRange& range) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onGlyphsLoaded", env)
                .addEventStringArray("fonts", fonts)
                .addEvent("rangeStart", range.first)
                .addEvent("rangeEnd", range.second));
    });
}

void ActionJournal::Impl::onGlyphsError(const FontStack& fonts, const GlyphRange& range, std::exception_ptr error) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        ActionJournalEvent event("onGlyphsError", env);

        event.addEventStringArray("fonts", fonts);
        event.addEvent("rangeStart", range.first);
        event.addEvent("rangeEnd", range.second);

        if (error) {
            try {
                std::rethrow_exception(error);
            } catch (const std::exception& e) {
                event.addEvent("error", std::string(e.what()));
            }
        }

        log(event);
    });
}

void ActionJournal::Impl::onGlyphsRequested(const FontStack& fonts, const GlyphRange& range) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onGlyphsRequested", env)
                .addEventStringArray("fonts", fonts)
                .addEvent("rangeStart", range.first)
                .addEvent("rangeEnd", range.second));
    });
}

void ActionJournal::Impl::onTileAction(TileOperation op, const OverscaledTileID& id, const std::string& sourceID) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        log(ActionJournalEvent("onTileAction", env)
                .addEventEnum("action", op)
                .addEvent("tileX", id.canonical.x)
                .addEvent("tileY", id.canonical.y)
                .addEvent("tileZ", id.canonical.z)
                .addEvent("overscaledZ", id.overscaledZ)
                .addEvent("sourceID", sourceID));
    });
}

void ActionJournal::Impl::onSpriteLoaded(const std::optional<style::Sprite>& sprite) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        ActionJournalEvent event("onSpriteLoaded", *this);

        if (sprite) {
            event.addEvent("id", sprite.value().id);
            event.addEvent("url", sprite.value().spriteURL);
        }

        log(event);
    });
}

void ActionJournal::Impl::onSpriteError(const std::optional<style::Sprite>& sprite, std::exception_ptr error) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        ActionJournalEvent event("onSpriteError", env);

        if (sprite) {
            event.addEvent("id", sprite.value().id);
            event.addEvent("url", sprite.value().spriteURL);
        }

        if (error) {
            try {
                std::rethrow_exception(error);
            } catch (const std::exception& e) {
                event.addEvent("error", std::string(e.what()));
            }
        }

        log(event);
    });
}

void ActionJournal::Impl::onSpriteRequested(const std::optional<style::Sprite>& sprite) {
    scheduler->schedule([=, this, env = MapEnvironmentSnapshot(*this)]() {
        ActionJournalEvent event("onSpriteRequested", env);

        if (sprite) {
            event.addEvent("id", sprite.value().id);
            event.addEvent("url", sprite.value().spriteURL);
        }

        log(event);
    });
}

void ActionJournal::Impl::onMapCreate() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onMapCreate", env)); });
}

void ActionJournal::Impl::onMapDestroy() {
    scheduler->schedule(
        [=, this, env = MapEnvironmentSnapshot(*this)]() { log(ActionJournalEvent("onMapDestroy", env)); });
}

void ActionJournal::Impl::log(ActionJournalEvent& value) {
    logToFile(value.toString());
}

void ActionJournal::Impl::log(ActionJournalEvent&& value) {
    logToFile(value.toString());
}

std::string ActionJournal::Impl::getFilepath(uint32_t fileIndex) {
    return options.path() + "/" + ACTION_JOURNAL_FILE_NAME + "." + std::to_string(fileIndex) + "." +
           ACTION_JOURNAL_FILE_EXTENSION;
}

uint32_t ActionJournal::Impl::detectFiles() {
    if (!std::filesystem::exists(options.path())) {
        return 0;
    }

    std::map<uint32_t, std::filesystem::path> existingFiles;

    const std::regex fileRegex(std::string(R"(.*\.([0-9]+)\.)") + ACTION_JOURNAL_FILE_EXTENSION);
    std::smatch fileMatch;

    for (const auto& entry : std::filesystem::directory_iterator(options.path())) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::string& path = entry.path().string();
        if (std::regex_match(path, fileMatch, fileRegex)) {
            if (fileMatch.size() == 2) {
                existingFiles.emplace(std::stoi(fileMatch[1].str()), path);
            }
        }
    }

    if (existingFiles.empty()) {
        return 0;
    }

    // removing extra files (old files or due to the file count changing)
    for (auto it = existingFiles.begin(); existingFiles.size() > options.logFileCount();) {
        std::filesystem::remove(it->second);
        it = existingFiles.erase(it);
    }

    uint32_t expectedIndex = 0;

    // validate file index
    for (const auto& file : existingFiles) {
        if (file.first != expectedIndex) {
            std::filesystem::rename(file.second, getFilepath(expectedIndex));
        }

        ++expectedIndex;
    }

    return expectedIndex - 1;
}

uint32_t ActionJournal::Impl::rollFiles() {
    // delete the oldest file
    const auto& oldestFilepath = getFilepath(0);
    if (std::filesystem::exists(oldestFilepath)) {
        std::filesystem::remove(oldestFilepath);
    }

    // rename the rest
    uint32_t expectedIndex = 0;
    for (uint32_t index = 1; index < options.logFileCount(); ++index) {
        const auto& filepath = getFilepath(index);
        if (std::filesystem::exists(filepath)) {
            std::filesystem::rename(filepath, getFilepath(expectedIndex++));
        }
    }

    return expectedIndex;
}

bool ActionJournal::Impl::openFile(uint32_t fileIndex, bool truncate) {
    assert(fileIndex < options.logFileCount());

    if (!std::filesystem::exists(options.path())) {
        std::filesystem::create_directories(options.path());
    }

    const auto& filepath = getFilepath(fileIndex);
    const std::ios::openmode openMode = (truncate ? std::ios::trunc : std::ios::ate | std::ios::app) |
                                        std::fstream::out | std::fstream::in;

    currentFile.open(filepath, openMode);

    if (currentFile.is_open()) {
        currentFileIndex = fileIndex;
        currentFileSize = std::filesystem::file_size(filepath);
        return true;
    }

    return false;
}

bool ActionJournal::Impl::prepareFile(size_t size) {
    if (currentFileSize + size <= options.logFileSize() && currentFile) {
        currentFileSize += size;
        return true;
    }

    // else roll file
    currentFile.close();

    if (currentFileIndex + 1 >= options.logFileCount()) {
        currentFileIndex = rollFiles();
    } else {
        ++currentFileIndex;
    }

    return openFile(currentFileIndex, true);
}

void ActionJournal::Impl::logToFile(const std::string& value) {
    if (value.empty()) {
        return;
    }

    std::lock_guard lock(fileMutex);

    if (!prepareFile(value.size() + 1)) {
        return;
    }

    currentFile << value << "\n";
    currentFile.flush();
}

} // namespace util
} // namespace mbgl
