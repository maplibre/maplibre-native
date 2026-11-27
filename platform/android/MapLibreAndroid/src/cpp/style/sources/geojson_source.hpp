#pragma once

#include "source.hpp"
#include <mln/style/sources/geojson_source.hpp>
#include <mln/style/sources/geojson_source_impl.hpp>
#include <mln/actor/optional_actor.hpp>
#include "../../geojson/geometry.hpp"
#include "../../geojson/feature.hpp"
#include "../../geojson/feature_collection.hpp"
#include "../../android_renderer_frontend.hpp"
#include <jni/jni.hpp>

namespace mln {
namespace android {

using GeoJSONDataCallback = std::function<void(std::shared_ptr<style::GeoJSONData>)>;

class FeatureConverter {
public:
    explicit FeatureConverter(Immutable<style::GeoJSONOptions> options_)
        : options(std::move(options_)),
          sequencedScheduler(Scheduler::GetSequenced()) {}
    void convertJson(std::shared_ptr<std::string>,
                     std::shared_ptr<const CRSWorldCoordinateHelper>,
                     ActorRef<GeoJSONDataCallback>);

    template <class JNIType>
    void convertObject(std::shared_ptr<jni::Global<jni::Object<JNIType>, jni::EnvAttachingDeleter>>,
                       std::shared_ptr<const CRSWorldCoordinateHelper>,
                       ActorRef<GeoJSONDataCallback>);

private:
    Immutable<style::GeoJSONOptions> options;
    std::shared_ptr<Scheduler> sequencedScheduler;
};

struct Update {
    /// Converts the data and tiles it in the projection given, in lat/lng for null.
    using Converter =
        std::function<void(ActorRef<GeoJSONDataCallback>, std::shared_ptr<const CRSWorldCoordinateHelper>)>;
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
    GeoJSONSource(jni::JNIEnv&, mln::style::Source&, AndroidRendererFrontend*);
    ~GeoJSONSource();

    void addToMap(JNIEnv&, const jni::Object<Source>&, mln::Map&, AndroidRendererFrontend&) override;
    void addToStyle(JNIEnv&, const jni::Object<Source>&, mln::style::Style&) override;

private:
    void setGeoJSONString(jni::JNIEnv&, const jni::String&);
    void setFeatureCollection(jni::JNIEnv&, const jni::Object<geojson::FeatureCollection>&);
    void setFeature(jni::JNIEnv&, const jni::Object<geojson::Feature>&);
    void setGeometry(jni::JNIEnv&, const jni::Object<geojson::Geometry>&);

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
    /// The last update and the data it made, held until the source is in a style, to give the core source the data
    /// if it is tiled for the style and to convert it again if it is not.
    std::unique_ptr<Update> lastUpdate;
    std::shared_ptr<style::GeoJSONData> lastData;
    std::shared_ptr<ThreadPool> threadPool;
    std::unique_ptr<OptionalActor<FeatureConverter>> converter;

    template <class JNIType>
    void setCollectionAsync(jni::JNIEnv&, const jni::Object<JNIType>&);

    void setAsync(Update::Converter);

    /// The projection of the style the source is in, which data is tiled in: null in lat/lng and before the source
    /// joins a style.
    std::shared_ptr<const CRSWorldCoordinateHelper> styleProjection() const;

    void runAwaitingUpdate();

    /// Gives the core source the last data if it is tiled in the projection of the style the source is in, and
    /// otherwise converts it again in that projection, as data converted before the source joins a style in a
    /// registered projection is.
    void giveLastData();

    template <class JNIType>
    void setCollectionSync(jni::JNIEnv&, const jni::Object<JNIType>&);

    jboolean isUpdateSynchronous(jni::JNIEnv&);
    void setOverrideSynchronousUpdate(jni::JNIEnv&, jni::jboolean);

}; // class GeoJSONSource

} // namespace android
} // namespace mln
