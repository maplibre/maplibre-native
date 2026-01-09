#pragma once

#include "source.hpp"
#include <mbgl/style/sources/geojson_source.hpp>
#include "../../geojson/geometry.hpp"
#include "../../geojson/feature.hpp"
#include "../../geojson/feature_collection.hpp"
#include "../../android_renderer_frontend.hpp"
#include <jni/jni.hpp>

namespace mbgl {
namespace android {

using GeoJSONDataCallback = std::function<void(std::shared_ptr<style::GeoJSONData>)>;

class FeatureConverter {
public:
    explicit FeatureConverter(Immutable<style::GeoJSONOptions> options_)
        : options(std::move(options_)),
          sequencedScheduler(Scheduler::GetSequenced()) {}
    void convertJson(std::shared_ptr<std::string>, ActorRef<GeoJSONDataCallback>);

    template <class JNIType>
    void convertObject(std::shared_ptr<jni::Global<jni::Object<JNIType>, jni::EnvAttachingDeleter>>,
                       ActorRef<GeoJSONDataCallback>);

private:
    Immutable<style::GeoJSONOptions> options;
    std::shared_ptr<Scheduler> sequencedScheduler;
};

struct Update {
    using Converter = std::function<void(ActorRef<GeoJSONDataCallback>)>;
    Converter converterFn;

    std::unique_ptr<Actor<GeoJSONDataCallback>> callback;

    Update(Converter, std::unique_ptr<Actor<GeoJSONDataCallback>>);
};

class GeoJSONSource : public Source {
public:
    using SuperTag = Source;
    static constexpr auto Name() { return "org/maplibre/android/style/sources/GeoJsonSource"; };

    static void registerNative(jni::JNIEnv&);

    GeoJSONSource(jni::JNIEnv&, const jni::String&, const jni::Object<>&);
    GeoJSONSource(jni::JNIEnv&, mbgl::style::Source&, AndroidRendererFrontend*);
    ~GeoJSONSource();

private:
    void setGeoJSONString(jni::JNIEnv&, const jni::String&);
    void setFeatureCollection(jni::JNIEnv&, const jni::Object<geojson::FeatureCollection>&);
    void setFeature(jni::JNIEnv&, const jni::Object<geojson::Feature>&);
    void setGeometry(jni::JNIEnv&, const jni::Object<geojson::Geometry>&);

    void setGeoJSONStringSync(jni::JNIEnv&, const jni::String&);
    void setFeatureCollectionSync(jni::JNIEnv&, const jni::Object<geojson::FeatureCollection>&);
    void setFeatureSync(jni::JNIEnv&, const jni::Object<geojson::Feature>&);
    void setGeometrySync(jni::JNIEnv&, const jni::Object<geojson::Geometry>&);

    void setURL(jni::JNIEnv&, const jni::String&);

    jni::Local<jni::Array<jni::Object<geojson::Feature>>> querySourceFeatures(jni::JNIEnv&,
                                                                              const jni::Array<jni::Object<>>&);

    jni::Local<jni::Array<jni::Object<geojson::Feature>>> getClusterChildren(jni::JNIEnv&,
                                                                             const jni::Object<geojson::Feature>&);
    jni::Local<jni::Array<jni::Object<geojson::Feature>>> getClusterLeaves(jni::JNIEnv&,
                                                                           const jni::Object<geojson::Feature>&,
                                                                           jni::jlong,
                                                                           jni::jlong);
    jint getClusterExpansionZoom(jni::JNIEnv&, const jni::Object<geojson::Feature>&);

    jni::Local<jni::String> getURL(jni::JNIEnv&);

    jni::Local<jni::Object<Source>> createJavaPeer(jni::JNIEnv&);
    std::unique_ptr<Update> awaitingUpdate;
    std::unique_ptr<Update> update;
    std::shared_ptr<ThreadPool> threadPool;
    std::unique_ptr<Actor<FeatureConverter>> converter;

    template <class JNIType>
    void setCollectionAsync(jni::JNIEnv&, const jni::Object<JNIType>&);

    void setAsync(Update::Converter);

    template <class JNIType>
    void setCollectionSync(jni::JNIEnv&, const jni::Object<JNIType>&);

    jboolean isUpdateSynchronous(jni::JNIEnv&);

}; // class GeoJSONSource

} // namespace android
} // namespace mbgl
