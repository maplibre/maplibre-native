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

constexpr auto ACTION_JOURNAL_PATH = "/tmp/action_journal/";
constexpr auto ACTION_JOURNAL_FILENAME = "action_journal";
constexpr auto ACTION_JOURNAL_FILE_EXTENSION = "log";

class ActionJournalEvent {
public:
    ActionJournalEvent(const rapidjson::GenericStringRef<char>& name, const ActionJournal::Impl& journal)
        : json(rapidjson::kObjectType),
          eventJson(rapidjson::kObjectType) {
        json.AddMember("name", name, json.GetAllocator());
        json.AddMember("time", std::format("{:%FT%TZ}", std::chrono::system_clock::now()), json.GetAllocator());

        const auto& clientOptions = journal.getMap().getClientOptions();
        const auto& clientName = clientOptions.name();
        const auto& clientVersion = clientOptions.version();

        if (!clientName.empty()) {
            json.AddMember("clientName", clientName, json.GetAllocator()); 
        }

        if (!clientVersion.empty()) {
            json.AddMember("clientVersion", clientVersion, json.GetAllocator());
        }

        const auto& style = journal.getMap().getStyle();
        const auto& styleName = style.getName();
        const auto& styleURL = style.getURL();
            
        if (!styleName.empty()) {
            json.AddMember("styleName", styleName, json.GetAllocator());
        }

        if (!styleURL.empty()) {
            json.AddMember("styleURL", styleURL, json.GetAllocator());
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

ActionJournal::Impl::Impl(const Map& map_,
                             const uint32_t logFileSize_,
                             const uint32_t logFileCount_)
    : map(map_),
      logFileSize(logFileSize_),
      logFileCount(logFileCount_),
      scheduler(Scheduler::GetSequenced()) {

    assert(logFileSize > 0);
    assert(logFileCount > 1);

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

    if (!std::filesystem::remove_all(ACTION_JOURNAL_PATH)) {
        Log::Error(Event::General, "Failed to clear ActionJournal");
    }

    openFile(0, false);
}

void ActionJournal::Impl::onCameraWillChange(CameraChangeMode mode) {
    scheduler->schedule(
        [=]() { log(ActionJournalEvent("onCameraWillChange", *this).addEvent("cameraMode", static_cast<int>(mode))); });
}

void ActionJournal::Impl::onCameraIsChanging() {

}

void ActionJournal::Impl::onCameraDidChange(CameraChangeMode mode) {
    scheduler->schedule(
        [=]() { log(ActionJournalEvent("onCameraDidChange", *this).addEvent("cameraMode", static_cast<int>(mode))); });
}

void ActionJournal::Impl::onWillStartLoadingMap() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onWillStartLoadingMap", *this)); });
}

void ActionJournal::Impl::onDidFinishLoadingMap() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onDidFinishLoadingMap", *this)); });
}

void ActionJournal::Impl::onDidFailLoadingMap(MapLoadError error, const std::string& errorStr) {
    scheduler->schedule([=, errorStr = errorStr]() {
        log(ActionJournalEvent("onDidFailLoadingMap", *this)
                .addEvent("error", errorStr)
                .addEvent("code", static_cast<int>(error)));
    });
}

void ActionJournal::Impl::onWillStartRenderingFrame() {

}

void ActionJournal::Impl::onDidFinishRenderingFrame(RenderFrameStatus frame) {

}

void ActionJournal::Impl::onWillStartRenderingMap() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onWillStartRenderingMap", *this)); });
}

void ActionJournal::Impl::onDidFinishRenderingMap(RenderMode) {
    scheduler->schedule([=]() { log(ActionJournalEvent("onDidFinishRenderingMap", *this)); });
}

void ActionJournal::Impl::onDidFinishLoadingStyle() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onDidFinishLoadingStyle", *this)); });
}

void ActionJournal::Impl::onSourceChanged(style::Source& source) {
    scheduler->schedule([=, type = source.getType()]() {
        log(ActionJournalEvent("onSourceChanged", *this).addEventEnum("type", type));
    });
}

void ActionJournal::Impl::onDidBecomeIdle() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onDidBecomeIdle", *this)); });
}

void ActionJournal::Impl::onStyleImageMissing(const std::string& id) {
    scheduler->schedule([=, id = id]() { log(ActionJournalEvent("onDidBecomeIdle", *this).addEvent("id", id)); });
}

void ActionJournal::Impl::onRegisterShaders(gfx::ShaderRegistry& shaders) {
    scheduler->schedule([=]() { log(ActionJournalEvent("onRegisterShaders", *this)); });
}

void ActionJournal::Impl::onPreCompileShader(shaders::BuiltIn id,
                                             gfx::Backend::Type backend,
                                             const std::string& defines) {
    scheduler->schedule([=]() {
        log(ActionJournalEvent("onPreCompileShader", *this)
                .addEvent("shader", static_cast<int>(id))
                .addEvent("backend", static_cast<int>(backend)));
    });
}

void ActionJournal::Impl::onPostCompileShader(shaders::BuiltIn id,
                                              gfx::Backend::Type backend,
                                              const std::string& defines) {

    // TODO add `shader_source.cpp` with `BuiltIn` enum string to `generate_shader_code.mjs`?
    scheduler->schedule([=]() {
        log(ActionJournalEvent("onPostCompileShader", *this)
                .addEvent("shader", static_cast<int>(id))
                .addEvent("backend", static_cast<int>(backend)));
    });
}

void ActionJournal::Impl::onShaderCompileFailed(shaders::BuiltIn id,
                                                gfx::Backend::Type backend,
                                                const std::string& defines) {
    scheduler->schedule([=]() {
        log(ActionJournalEvent("onShaderCompileFailed", *this)
                .addEvent("shader", static_cast<int>(id))
                .addEvent("backend", static_cast<int>(backend)));
    });
}

void ActionJournal::Impl::onGlyphsLoaded(const FontStack& fonts, const GlyphRange& range) {
    scheduler->schedule([=, fonts = fonts, range = range]() {
        log(ActionJournalEvent("onGlyphsLoaded", *this)
                .addEventStringArray("fonts", fonts)
                .addEvent("rangeStart", range.first)
                .addEvent("rangeEnd", range.second));
    });
}

void ActionJournal::Impl::onGlyphsError(const FontStack& fonts, const GlyphRange& range, std::exception_ptr error) {
    scheduler->schedule([=, fonts = fonts, range = range, error = error]() {
        ActionJournalEvent event("onGlyphsError", *this);

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
    scheduler->schedule([=, fonts = fonts, range = range]() {
        log(ActionJournalEvent("onGlyphsRequested", *this)
                .addEventStringArray("fonts", fonts)
                .addEvent("rangeStart", range.first)
                .addEvent("rangeEnd", range.second));
    });
}

void ActionJournal::Impl::onTileAction(TileOperation op, const OverscaledTileID& id, const std::string& sourceID) {
    scheduler->schedule([=]() {
        log(ActionJournalEvent("onTileAction", *this)
                .addEventEnum("action", op)
                .addEvent("tileX", id.canonical.x)
                .addEvent("tileY", id.canonical.y)
                .addEvent("tileZ", id.canonical.z)
                .addEvent("overscaledZ", id.overscaledZ)
                .addEvent("sourceID", sourceID));
    });
}

void ActionJournal::Impl::onSpriteLoaded(const std::optional<style::Sprite>& sprite) {
    scheduler->schedule([=]() {
        ActionJournalEvent event("onSpriteLoaded", *this);

        if (sprite) {
            event.addEvent("id", sprite.value().id);
            event.addEvent("url", sprite.value().spriteURL);
        }

        log(event);
    });
}

void ActionJournal::Impl::onSpriteError(const std::optional<style::Sprite>& sprite, std::exception_ptr error) {
    scheduler->schedule([=]() {
        ActionJournalEvent event("onSpriteError", *this);

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
    scheduler->schedule([=]() {
        ActionJournalEvent event("onSpriteRequested", *this);

        if (sprite) {
            event.addEvent("id", sprite.value().id);
            event.addEvent("url", sprite.value().spriteURL);
        }

        log(event);
    });
}

void ActionJournal::Impl::onMapCreate() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onMapCreate", *this)); });
}

void ActionJournal::Impl::onMapDestroy() {
    scheduler->schedule([=]() { log(ActionJournalEvent("onMapDestroy", *this)); });
}

void ActionJournal::Impl::log(ActionJournalEvent& value) {
    logToFile(value.toString());
}

void ActionJournal::Impl::log(ActionJournalEvent&& value) {
    logToFile(value.toString());
}

std::filesystem::path ActionJournal::Impl::getFilepath(uint32_t fileIndex) {
    return std::filesystem::path(ACTION_JOURNAL_PATH) /
           std::format("{}.{}.{}", ACTION_JOURNAL_FILENAME, fileIndex, ACTION_JOURNAL_FILE_EXTENSION);
}

uint32_t ActionJournal::Impl::detectFiles() {
    if (!std::filesystem::exists(ACTION_JOURNAL_PATH)) {
        return 0;
    }

    std::map<uint32_t, std::filesystem::path> existingFiles;

    const std::regex fileRegex(std::string(R"(.*\.([0-9]+)\.)") + ACTION_JOURNAL_FILE_EXTENSION);
    std::smatch fileMatch;

    for (const auto& entry : std::filesystem::directory_iterator(ACTION_JOURNAL_PATH)) {
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
    for (auto it = existingFiles.begin(); existingFiles.size() > logFileCount;) {
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
    for (uint32_t index = 1; index < logFileCount; ++index) {
        const auto& filepath = getFilepath(index);
        if (std::filesystem::exists(filepath)) {
            std::filesystem::rename(filepath, getFilepath(expectedIndex++));
        }
    }

    return expectedIndex;
}

bool ActionJournal::Impl::openFile(uint32_t fileIndex, bool truncate) {
    assert(fileIndex < logFileCount);

    if (!std::filesystem::exists(ACTION_JOURNAL_PATH)) {
        std::filesystem::create_directories(ACTION_JOURNAL_PATH);
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
    if (currentFileSize + size <= logFileSize && currentFile) {
        currentFileSize += size;
        return true;
    }

    // else roll file
    currentFile.close();

    if (currentFileIndex + 1 >= logFileCount) {
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