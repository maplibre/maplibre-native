#pragma once

#include <mbgl/util/action_journal.hpp>
#include <mbgl/util/action_journal_options.hpp>
#include <mbgl/map/map_observer.hpp>

#include <fstream>

namespace mbgl {

class Map;
class Scheduler;

namespace util {

class ActionJournalEvent;

class ActionJournal::Impl : public MapObserver {
public:
    Impl(const Map& map, const ActionJournalOptions& options);
    ~Impl() override;

    const Map& getMap() const { return map; }
    std::string getLogDirectory() const;
    std::vector<std::string> getLogFiles() const;
    std::vector<std::string> getLog();
    void clearLog();
    void flush();

    static std::string getDirectoryName();

    // Logged events

    // MapObserver
    void onCameraWillChange(CameraChangeMode) override;
    void onCameraDidChange(CameraChangeMode) override;
    void onWillStartLoadingMap() override;
    void onDidFinishLoadingMap() override;
    void onDidFailLoadingMap(MapLoadError, const std::string&) override;
    void onDidFinishRenderingFrame(const RenderFrameStatus&) override;
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

    // generate the file path for the given index -> action_journal.{fileIndex}.log
    std::string getFilepath(uint32_t fileIndex) const;

    // detect and validate files at `ActionJournalOptions.path()/ACTION_JOURNAL_DIRECTORY_NAME`
    uint32_t detectFiles() const;

    // create a new log file with the highest unused index
    // delete the oldest one if `logFileCount` number of files was reached
    // rename the remaining files to match the index chain if a file was deleted
    //
    // action_journal.0.log -> deleted
    // action_journal.1.log -> action_journal.0.log
    // ...
    // action_journal.{logFileCount - 1}.log -> action_journal.{logFileCount - 2}.log
    // action_journal.{logFileCount - 1}.log -> new file
    uint32_t rollFiles();

    // open the given index file
    bool openFile(uint32_t fileIndex, bool truncate = false);
    // check if the current file can log `size` bytes of info and roll files if needed
    bool prepareFile(size_t size);
    // log string to file and flush
    void logToFile(const std::string& value);

protected:
    const Map& map;

    ActionJournalOptions options;

    const std::shared_ptr<Scheduler> scheduler;

    // log file
    std::mutex fileMutex;
    std::fstream currentFile;
    uint32_t currentFileIndex{0};
    size_t currentFileSize{0};

    // rendering info reporting
    double previousFrameTime;
    double renderingInfoReportTime;

    struct {
        double encodingMin{std::numeric_limits<double>::max()};
        double encodingMax{-std::numeric_limits<double>::max()};
        double encodingTotal{0.0};

        double renderingMin{std::numeric_limits<double>::max()};
        double renderingMax{-std::numeric_limits<double>::max()};
        double renderingTotal{0.0};

        uint32_t frameCount{0};
    } renderingStats;
};

} // namespace util
} // namespace mbgl
