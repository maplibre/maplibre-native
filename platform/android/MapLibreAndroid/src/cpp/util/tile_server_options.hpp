#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/util/tile_server_options.hpp>
#include <mln/util/default_style.hpp>

#include <jni/jni.hpp>

#include "default_style.hpp"

namespace mln {
namespace android {

class TileServerOptions : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/util/TileServerOptions"; };

    static jni::Local<jni::Object<TileServerOptions>> New(jni::JNIEnv&, const mln::TileServerOptions&);

    static jni::Local<jni::Object<TileServerOptions>> DefaultConfiguration(jni::JNIEnv&,
                                                                           const jni::Class<TileServerOptions>&);
    static jni::Local<jni::Object<TileServerOptions>> MapboxConfiguration(jni::JNIEnv&,
                                                                          const jni::Class<TileServerOptions>&);
    static jni::Local<jni::Object<TileServerOptions>> MapTilerConfiguration(jni::JNIEnv&,
                                                                            const jni::Class<TileServerOptions>&);
    static jni::Local<jni::Object<TileServerOptions>> MapLibreConfiguration(jni::JNIEnv&,
                                                                            const jni::Class<TileServerOptions>&);

    static mln::TileServerOptions getTileServerOptions(jni::JNIEnv&, const jni::Object<TileServerOptions>&);

    static void registerNative(jni::JNIEnv&);

    static jni::Local<jni::Array<jni::Object<DefaultStyle>>> NewStyles(jni::JNIEnv& env,
                                                                       const std::vector<mln::util::DefaultStyle>&);
    static std::vector<mln::util::DefaultStyle> getDefaultStyles(jni::JNIEnv& env,
                                                                 const jni::Array<jni::Object<DefaultStyle>>& styles_);
};

} // namespace android
} // namespace mln
