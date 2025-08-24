#pragma once

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/style/transition_options.hpp>
#include <mbgl/style/observer.hpp>
#include <mbgl/style/source_observer.hpp>
#include <mbgl/style/layer_observer.hpp>
#include <mbgl/style/light_observer.hpp>
#include <mbgl/sprite/sprite_loader_observer.hpp>
#include <mbgl/style/image.hpp>
#include <mbgl/style/source.hpp>
#include <mbgl/style/layer.hpp>
#include <mbgl/style/collection.hpp>

#include <mbgl/text/glyph.hpp>

#include <mbgl/map/camera.hpp>

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/geo.hpp>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace mbgl {

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

    // Add style parsing filter
    void addStyleFilter(std::shared_ptr<mbgl::style::PluginStyleFilter>);

    std::string getName() const;
    CameraOptions getDefaultCamera() const;

    TransitionOptions getTransitionOptions() const;
    void setTransitionOptions(const TransitionOptions&);

    void setLight(std::unique_ptr<Light>);
    Light* getLight() const;

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
    void filterThenParse(const std::string& styleData);
    void parse(const std::string&);

    std::vector<std::shared_ptr<PluginStyleFilter>> _styleFilters;

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
} // namespace mbgl
