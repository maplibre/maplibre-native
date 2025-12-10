#include "JniBase.h"
#include "plugin_jni.hpp"
#include <jni/jni.hpp>

namespace {

void registerGluecodiumNatives(jni::JNIEnv& env) {
    JavaVM* vm;
    env.GetJavaVM(&vm);
    GLUECODIUM_JNI_ONLOAD(vm, nullptr);
}

} // namespace

// Auto-register Gluecodium JNI initialization with MapLibre's JNI flow
MLN_PLUGIN_REGISTER_JNI_NATIVES(registerGluecodiumNatives)
