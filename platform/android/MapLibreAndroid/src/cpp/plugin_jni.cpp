#include "plugin_jni.hpp"

namespace mbgl {
namespace android {

namespace {
  std::vector<PluginJniRegistrationFn>& getJniRegistry() {
    static std::vector<PluginJniRegistrationFn> registry;
    return registry;
  }
}

void addPluginJniRegistration(PluginJniRegistrationFn fn) {
  getJniRegistry().push_back(fn);
}

void registerAllPluginJniNatives(jni::JNIEnv& env) {
  for (auto fn : getJniRegistry()) {
    fn(env);
  }
}

} // namespace android
} // namespace mbgl
