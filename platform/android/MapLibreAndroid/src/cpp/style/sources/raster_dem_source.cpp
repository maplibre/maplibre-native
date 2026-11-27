#include "raster_dem_source.hpp"

#include "../android_conversion.hpp"
#include "../value.hpp"
#include "../conversion/url_or_tileset.hpp"
#include "source.hpp"

#include <mln/util/variant.hpp>

#include <string>

namespace mln {
namespace android {

namespace {

// `encoding` is null when the Java side was not given one; the source then keeps the encoding of its TileJSON or
// tileset, which defaults to "mapbox".
std::optional<mln::style::SourceOptions> sourceOptions(jni::JNIEnv& env, const jni::String& encoding) {
    if (!encoding) {
        return std::nullopt;
    }
    const auto name = jni::Make<std::string>(env, encoding);
    mln::style::SourceOptions options;
    if (name == "mapbox") {
        options.rasterEncoding = mln::Tileset::RasterEncoding::Mapbox;
    } else if (name == "terrarium") {
        options.rasterEncoding = mln::Tileset::RasterEncoding::Terrarium;
    } else {
        throw std::invalid_argument("invalid encoding: " + name);
    }
    return options;
}

} // namespace

RasterDEMSource::RasterDEMSource(jni::JNIEnv& env,
                                 const jni::String& sourceId,
                                 const jni::Object<>& urlOrTileSet,
                                 jni::jint tileSize,
                                 const jni::String& encoding)
    : Source(env,
             std::make_unique<mln::style::RasterDEMSource>(jni::Make<std::string>(env, sourceId),
                                                           convertURLOrTileset(Value(env, urlOrTileSet)),
                                                           tileSize,
                                                           sourceOptions(env, encoding))) {}

RasterDEMSource::RasterDEMSource(jni::JNIEnv& env, mln::style::Source& coreSource, AndroidRendererFrontend* frontend)
    : Source(env, coreSource, createJavaPeer(env), frontend) {}

RasterDEMSource::~RasterDEMSource() = default;

jni::Local<jni::String> RasterDEMSource::getURL(jni::JNIEnv& env) {
    std::optional<std::string> url = source.as<mln::style::RasterDEMSource>()->RasterDEMSource::getURL();
    return url ? jni::Make<jni::String>(env, *url) : jni::Local<jni::String>();
}

jni::Local<jni::String> RasterDEMSource::getEncoding(jni::JNIEnv& env) {
    const auto encoding = source.as<mln::style::RasterDEMSource>()->getEncoding();
    return jni::Make<jni::String>(env, encoding == mln::Tileset::RasterEncoding::Terrarium ? "terrarium" : "mapbox");
}

jni::Local<jni::Object<Source>> RasterDEMSource::createJavaPeer(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<RasterDEMSource>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(this));
}

void RasterDEMSource::registerNative(jni::JNIEnv& env) {
    // Lookup the class
    static auto& javaClass = jni::Class<RasterDEMSource>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<RasterDEMSource>(
        env,
        javaClass,
        "nativePtr",
        jni::MakePeer<RasterDEMSource, const jni::String&, const jni::Object<>&, jni::jint, const jni::String&>,
        "initialize",
        "finalize",
        METHOD(&RasterDEMSource::getURL, "nativeGetUrl"),
        METHOD(&RasterDEMSource::getEncoding, "nativeGetEncoding"));
}

} // namespace android
} // namespace mln
