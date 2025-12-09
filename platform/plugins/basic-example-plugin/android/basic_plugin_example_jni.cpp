#include "BasicPluginExample.hpp"
#include "plugin_jni.hpp"

#include <jni/jni.hpp>
#include <memory>

namespace mbgl {
namespace android {

class BasicPluginExampleJni {
public:
  static constexpr auto Name() { return "org/maplibre/android/testapp/plugins/BasicPluginExample"; }

  static void registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<BasicPluginExampleJni>::Singleton(env);

    #define METHOD(MethodPtr, name) jni::MakeNativeMethod<decltype(MethodPtr), (MethodPtr)>(name)

    jni::RegisterNatives(
        env,
        *javaClass,
        METHOD(&BasicPluginExampleJni::nativeCreate, "nativeCreate"),
        METHOD(&BasicPluginExampleJni::nativeDestroy, "nativeDestroy"),
        METHOD(&BasicPluginExampleJni::nativeShowSanFrancisco, "nativeShowSanFrancisco"));
  }

  static jni::jlong nativeCreate(jni::JNIEnv&, const jni::Object<BasicPluginExampleJni>&) {
    auto plugin = new mbgl::platform::BasicPluginExample();
    return reinterpret_cast<jni::jlong>(plugin);
  }

  static void nativeDestroy(jni::JNIEnv&, const jni::Object<BasicPluginExampleJni>&, jni::jlong nativePtr) {
    if (nativePtr != 0) {
      auto plugin = reinterpret_cast<mbgl::platform::BasicPluginExample*>(nativePtr);
      delete plugin;
    }
  }

  static void nativeShowSanFrancisco(jni::JNIEnv&, const jni::Object<BasicPluginExampleJni>&, jni::jlong nativePtr) {
    if (nativePtr != 0) {
      auto plugin = reinterpret_cast<mbgl::platform::BasicPluginExample*>(nativePtr);
      plugin->showSanFrancisco();
    }
  }
};

void registerBasicPluginExampleNatives(jni::JNIEnv& env) {
  BasicPluginExampleJni::registerNative(env);
}

// Self-register JNI native methods for this plugin
MLN_PLUGIN_REGISTER_JNI_NATIVES(registerBasicPluginExampleNatives)

} // namespace android
} // namespace mbgl
