#pragma once

#include <mln/actor/actor.hpp>
#include <mln/annotation/annotation.hpp>
#include <mln/renderer/renderer_frontend.hpp>
#include <mln/util/geo.hpp>
#include <mln/util/run_loop.hpp>

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <mln/util/geojson.hpp>

#include "map_renderer.hpp"

#include <jni/jni.hpp>

namespace mln {

class RenderedQueryOptions;
class SourceQueryOptions;

namespace util {

class AsyncTask;

} // namespace util

namespace android {

class AndroidRendererFrontend : public RendererFrontend, public std::enable_shared_from_this<AndroidRendererFrontend> {
    struct Private {
        explicit Private() = default;
    };

public:
    AndroidRendererFrontend(Private, jni::JNIEnv&, const jni::Object<MapRenderer>&);
    static std::shared_ptr<AndroidRendererFrontend> create(jni::JNIEnv&, const jni::Object<MapRenderer>&);
    void init(jni::JNIEnv&, const jni::Object<MapRenderer>&);

    ~AndroidRendererFrontend() override;

    void reset() override;
    void setObserver(RendererObserver&) override;

    void update(std::shared_ptr<UpdateParameters>) override;

    const TaggedScheduler& getThreadPool() const override;

    // Feature querying
    std::vector<Feature> queryRenderedFeatures(const ScreenCoordinate&, const RenderedQueryOptions&) const;
    std::vector<Feature> queryRenderedFeatures(const ScreenBox&, const RenderedQueryOptions&) const;
    std::vector<Feature> querySourceFeatures(const std::string& sourceID, const SourceQueryOptions&) const;
    void setFeatureState(const std::string& sourceID,
                         const std::optional<std::string>& sourceLayerID,
                         const std::string& featureID,
                         const FeatureState& state) const;
    FeatureState getFeatureState(const std::string& sourceID,
                                 const std::optional<std::string>& sourceLayerID,
                                 const std::string& featureID) const;
    void removeFeatureState(const std::string& sourceID,
                            const std::optional<std::string>& sourceLayerID,
                            const std::optional<std::string>& featureID,
                            const std::optional<std::string>& stateKey) const;
    AnnotationIDs queryPointAnnotations(const ScreenBox& box, const std::chrono::milliseconds& timeout) const;
    AnnotationIDs queryShapeAnnotations(const ScreenBox& box, const std::chrono::milliseconds& timeout) const;

    // Feature extension query
    FeatureExtensionValue queryFeatureExtensions(const std::string& sourceID,
                                                 const Feature& feature,
                                                 const std::string& extension,
                                                 const std::string& extensionField,
                                                 const std::optional<std::map<std::string, mln::Value>>& args) const;

    // Memory
    void setTileCacheEnabled(bool);
    bool getTileCacheEnabled() const;
    void reduceMemoryUse();

private:
    MapRenderer& mapRenderer;
    util::RunLoop* mapRunLoop;
    std::unique_ptr<util::AsyncTask> updateAsyncTask;
    std::shared_ptr<UpdateParameters> updateParams;
};

} // namespace android
} // namespace mln
