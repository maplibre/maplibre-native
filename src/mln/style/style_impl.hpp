#pragma once

#include <mln/actor/scheduler.hpp>
#include <mln/style/style.hpp>
#include <mln/style/transition_options.hpp>
#include <mln/style/observer.hpp>
#include <mln/style/source_observer.hpp>
#include <mln/style/layer_observer.hpp>
#include <mln/style/light_observer.hpp>
#include <mln/sprite/sprite_loader_observer.hpp>
#include <mln/style/image.hpp>
#include <mln/style/source.hpp>
#include <mln/style/layer.hpp>
#include <mln/style/collection.hpp>

#include <mln/text/glyph.hpp>

#include <mln/map/camera.hpp>

#include <mln/util/noncopyable.hpp>
#include <mln/util/feature.hpp>
#include <mln/util/geo.hpp>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace mln {

class FileSource;
class AsyncRequest;
class SpriteLoader;

namespace style {

class Style::Impl : public SpriteLoaderObserver,
                    public SourceObserver,
                    public LayerObserver,
                    public LightObserver,
                    public util::noncopyable {
public:
    Impl(std::shared_ptr<FileSource>, float pixelRatio, const TaggedScheduler& threadPool_);
    ~Impl() override;

    void loadJSON(const std::string&);
    void loadURL(const std::string&);

    /**
     * @brief Cancels any pending style request.
     *
     * This will cancel any in-progress style URL load. Has no effect if no
     * style load is in progress.
     */
    void cancelPendingRequest() noexcept;

    std::string getJSON() const;
    std::string getURL() const;

    void setObserver(Observer*);

    bool isLoaded() const;

    std::exception_ptr getLastError() const { return lastError; }

    std::vector<Source*> getSources();
    std::vector<const Source*> getSources() const;
    Source* getSource(const std::string& id) const;

    void addSource(std::unique_ptr<Source>);
    std::unique_ptr<Source> removeSource(const std::string& sourceID);

    std::vector<Layer*> getLayers();
    std::vector<const Layer*> getLayers() const;
    Layer* getLayer(const std::string& id) const;

    Layer* addLayer(std::unique_ptr<Layer>, const std::optional<std::string>& beforeLayerID = std::nullopt);
    std::unique_ptr<Layer> removeLayer(const std::string& layerID);

    std::string getName() const;
    CameraOptions getDefaultCamera() const;

    TransitionOptions getTransitionOptions() const;
    void setTransitionOptions(const TransitionOptions&);

    void setLight(std::unique_ptr<Light>);
    Light* getLight() const;

    /// Set a global state property. A null value resets the property to the
    /// default defined in the style's root "state" property (or null).
    void setGlobalStateProperty(const std::string& property, const Value& value);
    GlobalStateMap getGlobalState() const;
    std::shared_ptr<const GlobalStateMap> getGlobalStateShared() const;

    /// Re-evaluate the visibility expressions of all layers against the
    /// current global state.
    void reevaluateLayerVisibilities();

    std::optional<Immutable<style::Image::Impl>> getImage(const std::string&) const;
    void addImage(std::unique_ptr<style::Image>);
    void removeImage(const std::string&);

    const std::string& getGlyphURL() const;
    std::shared_ptr<FontFaces> getFontFaces() const;

    using ImageImpls = std::vector<Immutable<Image::Impl>>;
    Immutable<ImageImpls> getImageImpls() const;
    Immutable<std::vector<Immutable<Source::Impl>>> getSourceImpls() const;
    Immutable<std::vector<Immutable<Layer::Impl>>> getLayerImpls() const;

    void dumpDebugLogs() const;
    bool areSpritesLoaded() const;

    bool mutated = false;
    bool loaded = false;

private:
    void parse(const std::string&);

    std::shared_ptr<FileSource> fileSource;

    std::string url;
    std::string json;

    std::unique_ptr<AsyncRequest> styleRequest;
    std::unique_ptr<SpriteLoader> spriteLoader;

    std::string glyphURL;
    std::shared_ptr<FontFaces> fontFaces;
    Immutable<ImageImpls> images = makeMutable<ImageImpls>();
    CollectionWithPersistentOrder<Source> sources;
    Collection<Layer> layers;
    TransitionOptions transitionOptions;
    std::unique_ptr<Light> light;
    std::unordered_map<std::string, bool> spritesLoadingStatus;

    // Global state for the "global-state" expression. The map itself is
    // immutable and replaced wholesale on change so that it can be safely
    // shared across threads.
    std::shared_ptr<const GlobalStateMap> globalState = std::make_shared<const GlobalStateMap>();
    GlobalStateMap globalStateDefaults;

    // Defaults
    std::string name;
    CameraOptions defaultCamera;

    // SpriteLoaderObserver implementation.
    void onSpriteLoaded(std::optional<style::Sprite> sprite, std::vector<Immutable<style::Image::Impl>>) override;
    void onSpriteError(std::optional<style::Sprite> sprite, std::exception_ptr) override;
    void onSpriteRequested(const std::optional<style::Sprite>&) override;

    // SourceObserver implementation.
    void onSourceLoaded(Source&) override;
    void onSourceChanged(Source&) override;
    void onSourceError(Source&, std::exception_ptr) override;
    void onSourceDescriptionChanged(Source&) override;

    // LayerObserver implementation.
    void onLayerChanged(Layer&) override;

    // LightObserver implementation.
    void onLightChanged(const Light&) override;

    Observer nullObserver;
    Observer* observer = &nullObserver;

    std::exception_ptr lastError;
};

} // namespace style
} // namespace mln
