#include "coordinate_reference_system.hpp"
#include "attach_env.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>

namespace mln {
namespace android {

namespace {

class TileMatrixSet {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/CoordinateReferenceSystem$TileMatrixSet"; };
};

class Projection {
public:
    static constexpr auto Name() { return "org/maplibre/android/geometry/CoordinateReferenceSystem$Projection"; };
};

using Pair = std::array<double, 2>;
using Pairs = jni::Array<jni::jdouble>;
using Convert = jni::Method<Projection, void(Pairs, Pairs, jni::jint)>;
using GlobalProjection = jni::Global<jni::Object<Projection>, jni::EnvAttachingDeleter>;

static_assert(sizeof(Pair) == 2 * sizeof(jni::jdouble), "a span of pairs is the packed array the projection reads");

/// The most pairs one call to the Kotlin projection takes, so that the arrays it reads and writes stay small.
constexpr std::size_t pairsPerCall = 4096;

/// The arrays one thread hands the Kotlin projection, kept for its next call and grown to the largest call.
struct Arrays {
    jni::Global<Pairs, jni::EnvAttachingDeleter> in;
    jni::Global<Pairs, jni::EnvAttachingDeleter> out;
    std::size_t capacity = 0;

    void reserve(jni::JNIEnv& env, std::size_t pairs) {
        if (pairs <= capacity) {
            return;
        }
        const auto length = static_cast<jni::jsize>(2 * pairs);
        in = jni::NewGlobal<jni::EnvAttachingDeleter>(env, Pairs::New(env, length));
        out = jni::NewGlobal<jni::EnvAttachingDeleter>(env, Pairs::New(env, length));
        capacity = pairs;
    }
};

void convert(const GlobalProjection& projection,
             const Convert& method,
             std::span<const Pair> from,
             std::span<Pair> to) {
    if (from.empty()) {
        return;
    }
    thread_local Arrays arrays;
    const UniqueEnv env = AttachEnv();
    arrays.reserve(*env, std::min(from.size(), pairsPerCall));
    for (std::size_t first = 0; first < from.size(); first += pairsPerCall) {
        const std::size_t count = std::min(pairsPerCall, from.size() - first);
        const auto length = static_cast<jni::jsize>(2 * count);
        jni::SetArrayRegion(*env, *arrays.in.get(), 0, length, from[first].data());
        try {
            projection.Call(*env, method, arrays.in, arrays.out, static_cast<jni::jint>(count));
            jni::GetArrayRegion(*env, *arrays.out.get(), 0, length, to[first].data());
        } catch (const jni::PendingJavaException&) {
            jni::ExceptionClear(*env);
            constexpr double nan = std::numeric_limits<double>::quiet_NaN();
            std::fill_n(to.begin() + static_cast<std::ptrdiff_t>(first), count, Pair{nan, nan});
        }
    }
}

} // namespace

mln::CRS CoordinateReferenceSystem::getCRS(jni::JNIEnv& env, const jni::Object<CoordinateReferenceSystem>& crs) {
    static auto& javaClass = jni::Class<CoordinateReferenceSystem>::Singleton(env);
    static auto nameField = javaClass.GetField<jni::String>(env, "name");
    static auto tileMatrixSetField = javaClass.GetField<jni::Object<TileMatrixSet>>(env, "tileMatrixSet");
    static auto projectionField = javaClass.GetField<jni::Object<Projection>>(env, "projection");
    static auto& tileMatrixSetClass = jni::Class<TileMatrixSet>::Singleton(env);
    static auto originXField = tileMatrixSetClass.GetField<jni::jdouble>(env, "originX");
    static auto originYField = tileMatrixSetClass.GetField<jni::jdouble>(env, "originY");
    static auto extentAtZoom0Field = tileMatrixSetClass.GetField<jni::jdouble>(env, "extentAtZoom0");
    static auto& projectionClass = jni::Class<Projection>::Singleton(env);
    static const Convert forward = projectionClass.GetMethod<void(Pairs, Pairs, jni::jint)>(env, "forward");
    static const Convert inverse = projectionClass.GetMethod<void(Pairs, Pairs, jni::jint)>(env, "inverse");

    const auto tileMatrixSet = crs.Get(env, tileMatrixSetField);
    auto projection = std::make_shared<GlobalProjection>(
        jni::NewGlobal<jni::EnvAttachingDeleter>(env, crs.Get(env, projectionField)));
    return {
        .name = jni::Make<std::string>(env, crs.Get(env, nameField)),
        .forward = [projection](std::span<const Pair> lngLats,
                                std::span<Pair> positions) { convert(*projection, forward, lngLats, positions); },
        .inverse = [projection](std::span<const Pair> positions,
                                std::span<Pair> lngLats) { convert(*projection, inverse, positions, lngLats); },
        .tileMatrixSet = {.origin = {tileMatrixSet.Get(env, originXField), tileMatrixSet.Get(env, originYField)},
                          .extentAtZoom0 = tileMatrixSet.Get(env, extentAtZoom0Field)},
    };
}

void CoordinateReferenceSystem::registerNative(jni::JNIEnv& env) {
    jni::Class<CoordinateReferenceSystem>::Singleton(env);
    jni::Class<TileMatrixSet>::Singleton(env);
    jni::Class<Projection>::Singleton(env);
}

} // namespace android
} // namespace mln
