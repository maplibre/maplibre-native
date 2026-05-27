#include "gltf_model_layer_host.hpp"

#include <mbgl/gfx/drawable.hpp>
#include <mbgl/renderer/paint_parameters.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/mat4.hpp>
#include <mbgl/util/projection.hpp>
#include <mbgl/math/angles.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/premultiply.hpp>

#include <cmath>
#include <limits>

namespace mbgl {
namespace android {

namespace {

Point<double> projectLocation(const LatLng& loc, const TransformState& state) {
    LatLng unwrapped = loc.wrapped();
    unwrapped.unwrapForShortestPath(state.getLatLng(LatLng::Wrapped));
    return Projection::project(unwrapped, state.getScale());
}

} // anonymous namespace

GltfModelLayerHost::GltfModelLayerHost(std::string modelPath_,
                                       LatLng position_,
                                       float scale_,
                                       std::array<float, 3> rotationDegrees_,
                                       std::array<bool, 3> mirrorAxes_)
    : modelPath(std::move(modelPath_)),
      position(std::move(position_)),
      scale(scale_),
      rotationDegrees(rotationDegrees_),
      mirrorAxes(mirrorAxes_) {
    transformInvariants.rotX = util::deg2radf(rotationDegrees[0]);
    transformInvariants.rotY = util::deg2radf(rotationDegrees[1]);
    transformInvariants.rotZ = util::deg2radf(rotationDegrees[2]);
    transformInvariants.sx = mirrorAxes[0] ? -1.0f : 1.0f;
    transformInvariants.sy = mirrorAxes[1] ? -1.0f : 1.0f;
    transformInvariants.sz = mirrorAxes[2] ? -1.0f : 1.0f;
    const double latRad = position.latitude() * M_PI / 180.0;
    transformInvariants.earthCircumAtLat = 2.0 * M_PI * 6378137.0 * std::cos(latRad);
}

GltfModelLayerHost::GltfModelLayerHost(std::vector<uint8_t> modelData_,
                                       LatLng position_,
                                       float scale_,
                                       std::array<float, 3> rotationDegrees_,
                                       std::array<bool, 3> mirrorAxes_)
    : modelData(std::move(modelData_)),
      position(std::move(position_)),
      scale(scale_),
      rotationDegrees(rotationDegrees_),
      mirrorAxes(mirrorAxes_) {
    transformInvariants.rotX = util::deg2radf(rotationDegrees[0]);
    transformInvariants.rotY = util::deg2radf(rotationDegrees[1]);
    transformInvariants.rotZ = util::deg2radf(rotationDegrees[2]);
    transformInvariants.sx = mirrorAxes[0] ? -1.0f : 1.0f;
    transformInvariants.sy = mirrorAxes[1] ? -1.0f : 1.0f;
    transformInvariants.sz = mirrorAxes[2] ? -1.0f : 1.0f;
    const double latRad = position.latitude() * M_PI / 180.0;
    transformInvariants.earthCircumAtLat = 2.0 * M_PI * 6378137.0 * std::cos(latRad);
}

void GltfModelLayerHost::initialize() {
    if (!modelData.empty()) {
        Log::Info(Event::General, "GltfModelLayerHost: loading model from memory (" +
                  std::to_string(modelData.size()) + " bytes)");
        std::string dataStr(reinterpret_cast<const char*>(modelData.data()), modelData.size());
        model = util::loadGltfFromMemory(dataStr);
        modelData.clear();
    } else {
        Log::Info(Event::General, "GltfModelLayerHost: loading model from path: " + modelPath);
        model = util::loadGltf(modelPath);
    }

    if (!model.valid()) {
        Log::Error(Event::General, "GltfModelLayerHost: failed to load model");
    } else {
        Log::Info(Event::General,
                  "GltfModelLayerHost: loaded " + std::to_string(model.meshes.size()) + " mesh(es), texture=" +
                      std::to_string(model.texture.has_value()));
        for (size_t i = 0; i < model.meshes.size(); ++i) {
            Log::Info(Event::General,
                      "  mesh[" + std::to_string(i) + "]: " +
                          std::to_string(model.meshes[i].vertices->elements()) + " vertices, " +
                          std::to_string(model.meshes[i].indices->elements()) + " triangles");
        }
    }
}

void GltfModelLayerHost::update(Interface& interface) {
    if (!model.valid()) {
        Log::Warning(Event::General, "GltfModelLayerHost::update: model is not valid, skipping render");
        return;
    }

    if (interface.getDrawableCount() > 0) {
        return;
    }

    Log::Info(Event::General, "GltfModelLayerHost::update: adding geometry for " +
              std::to_string(model.meshes.size()) + " mesh(es)");

    struct FrameCache {
        uint64_t lastFrameCount = std::numeric_limits<uint64_t>::max();
        float xyScale = 0.0f;
        mat4 nearClippedProjectionTranslated = matrix::identity4();
    };

    const LatLng loc = position;
    const float modelScale = scale;
    const auto invariants = transformInvariants;
    auto frameCache = std::make_shared<FrameCache>();

    interface.setGeometryTweakerCallback(
        [loc, modelScale, invariants, frameCache]([[maybe_unused]] gfx::Drawable& drawable,
                                                  const PaintParameters& params,
                                                  Interface::GeometryOptions& opts) {
            if (frameCache->lastFrameCount != params.frameCount) {
                frameCache->lastFrameCount = params.frameCount;
                const Point<double>& center = projectLocation(loc, params.state);

                // Convert model units (meters) to world pixels for X/Y axes.
                // worldSize = 512 * 2^zoom; earthCircum at latitude in meters.
                // Z axis is kept in meters because the camera's worldToCamera matrix
                // already applies pixelsPerMeter scaling to Z (see Camera::getWorldToCamera).
                const double worldSize = 512.0 * params.state.getScale();
                frameCache->xyScale = static_cast<float>(worldSize / invariants.earthCircumAtLat) * modelScale;

                mat4 translation = matrix::identity4();
                matrix::translate(translation, translation, center.x, center.y, 0.0);
                matrix::multiply(frameCache->nearClippedProjectionTranslated,
                                 params.transformParams.nearClippedProjMatrix,
                                 translation);
            }

            mat4 m = matrix::identity4();
            matrix::scale(
                m, m, frameCache->xyScale * invariants.sx, frameCache->xyScale * invariants.sy, modelScale * invariants.sz);

            // User-specified rotations
            matrix::rotate_x(m, m, invariants.rotX);
            matrix::rotate_y(m, m, invariants.rotY);
            matrix::rotate_z(m, m, invariants.rotZ);

            matrix::multiply(opts.matrix, frameCache->nearClippedProjectionTranslated, m);
        });

    Interface::GeometryOptions options;
    if (model.texture) {
        auto image = std::make_shared<PremultipliedImage>(model.texture->clone<PremultipliedImage>());
        options.texture = interface.context.createTexture2D();
        options.texture->setImage(image);
        options.texture->setSamplerConfiguration({.filter = gfx::TextureFilterType::Linear,
                                                  .wrapU = gfx::TextureWrapType::Clamp,
                                                  .wrapV = gfx::TextureWrapType::Clamp});
    }
    interface.setGeometryOptions(options);

    for (const auto& mesh : model.meshes) {
        interface.addGeometry(mesh.vertices, mesh.indices, true);
    }

    interface.finish();
}

void GltfModelLayerHost::deinitialize() {
    model = {};
}

jni::jlong GltfModelLayerHost::createNativeHost(jni::JNIEnv& env,
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
                                                jni::jboolean mirrorZ) {
    auto host = std::make_unique<GltfModelLayerHost>(
        jni::Make<std::string>(env, path),
        LatLng(lat, lng),
        scale,
        std::array<float, 3>{rotX, rotY, rotZ},
        std::array<bool, 3>{static_cast<bool>(mirrorX), static_cast<bool>(mirrorY), static_cast<bool>(mirrorZ)});

    return reinterpret_cast<jni::jlong>(host.release());
}

jni::jlong GltfModelLayerHost::createNativeHostFromData(jni::JNIEnv& env,
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
                                                        jni::jboolean mirrorZ) {
    const auto length = data.Length(env);
    std::vector<jni::jbyte> temp(length);
    data.GetRegion<std::vector<jni::jbyte>>(env, 0, temp);
    std::vector<uint8_t> bytes(temp.begin(), temp.end());

    auto host = std::make_unique<GltfModelLayerHost>(
        std::move(bytes),
        LatLng(lat, lng),
        scale,
        std::array<float, 3>{rotX, rotY, rotZ},
        std::array<bool, 3>{static_cast<bool>(mirrorX), static_cast<bool>(mirrorY), static_cast<bool>(mirrorZ)});

    return reinterpret_cast<jni::jlong>(host.release());
}

void GltfModelLayerHost::registerNative(jni::JNIEnv& env) {
    static auto& javaClass = jni::Class<GltfModelLayerHost>::Singleton(env);
    jni::RegisterNatives(env,
                         *javaClass,
                         jni::MakeNativeMethod<decltype(&GltfModelLayerHost::createNativeHost),
                                               &GltfModelLayerHost::createNativeHost>("nativeCreateHost"),
                         jni::MakeNativeMethod<decltype(&GltfModelLayerHost::createNativeHostFromData),
                                               &GltfModelLayerHost::createNativeHostFromData>("nativeCreateHostFromData"));
}

} // namespace android
} // namespace mbgl