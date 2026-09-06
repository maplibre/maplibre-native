#pragma once

#include <mln/actor/scheduler.hpp>
#include <mln/map/camera.hpp>
#include <mln/style/image.hpp>
#include <mln/style/transition_options.hpp>
#include <mln/util/feature.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/immutable.hpp>

#include <string>
#include <vector>
#include <memory>

namespace mln {

class FileSource;

namespace style {

class Light;
class Source;
class Layer;

class Style {
public:
    Style(std::shared_ptr<FileSource>, float pixelRatio, const TaggedScheduler& threadPool_);
    ~Style();

    void loadJSON(const std::string&);
    void loadURL(const std::string&);

    std::string getJSON() const;
    std::string getURL() const;

    /// False while a style passed to loadJSON/loadURL is still being fetched or parsed.
    bool isLoaded() const;

    // Defaults
    std::string getName() const;
    CameraOptions getDefaultCamera() const;

    // TransitionOptions
    TransitionOptions getTransitionOptions() const;
    void setTransitionOptions(const TransitionOptions&);

    // Light
    Light* getLight();
    const Light* getLight() const;

    void setLight(std::unique_ptr<Light>);

    // Global state, used by the "global-state" expression.
    // Setting a null value resets the property to the default defined in the
    // style's root "state" property (or null if there is none).
    GlobalStateMap getGlobalState() const;
    /// Throws if the style has not finished loading.
    void setGlobalStateProperty(const std::string& name, const Value& value);

    // Images
    std::optional<Image> getImage(const std::string&) const;
    void addImage(std::unique_ptr<Image>);
    void removeImage(const std::string&);

    // Sources
    std::vector<Source*> getSources();
    std::vector<const Source*> getSources() const;

    Source* getSource(const std::string&);
    const Source* getSource(const std::string&) const;

    void addSource(std::unique_ptr<Source>);
    std::unique_ptr<Source> removeSource(const std::string& sourceID);

    // Layers
    std::vector<Layer*> getLayers();
    std::vector<const Layer*> getLayers() const;

    Layer* getLayer(const std::string&);
    const Layer* getLayer(const std::string&) const;

    void addLayer(std::unique_ptr<Layer>, const std::optional<std::string>& beforeLayerID = std::nullopt);
    std::unique_ptr<Layer> removeLayer(const std::string& layerID);

    // Private implementation
    class Impl;
    const std::unique_ptr<Impl> impl;
};

} // namespace style
} // namespace mln
