#include "custom_vector_source.hpp"
#include "attach_env.hpp"

#include <mbgl/style/sources/custom_vector_source.hpp>

#include <string>

namespace mbgl {
namespace android {

CustomVectorSource::CustomVectorSource(jni::JNIEnv& env,
                                       const jni::String& sourceId,
                                       jni::jint minZoom,
                                       jni::jint maxZoom)
    : Source(env,
             std::make_unique<mbgl::style::CustomVectorSource>(
                 jni::Make<std::string>(env, sourceId),
                 mbgl::style::CustomVectorSource::Options{
                     std::bind(&CustomVectorSource::fetchTile, this, std::placeholders::_1),
                     std::bind(&CustomVectorSource::cancelTile, this, std::placeholders::_1),
                     {static_cast<uint8_t>(minZoom), static_cast<uint8_t>(maxZoom)}})) {}

CustomVectorSource::~CustomVectorSource() {
    onRemovedFromMap();
}

void CustomVectorSource::fetchTile(const mbgl::CanonicalTileID& tileID) {
    android::UniqueEnv _env = android::AttachEnv();

    static auto& javaClass = jni::Class<CustomVectorSource>::Singleton(*_env);
    static auto fetchTile = javaClass.GetMethod<void(jni::jint, jni::jint, jni::jint)>(*_env, "fetchTile");

    if (!javaPeer) {
        return;
    }

    auto peer = jni::Cast(*_env, javaClass, javaPeer);
    peer.Call(*_env, fetchTile, (int)tileID.z, (int)tileID.x, (int)tileID.y);
}

void CustomVectorSource::cancelTile(const mbgl::CanonicalTileID& tileID) {
    android::UniqueEnv _env = android::AttachEnv();

    static auto& javaClass = jni::Class<CustomVectorSource>::Singleton(*_env);
    static auto cancelTile = javaClass.GetMethod<void(jni::jint, jni::jint, jni::jint)>(*_env, "cancelTile");

    if (!javaPeer) {
        return;
    }

    auto peer = jni::Cast(*_env, javaClass, javaPeer);
    peer.Call(*_env, cancelTile, (int)tileID.z, (int)tileID.x, (int)tileID.y);
}

void CustomVectorSource::onAddedToMap() {
    android::UniqueEnv _env = android::AttachEnv();

    static auto& javaClass = jni::Class<CustomVectorSource>::Singleton(*_env);
    static auto onAddedToMap = javaClass.GetMethod<void()>(*_env, "onAddedToMap");

    assert(javaPeer);

    auto peer = jni::Cast(*_env, javaClass, javaPeer);
    peer.Call(*_env, onAddedToMap);
}

void CustomVectorSource::onRemovedFromMap() {
    android::UniqueEnv _env = android::AttachEnv();

    static auto& javaClass = jni::Class<CustomVectorSource>::Singleton(*_env);
    static auto onRemovedFromMap = javaClass.GetMethod<void()>(*_env, "onRemovedFromMap");

    if (javaPeer) {
        auto peer = jni::Cast(*_env, javaClass, javaPeer);
        peer.Call(*_env, onRemovedFromMap);
    }
}

void CustomVectorSource::setTileData(jni::JNIEnv& env, jni::jint z, jni::jint x, jni::jint y,
                                      const jni::Array<jni::jbyte>& jData, jni::jint format) {
    std::shared_ptr<std::string> data;
    jsize length = jData.Length(env);
    if (length > 0) {
        data = std::make_shared<std::string>(length, char());
        jni::GetArrayRegion(env, *jData, 0, length, reinterpret_cast<jbyte*>(&(*data)[0]));
    }

    source.as<mbgl::style::CustomVectorSource>()->setTileData(
        CanonicalTileID(z, x, y),
        data,
        static_cast<mbgl::style::TileDataFormat>(format));
}

void CustomVectorSource::setTileError(jni::JNIEnv& env, jni::jint z, jni::jint x, jni::jint y,
                                       const jni::String& message) {
    auto msg = jni::Make<std::string>(env, message);
    source.as<mbgl::style::CustomVectorSource>()->setTileError(
        CanonicalTileID(z, x, y),
        std::make_exception_ptr(std::runtime_error(msg)));
}

void CustomVectorSource::invalidateTile(jni::JNIEnv&, jni::jint z, jni::jint x, jni::jint y) {
    source.as<mbgl::style::CustomVectorSource>()->invalidateTile(CanonicalTileID(z, x, y));
}

void CustomVectorSource::addToMap(JNIEnv& env,
                                   const jni::Object<Source>& obj,
                                   mbgl::Map& map,
                                   AndroidRendererFrontend& frontend) {
    Source::addToMap(env, obj, map, frontend);
    onAddedToMap();
}

bool CustomVectorSource::removeFromMap(JNIEnv& env, const jni::Object<Source>& source, mbgl::Map& map) {
    bool successfullyRemoved = Source::removeFromMap(env, source, map);
    if (successfullyRemoved) {
        onRemovedFromMap();
    }
    return successfullyRemoved;
}

void CustomVectorSource::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<CustomVectorSource>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    jni::RegisterNativePeer<CustomVectorSource>(
        env,
        javaClass,
        "nativePtr",
        jni::MakePeer<CustomVectorSource, const jni::String&, jni::jint, jni::jint>,
        "initialize",
        "finalize",
        METHOD(&CustomVectorSource::setTileData, "nativeSetTileData"),
        METHOD(&CustomVectorSource::setTileError, "nativeSetTileError"),
        METHOD(&CustomVectorSource::invalidateTile, "nativeInvalidateTile"));
}

} // namespace android
} // namespace mbgl
