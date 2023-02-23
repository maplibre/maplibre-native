#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/tile_server_options.hpp>
#include <mbgl/util/default_style.hpp>

#include <jni/jni.hpp>

#include "default_style.hpp"

namespace mbgl {
namespace android {

class TileServerOptions : private mbgl::util::noncopyable {
public:

    static constexpr auto Name() { return "com/mapbox/mapboxsdk/util/TileServerOptions"; };

    static jni::Local<jni::Object<TileServerOptions>> New(jni::JNIEnv&, const mbgl::TileServerOptions&);

    static jni::Local<jni::Object<TileServerOptions>> DefaultConfiguration(jni::JNIEnv&, const jni::Class<TileServerOptions>&);
    static jni::Local<jni::Object<TileServerOptions>> MapboxConfiguration(jni::JNIEnv&, const jni::Class<TileServerOptions>&);
    static jni::Local<jni::Object<TileServerOptions>> MapTilerConfiguration(jni::JNIEnv&, const jni::Class<TileServerOptions>&);
    static jni::Local<jni::Object<TileServerOptions>> MapLibreConfiguration(jni::JNIEnv&, const jni::Class<TileServerOptions>&);

    static mbgl::TileServerOptions getTileServerOptions(jni::JNIEnv&, const jni::Object<TileServerOptions>&);
    
    static void registerNative(jni::JNIEnv&);

    static jni::Local<jni::Array<jni::Object<DefaultStyle>>> NewStyles(jni::JNIEnv& env, const std::vector<mbgl::util::DefaultStyle> &);
    static std::vector<mbgl::util::DefaultStyle> getDefaultStyles(jni::JNIEnv& env, const jni::Array<jni::Object<DefaultStyle>>& styles_);
    

};


} // namespace android
} // namespace mbgl
