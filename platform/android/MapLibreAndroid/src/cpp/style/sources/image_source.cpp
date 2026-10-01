#include "image_source.hpp"

// Java -> C++ conversion
#include "../android_conversion.hpp"

// C++ -> Java conversion
#include "../../conversion/conversion.hpp"
#include <mln/style/conversion_impl.hpp>
#include <mln/util/premultiply.hpp>

#include "bitmap.hpp"
#include <string>
#include <array>

namespace mln {
namespace android {

ImageSource::ImageSource(jni::JNIEnv& env,
                         const jni::String& sourceId,
                         const jni::Object<LatLngQuad>& coordinatesObject)
    : Source(env,
             std::make_unique<mln::style::ImageSource>(jni::Make<std::string>(env, sourceId),
                                                       LatLngQuad::getLatLngArray(env, coordinatesObject))) {}

ImageSource::ImageSource(jni::JNIEnv& env, mln::style::Source& coreSource, AndroidRendererFrontend* frontend)
    : Source(env, coreSource, createJavaPeer(env), frontend) {}

ImageSource::~ImageSource() = default;

void ImageSource::setURL(jni::JNIEnv& env, const jni::String& url) {
    // Update the core source
    source.as<mln::style::ImageSource>()->ImageSource::setURL(jni::Make<std::string>(env, url));
}

jni::Local<jni::String> ImageSource::getURL(jni::JNIEnv& env) {
    std::optional<std::string> url = source.as<mln::style::ImageSource>()->ImageSource::getURL();
    return url ? jni::Make<jni::String>(env, *url) : jni::Local<jni::String>();
}

void ImageSource::setImage(jni::JNIEnv& env, const jni::Object<Bitmap>& bitmap) {
    source.as<mln::style::ImageSource>()->setImage(Bitmap::GetImage(env, bitmap));
}

void ImageSource::setCoordinates(jni::JNIEnv& env, const jni::Object<LatLngQuad>& coordinatesObject) {
    source.as<mln::style::ImageSource>()->setCoordinates(LatLngQuad::getLatLngArray(env, coordinatesObject));
}

jni::Local<jni::Object<Source>> ImageSource::createJavaPeer(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<ImageSource>::Singleton(env);
    static auto constructor = javaClass.GetConstructor<jni::jlong>(env);
    return javaClass.New(env, constructor, reinterpret_cast<jni::jlong>(this));
}

void ImageSource::registerNative(jni::JNIEnv& env) {
    // Lookup the class
    static auto& javaClass = jni::Class<ImageSource>::Singleton(env);

#define METHOD(MethodPtr, name) jni::MakeNativePeerMethod<decltype(MethodPtr), (MethodPtr)>(name)

    // Register the peer
    jni::RegisterNativePeer<ImageSource>(env,
                                         javaClass,
                                         "nativePtr",
                                         jni::MakePeer<ImageSource, const jni::String&, const jni::Object<LatLngQuad>&>,
                                         "initialize",
                                         "finalize",
                                         METHOD(&ImageSource::setURL, "nativeSetUrl"),
                                         METHOD(&ImageSource::getURL, "nativeGetUrl"),
                                         METHOD(&ImageSource::setImage, "nativeSetImage"),
                                         METHOD(&ImageSource::setCoordinates, "nativeSetCoordinates"));
}

} // namespace android
} // namespace mln
