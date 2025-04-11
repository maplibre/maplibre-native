#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/style/style.hpp>

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
    ActionJournalEvent(const rapidjson::GenericStringRef<char>& name, const ActionJournal& journal)
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
    void addEvent(const rapidjson::GenericStringRef<char>& key, const T& value) {
        eventJson.AddMember(key, value, json.GetAllocator());
    }

    void addEventStringArray(const rapidjson::GenericStringRef<char>& key, const std::vector<std::string>& value) {
        rapidjson::Value arrayJson(rapidjson::kArrayType);

        for (const auto& elem : value) {
            rapidjson::Value elemJson(rapidjson::kStringType);
            elemJson.SetString(elem.c_str(), elem.size());
            arrayJson.PushBack(elemJson, json.GetAllocator());
        }

        eventJson.AddMember(key, arrayJson, json.GetAllocator());
    }

    template <typename T>
    void addEventEnum(const rapidjson::GenericStringRef<char>& key, const T& value) {
        eventJson.AddMember(key, rapidjson::StringRef(Enum<T>::toString(value)), json.GetAllocator());
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

ActionJournal::ActionJournal(const Map& map_,
                             const uint32_t logFileSize_,
                             const uint32_t logFileCount_)
    : map(map_),
      logFileSize(logFileSize_),
      logFileCount(logFileCount_) {

    assert(logFileSize > 0);
    assert(logFileCount > 1);

    const auto fileIndex = detectFiles();
    openFile(fileIndex, false);
}

ActionJournal::~ActionJournal() {

}

std::vector<std::string> ActionJournal::getLog() {
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

void ActionJournal::clearLog() {
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

void ActionJournal::onCameraWillChange(CameraChangeMode mode) {
    ActionJournalEvent event("onCameraWillChange", *this);

    event.addEvent("cameraMode", static_cast<int>(mode));

    log(event);
}

void ActionJournal::onCameraIsChanging() {

}

void ActionJournal::onCameraDidChange(CameraChangeMode mode) {
    ActionJournalEvent event("onCameraDidChange", *this);

    event.addEvent("cameraMode", static_cast<int>(mode));

    log(event);
}

void ActionJournal::onWillStartLoadingMap() {
    ActionJournalEvent event("onWillStartLoadingMap", *this);

    log(event);
}

void ActionJournal::onDidFinishLoadingMap() {
    ActionJournalEvent event("onDidFinishLoadingMap", *this);

    log(event);
}

void ActionJournal::onDidFailLoadingMap(MapLoadError error, const std::string& errorStr) {
    ActionJournalEvent event("onDidFailLoadingMap", *this);

    event.addEvent("error", errorStr);
    event.addEvent("code", static_cast<int>(error));

    log(event);
}

void ActionJournal::onWillStartRenderingFrame() {

}

void ActionJournal::onDidFinishRenderingFrame(RenderFrameStatus frame) {
    
    //ActionJournalEvent event("frameFinished", *this);
    //
    //event.addEvent("frameEncodingTime", frame.frameEncodingTime);
    //event.addEvent("frameRenderingTime", frame.frameRenderingTime);
    //event.addEvent("placementChanged", frame.placementChanged);
    //event.addEvent("needsRepaint", frame.needsRepaint);

    //log(event);
}

void ActionJournal::onWillStartRenderingMap() {
    ActionJournalEvent event("onWillStartRenderingMap", *this);

    log(event);
}

void ActionJournal::onDidFinishRenderingMap(RenderMode) {
    ActionJournalEvent event("onDidFinishRenderingMap", *this);

    log(event);
}

void ActionJournal::onDidFinishLoadingStyle() {
    ActionJournalEvent event("onDidFinishLoadingStyle", *this);

    log(event);
}

void ActionJournal::onSourceChanged(style::Source& source) {
    ActionJournalEvent event("onSourceChanged", *this);
    
    event.addEventEnum("type", source.getType());

    log(event);
}

void ActionJournal::onDidBecomeIdle() {
    ActionJournalEvent event("onDidBecomeIdle", *this);

    log(event);
}

void ActionJournal::onStyleImageMissing(const std::string& id) {
    ActionJournalEvent event("onStyleImageMissing", *this);

     event.addEvent("id", id);

    log(event);
}

void ActionJournal::onRegisterShaders(gfx::ShaderRegistry& shaders) {
    ActionJournalEvent event("onRegisterShaders", *this);

    log(event);
}

void ActionJournal::onPreCompileShader(shaders::BuiltIn id, gfx::Backend::Type backend, const std::string& defines) {
    ActionJournalEvent event("onPreCompileShader", *this);

    event.addEvent("shader", static_cast<int>(id));
    event.addEvent("backend", static_cast<int>(backend));

    log(event);
}

void ActionJournal::onPostCompileShader(shaders::BuiltIn id, gfx::Backend::Type backend, const std::string& defines) {
    ActionJournalEvent event("onPostCompileShader", *this);

    // TODO add `shader_source.cpp` with `BuiltIn` enum string to `generate_shader_code.mjs`?
    event.addEvent("shader", static_cast<int>(id));
    event.addEvent("backend", static_cast<int>(backend));

    log(event);
}

void ActionJournal::onShaderCompileFailed(shaders::BuiltIn id, gfx::Backend::Type backend, const std::string& defines) {
    ActionJournalEvent event("onShaderCompileFailed", *this);

    event.addEvent("shader", static_cast<int>(id));
    event.addEvent("backend", static_cast<int>(backend));
    
    log(event);
}

void ActionJournal::onGlyphsLoaded(const FontStack& fonts, const GlyphRange& range) {
    ActionJournalEvent event("onGlyphsLoaded", *this);
    
    event.addEventStringArray("fonts", fonts);
    event.addEvent("rangeStart", range.first);
    event.addEvent("rangeEnd", range.second);

    log(event);
}

void ActionJournal::onGlyphsError(const FontStack& fonts, const GlyphRange& range, std::exception_ptr error) {
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
}

void ActionJournal::onGlyphsRequested(const FontStack& fonts, const GlyphRange& range) {
    ActionJournalEvent event("onGlyphsRequested", *this);

    event.addEventStringArray("fonts", fonts);
    event.addEvent("rangeStart", range.first);
    event.addEvent("rangeEnd", range.second);

    log(event);
}

void ActionJournal::onTileAction(TileOperation op, const OverscaledTileID& id, const std::string& sourceID) {
    ActionJournalEvent event("onTileAction", *this);

    event.addEventEnum("action", op);
    event.addEvent("tileX", id.canonical.x);
    event.addEvent("tileY", id.canonical.y);
    event.addEvent("tileZ", id.canonical.z);
    event.addEvent("overscaledZ", id.overscaledZ);
    event.addEvent("sourceID", sourceID);

    log(event);
}

void ActionJournal::onSpriteLoaded(const std::optional<style::Sprite>& sprite) {
    ActionJournalEvent event("onSpriteLoaded", *this);

    if (sprite) {
        event.addEvent("id", sprite.value().id);
        event.addEvent("url", sprite.value().spriteURL);
    }

    log(event);
}

void ActionJournal::onSpriteError(const std::optional<style::Sprite>& sprite, std::exception_ptr error) {
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
}

void ActionJournal::onSpriteRequested(const std::optional<style::Sprite>& sprite) {
    ActionJournalEvent event("onSpriteRequested", *this);

    if (sprite) {
        event.addEvent("id", sprite.value().id);
        event.addEvent("url", sprite.value().spriteURL);
    }

    log(event);
}

void ActionJournal::onMapCreate() {
    ActionJournalEvent event("onMapCreate", *this);

    log(event);
}

void ActionJournal::onMapDestroy() {
    ActionJournalEvent event("onMapDestroy", *this);

    log(event);
}

void ActionJournal::log(ActionJournalEvent& value) {
    logToFile(value.toString());
}

std::filesystem::path ActionJournal::getFilepath(uint32_t fileIndex) {
    return std::filesystem::path(ACTION_JOURNAL_PATH) /
           std::format("{}.{}.{}", ACTION_JOURNAL_FILENAME, fileIndex, ACTION_JOURNAL_FILE_EXTENSION);
}

uint32_t ActionJournal::detectFiles() {
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

uint32_t ActionJournal::rollFiles() {
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

void ActionJournal::openFile(uint32_t fileIndex, bool truncate) {
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
    } else {
        Log::Error(Event::General, std::string("Failed to open ActionJournal file: ") + filepath.string());
    }
}

void ActionJournal::prepareFile(size_t size) {
    if (currentFileSize + size <= logFileSize) {
        currentFileSize += size;
        return;
    }

    // else roll file
    currentFile.close();

    if (currentFileIndex + 1 >= logFileCount) {
        currentFileIndex = rollFiles();
    } else {
        ++currentFileIndex;
    }

    openFile(currentFileIndex, true);
}

void ActionJournal::logToFile(const std::string& value) {

    const auto start = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::steady_clock::now().time_since_epoch()).count();

    std::lock_guard lock(fileMutex);

    prepareFile(value.size() + 1);

    currentFile << value << "\n";
    currentFile.flush();

   const auto end = std::chrono::duration_cast<std::chrono::duration<double>>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();

    Log::Error(Event::General, "ActionJournal::logToFile " + std::to_string(end - start));
}

} // namespace util
} // namespace mbgl