#pragma once

#include <mbgl/style/layers/custom_drawable_layer.hpp>
#include <mbgl/util/geo.hpp>
#include <mbgl/util/gltf_loader.hpp>
#include <mbgl/util/mat4.hpp>

#include <jni/jni.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mbgl {
namespace android {

class GltfModelLayerHost : public style::CustomDrawableLayerHost {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/layers/GltfModelLayer"; }

    GltfModelLayerHost(std::string modelPath,
                       LatLng position,
                       float scale,
                       std::array<float, 3> rotationDegrees,
                       std::array<bool, 3> mirrorAxes);

    GltfModelLayerHost(std::vector<uint8_t> modelData,
                       LatLng position,
                       float scale,
                       std::array<float, 3> rotationDegrees,
                       std::array<bool, 3> mirrorAxes);

    void initialize() override;
    void update(Interface& interface) override;
    void deinitialize() override;

    static jni::jlong createNativeHost(jni::JNIEnv& env,
                                       const jni::Class<GltfModelLayerHost>&,
                                       const jni::String& path,
                                       jni::jdouble lat,
                                       jni::jdouble lng,
                                       jni::jfloat scale,
                                       jni::jfloat rotX,
                                       jni::jfloat rotY,
                                       jni::jfloat rotZ,
                                       jni::jboolean mirrorX,
                                       jni::jboolean mirrorY,
                                       jni::jboolean mirrorZ);

    static jni::jlong createNativeHostFromData(jni::JNIEnv& env,
                                               const jni::Class<GltfModelLayerHost>&,
                                               const jni::Array<jni::jbyte>& data,
                                               jni::jdouble lat,
                                               jni::jdouble lng,
                                               jni::jfloat scale,
                                               jni::jfloat rotX,
                                               jni::jfloat rotY,
                                               jni::jfloat rotZ,
                                               jni::jboolean mirrorX,
                                               jni::jboolean mirrorY,
                                               jni::jboolean mirrorZ);

    static void registerNative(jni::JNIEnv& env);

private:
    struct TransformInvariants {
        float rotX = 0.0f;
        float rotY = 0.0f;
        float rotZ = 0.0f;
        float sx = 1.0f;
        float sy = 1.0f;
        float sz = 1.0f;
        double earthCircumAtLat = 1.0;
    };

    std::string modelPath;
    std::vector<uint8_t> modelData;
    LatLng position;
    float scale;
    std::array<float, 3> rotationDegrees;
    std::array<bool, 3> mirrorAxes;
    TransformInvariants transformInvariants;
    util::GltfModel model;
};

} // namespace android
} // namespace mbgl
