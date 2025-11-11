#pragma once

#include <mbgl/actor/actor.hpp>
#include <mbgl/annotation/annotation.hpp>
#include <mbgl/renderer/renderer_frontend.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/run_loop.hpp>

#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <mbgl/util/geojson.hpp>

#include "map_renderer.hpp"

#include <jni/jni.hpp>

namespace mbgl {

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
    AnnotationIDs queryPointAnnotations(const ScreenBox& box, const std::chrono::milliseconds& timeout) const;
    AnnotationIDs queryShapeAnnotations(const ScreenBox& box, const std::chrono::milliseconds& timeout) const;

    // Feature extension query
    FeatureExtensionValue queryFeatureExtensions(const std::string& sourceID,
                                                 const Feature& feature,
                                                 const std::string& extension,
                                                 const std::string& extensionField,
                                                 const std::optional<std::map<std::string, mbgl::Value>>& args) const;

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
} // namespace mbgl
