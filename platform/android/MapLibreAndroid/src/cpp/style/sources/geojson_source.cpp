#include "geojson_source.hpp"
#include "attach_env.hpp"

#include <mln/map/world_coordinate_helper.hpp>
#include <mln/renderer/query.hpp>

// Java -> C++ conversion
#include "../android_conversion.hpp"
#include "../conversion/filter.hpp"
#include <mln/style/conversion/geojson.hpp>
#include <mln/style/conversion/geojson_options.hpp>
#include <mln/style/conversion_impl.hpp>

// C++ -> Java conversion
#include "../../conversion/conversion.hpp"
#include "../../conversion/collection.hpp"
#include "../../geojson/feature.hpp"
#include "../conversion/url_or_tileset.hpp"

#include <string>

// GeoJSONSource uses a "coalescing" model for high frequency asynchronous data
// update calls, which in practice means, that any update that started
// processing is going to finish and the last scheduled update is going to
// finish as well. Any updates scheduled during processing can be canceled. Conversion
// from Java features to core ones is done on a worker thread and once finished,
// the ownership of the converted features is returned to the calling thread.
namespace mln {
namespace android {

// This conversion is expected not to fail because it's used only in contexts where
// the value was originally a GeoJsonOptions object on the Java side. If it fails
// to convert, it's a bug in our serialization or Java-side static typing.
static Immutable<style::GeoJSONOptions> convertGeoJSONOptions(jni::JNIEnv& env, const jni::Object<>& options) {
    using namespace mln::style::conversion;
    if (!options) {
        return style::GeoJSONOptions::defaultOptions();
    }
    Error error;
    std::optional<style::GeoJSONOptions> result = convert<style::GeoJSONOptions>(mln::android::Value(env, options),
                                                                                 error);
    if (!result) {
        throw std::logic_error(error.message);
    }
    return makeMutable<style::GeoJSONOptions>(std::move(*result));
}

GeoJSONSource::GeoJSONSource(jni::JNIEnv& env, const jni::String& sourceId, const jni::Object<>& options)
    : Source(env,
             std::make_unique<mln::style::GeoJSONSource>(jni::Make<std::string>(env, sourceId),
                                                         convertGeoJSONOptions(env, options))),
      converter(
          std::make_unique<OptionalActor<FeatureConverter>>(source.as<style::GeoJSONSource>()->isUpdateSynchronous(),
                                                            Scheduler::GetBackground(),
                                                            source.as<style::GeoJSONSource>()->impl().getOptions())) {}

GeoJSONSource::GeoJSONSource(jni::JNIEnv& env, mln::style::Source& coreSource, AndroidRendererFrontend* frontend)
    : Source(env, coreSource, createJavaPeer(env), frontend),
      converter(
          std::make_unique<OptionalActor<FeatureConverter>>(source.as<style::GeoJSONSource>()->isUpdateSynchronous(),
                                                            Scheduler::GetBackground(),
                                                            source.as<style::GeoJSONSource>()->impl().getOptions())) {}

GeoJSONSource::~GeoJSONSource() = default;

void GeoJSONSource::addToMap(JNIEnv& env,
                             const jni::Object<Source>& obj,
                             mln::Map& map,
                             AndroidRendererFrontend& frontend) {
    Source::addToMap(env, obj, map, frontend);
    giveLastData();
}

void GeoJSONSource::addToStyle(JNIEnv& env, const jni::Object<Source>& obj, mln::style::Style& style) {
    Source::addToStyle(env, obj, style);
    giveLastData();
}

void GeoJSONSource::setGeoJSONString(jni::JNIEnv& env, const jni::String& jString) {
    std::shared_ptr<std::string> json = std::make_shared<std::string>(jni::Make<std::string>(env, jString));

    OptionalActorRef<FeatureConverter> converterRef = converter->self();
    Update::Converter converterFn = [converterRef, json](ActorRef<GeoJSONDataCallback> _callback,
                                                         std::shared_ptr<const CRSWorldCoordinateHelper> projection) {
        converterRef.invoke(&FeatureConverter::convertJson, json, std::move(projection), _callback);
    };

    setAsync(converterFn);
}

void GeoJSONSource::setFeatureCollection(jni::JNIEnv& env, const jni::Object<geojson::FeatureCollection>& jFeatures) {
    setCollectionAsync(env, jFeatures);
}

void GeoJSONSource::setFeature(jni::JNIEnv& env, const jni::Object<geojson::Feature>& jFeature) {
    setCollectionAsync(env, jFeature);
}

void GeoJSONSource::setGeometry(jni::JNIEnv& env, const jni::Object<geojson::Geometry>& jGeometry) {
    setCollectionAsync(env, jGeometry);
}

void GeoJSONSource::setURL(jni::JNIEnv& env, const jni::String& url) {
    // Update the core source
    source.as<style::GeoJSONSource>()->setURL(jni::Make<std::string>(env, url));
}

jni::Local<jni::String> GeoJSONSource::getURL(jni::JNIEnv& env) {
    std::optional<std::string> url = source.as<style::GeoJSONSource>()->getURL();
    return url ? jni::Make<jni::String>(env, *url) : jni::Local<jni::String>();
}

jni::Local<jni::Array<jni::Object<geojson::Feature>>> GeoJSONSource::querySourceFeatures(
    jni::JNIEnv& env, const jni::Array<jni::Object<>>& jfilter) {
    using namespace mln::android::conversion;
    using namespace mln::android::geojson;

    std::vector<mln::Feature> features;
    if (rendererFrontend) {
        features = rendererFrontend->querySourceFeatures(source.getID(), {{}, toFilter(env, jfilter)});
    }
    return Feature::convert(env, features);
}

jni::Local<jni::Array<jni::Object<geojson::Feature>>> GeoJSONSource::getClusterChildren(
    jni::JNIEnv& env, const jni::Object<geojson::Feature>& feature) {
    using namespace mln::android::conversion;
    using namespace mln::android::geojson;

    if (rendererFrontend) {
        mln::Feature _feature = Feature::convert(env, feature);
        _feature.properties["cluster_id"] = static_cast<uint64_t>(_feature.properties["cluster_id"].get<double>());
        const auto featureExtension = rendererFrontend->queryFeatureExtensions(
            source.getID(), _feature, "supercluster", "children", {});
        if (featureExtension.is<mln::FeatureCollection>()) {
            return Feature::convert(env, featureExtension.get<mln::FeatureCollection>());
        }
    }
    return jni::Array<jni::Object<Feature>>::New(env, 0);
}

jni::Local<jni::Array<jni::Object<geojson::Feature>>> GeoJSONSource::getClusterLeaves(
    jni::JNIEnv& env, const jni::Object<geojson::Feature>& feature, jni::jlong limit, jni::jlong offset) {
    using namespace mln::android::conversion;
    using namespace mln::android::geojson;

    if (rendererFrontend) {
        mln::Feature _feature = Feature::convert(env, feature);
        _feature.properties["cluster_id"] = static_cast<uint64_t>(_feature.properties["cluster_id"].get<double>());
        const std::map<std::string, mln::Value> options = {{"limit", static_cast<uint64_t>(limit)},
                                                           {"offset", static_cast<uint64_t>(offset)}};
        auto featureExtension = rendererFrontend->queryFeatureExtensions(
            source.getID(), _feature, "supercluster", "leaves", options);
        if (featureExtension.is<mln::FeatureCollection>()) {
            return Feature::convert(env, featureExtension.get<mln::FeatureCollection>());
        }
    }
    return jni::Array<jni::Object<Feature>>::New(env, 0);
    ;
}

jint GeoJSONSource::getClusterExpansionZoom(jni::JNIEnv& env, const jni::Object<geojson::Feature>& feature) {
    using namespace mln::android::conversion;
    using namespace mln::android::geojson;

    if (rendererFrontend) {
        mln::Feature _feature = Feature::convert(env, feature);
        _feature.properties["cluster_id"] = static_cast<uint64_t>(_feature.properties["cluster_id"].get<double>());
        auto featureExtension = rendererFrontend->queryFeatureExtensions(
            source.getID(), _feature, "supercluster", "expansion-zoom", {});
        if (featureExtension.is<mln::Value>()) {
            auto value = featureExtension.get<mln::Value>();
            if (value.is<uint64_t>()) {
                return value.get<uint64_t>();
            }
        }
    }
    return 0;
}

jni::Local<jni::Object<Source>> GeoJSONSource::createJavaPeer(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<GeoJSONSource>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(this));
}

template <class JNIType>
void GeoJSONSource::setCollectionAsync(jni::JNIEnv& env, const jni::Object<JNIType>& jObject) {
    auto global = jni::NewGlobal<jni::EnvAttachingDeleter>(env, jObject);
    auto object = std::make_shared<decltype(global)>(std::move(global));

    OptionalActorRef<FeatureConverter> converterRef = converter->self();
    Update::Converter converterFn = [converterRef, object](ActorRef<GeoJSONDataCallback> _callback,
                                                           std::shared_ptr<const CRSWorldCoordinateHelper> projection) {
        converterRef.invoke(&FeatureConverter::convertObject<JNIType>, object, std::move(projection), _callback);
    };

    setAsync(converterFn);
}

void GeoJSONSource::setAsync(Update::Converter converterFn) {
    awaitingUpdate = std::make_unique<Update>(
        std::move(converterFn),
        std::make_unique<Actor<GeoJSONDataCallback>>(*Scheduler::GetCurrent(),
                                                     [this](std::shared_ptr<style::GeoJSONData> geoJSONData) {
                                                         // conversion from Java features to core ones finished
                                                         android::UniqueEnv _env = android::AttachEnv();

                                                         lastUpdate = std::move(update);
                                                         lastData = std::move(geoJSONData);
                                                         if (!ownedSource) {
                                                             giveLastData();
                                                         }
                                                         runAwaitingUpdate();
                                                     }));
    runAwaitingUpdate();
}

std::shared_ptr<const CRSWorldCoordinateHelper> GeoJSONSource::styleProjection() const {
    return style::GeoJSONSource::Impl::registeredProjection(*source.as<style::GeoJSONSource>());
}

void GeoJSONSource::runAwaitingUpdate() {
    if (update || !awaitingUpdate) {
        return;
    }
    lastUpdate.reset();
    lastData.reset();
    update = std::move(awaitingUpdate);
    update->converterFn(update->callback->self(), styleProjection());
}

void GeoJSONSource::giveLastData() {
    if (!lastUpdate) {
        return;
    }
    const auto projection = styleProjection();
    const std::string tiledIn = lastData ? lastData->getProjection() : std::string();
    if (tiledIn == (projection ? projection->name() : std::string())) {
        source.as<style::GeoJSONSource>()->setGeoJSONData(std::move(lastData));
    } else if (!awaitingUpdate) {
        awaitingUpdate = std::move(lastUpdate);
    }
    lastUpdate.reset();
    lastData.reset();
    runAwaitingUpdate();
}

jboolean GeoJSONSource::isUpdateSynchronous(jni::JNIEnv&) {
    return source.as<style::GeoJSONSource>()->isUpdateSynchronous();
}

void GeoJSONSource::setOverrideSynchronousUpdate(jni::JNIEnv&, jni::jboolean newOverride) {
    source.as<style::GeoJSONSource>()->setOverrideSynchronousUpdate(newOverride);
}

void GeoJSONSource::registerNative(jni::JNIEnv& env) {
    // Lookup the class
    static auto& javaClass = jni::Class<GeoJSONSource>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<GeoJSONSource>(
        env,
        javaClass,
        "nativePtr",
        jni::MakePeer<GeoJSONSource, const jni::String&, const jni::Object<>&>,
        "initialize",
        "finalize",
        METHOD(&GeoJSONSource::setGeoJSONString, "nativeSetGeoJsonString"),
        METHOD(&GeoJSONSource::setFeatureCollection, "nativeSetFeatureCollection"),
        METHOD(&GeoJSONSource::setFeature, "nativeSetFeature"),
        METHOD(&GeoJSONSource::setGeometry, "nativeSetGeometry"),
        METHOD(&GeoJSONSource::setURL, "nativeSetUrl"),
        METHOD(&GeoJSONSource::getURL, "nativeGetUrl"),
        METHOD(&GeoJSONSource::querySourceFeatures, "querySourceFeatures"),
        METHOD(&GeoJSONSource::getClusterChildren, "nativeGetClusterChildren"),
        METHOD(&GeoJSONSource::getClusterLeaves, "nativeGetClusterLeaves"),
        METHOD(&GeoJSONSource::getClusterExpansionZoom, "nativeGetClusterExpansionZoom"),
        METHOD(&GeoJSONSource::isUpdateSynchronous, "nativeIsUpdateSynchronous"),
        METHOD(&GeoJSONSource::setOverrideSynchronousUpdate, "nativeSetOverrideSynchronousUpdate"));
}

void FeatureConverter::convertJson(std::shared_ptr<std::string> json,
                                   std::shared_ptr<const CRSWorldCoordinateHelper> projection,
                                   ActorRef<GeoJSONDataCallback> callback) {
    using namespace mln::style::conversion;

    android::UniqueEnv _env = android::AttachEnv();
    // Convert the jni object
    Error error;
    std::optional<GeoJSON> converted = parseGeoJSON(*json, error);
    if (!converted) {
        mln::Log::Error(mln::Event::JNI, "Error setting geo json: " + error.message);
        return;
    }

    callback.invoke(
        &GeoJSONDataCallback::operator(),
        style::GeoJSONSource::Impl::createData(*converted, sequencedScheduler, options, std::move(projection)));
}

template <class JNIType>
void FeatureConverter::convertObject(
    std::shared_ptr<jni::Global<jni::Object<JNIType>, jni::EnvAttachingDeleter>> jObject,
    std::shared_ptr<const CRSWorldCoordinateHelper> projection,
    ActorRef<GeoJSONDataCallback> callback) {
    using namespace mln::android::geojson;

    android::UniqueEnv _env = android::AttachEnv();
    // Convert the jni object
    auto geometry = JNIType::convert(*_env, *jObject);
    callback.invoke(
        &GeoJSONDataCallback::operator(),
        style::GeoJSONSource::Impl::createData(geometry, sequencedScheduler, options, std::move(projection)));
}

Update::Update(Converter _converterFn, std::unique_ptr<Actor<GeoJSONDataCallback>> _callback)
    : converterFn(std::move(_converterFn)),
      callback(std::move(_callback)) {}

} // namespace android
} // namespace mln
