#pragma once

#include <mbgl/util/action_journal.hpp>
#include <mbgl/map/map_observer.hpp>

#include <fstream>
#include <filesystem>

namespace mbgl {

class Map;
class Scheduler;

namespace util {

class ActionJournalEvent;

class ActionJournal::Impl : public MapObserver {
public:
    Impl(const Map& map, const uint32_t logFileSize = 1024 * 1024, const uint32_t logFileCount = 5);
    ~Impl() = default;

    const Map& getMap() const { return map; }

    std::vector<std::string> getLog();
    void clearLog();

    // MapObserver
    void onCameraWillChange(CameraChangeMode) override;
    void onCameraIsChanging() override;
    void onCameraDidChange(CameraChangeMode) override;
    void onWillStartLoadingMap() override;
    void onDidFinishLoadingMap() override;
    void onDidFailLoadingMap(MapLoadError, const std::string&) override;
    void onWillStartRenderingFrame() override;
    void onDidFinishRenderingFrame(RenderFrameStatus) override;
    void onWillStartRenderingMap() override;
    void onDidFinishRenderingMap(RenderMode) override;
    void onDidFinishLoadingStyle() override;
    void onSourceChanged(style::Source&) override;
    void onDidBecomeIdle() override;
    void onStyleImageMissing(const std::string&) override;
    void onRegisterShaders(gfx::ShaderRegistry&) override;
    void onPreCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) override;
    void onPostCompileShader(shaders::BuiltIn, gfx::Backend::Type, const std::string&) override;
    void onShaderCompileFailed(shaders::BuiltIn, gfx::Backend::Type, const std::string&) override;
    void onGlyphsLoaded(const FontStack&, const GlyphRange&) override;
    void onGlyphsError(const FontStack&, const GlyphRange&, std::exception_ptr) override;
    void onGlyphsRequested(const FontStack&, const GlyphRange&) override;
    void onTileAction(TileOperation, const OverscaledTileID&, const std::string&) override;
    void onSpriteLoaded(const std::optional<style::Sprite>&) override;
    void onSpriteError(const std::optional<style::Sprite>&, std::exception_ptr) override;
    void onSpriteRequested(const std::optional<style::Sprite>&) override;

    void onMapCreate();
    void onMapDestroy();

protected:

    void log(ActionJournalEvent& value);
    void log(ActionJournalEvent&& value);

    // file operations
    std::filesystem::path getFilepath(uint32_t fileIndex);
    uint32_t detectFiles();
    uint32_t rollFiles();

    bool openFile(uint32_t fileIndex, bool truncate = false);
    bool prepareFile(size_t size);
    void logToFile(const std::string& value);

protected:

    friend class ActionJournalEvent;
    friend class Map;

    const Map& map;

    const uint32_t logFileSize;
    const uint32_t logFileCount;

    const std::shared_ptr<Scheduler> scheduler;

    std::mutex fileMutex;
    std::fstream currentFile;
    uint32_t currentFileIndex{0};
    size_t currentFileSize{0};
};

} // namespace util
} // namespace mbgl
