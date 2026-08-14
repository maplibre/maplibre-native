#include "rendering_stats.hpp"

#include "java/util.hpp"

#include <mbgl/gfx/rendering_stats.hpp>
#include <mbgl/util/containers.hpp>

namespace mln::android {

using namespace jni;
using namespace java::util;

namespace {

struct SourceLayerIDTag {
    static constexpr auto Name() { return "org/maplibre/android/maps/RenderingStats$SourceLayerID"; }
    static auto New(JNIEnv& env, const gfx::RenderingStats::SourceLayerID& id) {
        const auto& javaClass = Class<SourceLayerIDTag>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<String, String>(env);
        return javaClass.New(env, constructor, Make<String>(env, id.sourceID), Make<String>(env, id.layerID));
    }
};

struct NDCBoundTag {
    static constexpr auto Name() { return "org/maplibre/android/maps/RenderingStats$NDCBound"; }
    static auto New(JNIEnv& env, const gfx::RenderingStats::NDCBound& bound) {
        const auto& javaClass = Class<NDCBoundTag>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<double, double, double, double>(env);
        return javaClass.New(env, constructor, bound.minX, bound.maxX, bound.minY, bound.maxY);
    }
};

struct TileIDTag {
    static constexpr auto Name() { return "org/maplibre/android/maps/RenderingStats$TileID"; }
    static auto New(JNIEnv& env, const OverscaledTileID& id) {
        const auto& javaClass = Class<TileIDTag>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<int, int, int, int, int>(env);
        return javaClass.New(env,
                             constructor,
                             id.canonical.z,
                             static_cast<int>(id.canonical.x),
                             static_cast<int>(id.canonical.y),
                             id.overscaledZ,
                             id.wrap);
    }
};

struct FeatureInfoTag {
    static constexpr auto Name() { return "org/maplibre/android/maps/RenderingStats$FeatureInfo"; }
    static auto New(JNIEnv& env, const std::string& featureID, const gfx::RenderingStats::FeatureInfo& info) {
        const auto& javaClass = Class<FeatureInfoTag>::Singleton(env);
        static const auto constructor = javaClass.GetConstructor<String, Object<NDCBoundTag>, Object<Set>>(env);
        return javaClass.New(env,
                             constructor,
                             Make<String>(env, featureID),
                             NDCBoundTag::New(env, info.ndcBound),
                             makeTileSet(env, info.tileIDs));
    }

protected:
    static auto makeTileSet(JNIEnv& env, const mln::unordered_set<OverscaledTileID>& tileIDs) -> Local<Object<Set>> {
        const auto tileIDsSetObj = HashSet::New(env, static_cast<jint>(tileIDs.size()));
        for (const auto& tileID : tileIDs) {
            HashSet::add(env, tileIDsSetObj, TileIDTag::New(env, tileID));
        }
        return Cast(env, Class<Set>::Singleton(env), tileIDsSetObj);
    }
};

static auto makeFeatureList(JNIEnv& env, const gfx::RenderingStats::LayerFeaturesMap& features) {
    auto featureListObj = ArrayList::New(env, static_cast<jint>(features.size()));
    for (const auto& [featureKey, featureInfo] : features) {
        ArrayList::add(env, featureListObj, FeatureInfoTag::New(env, featureKey, featureInfo));
    }
    return featureListObj;
}

static auto makeFeatureInfoMap(JNIEnv& env, const gfx::RenderingStats::FrameRenderedFeaturesMap& featureInfoMap) {
    const auto result = HashMap::New(env, static_cast<jint>(featureInfoMap.size()));
    for (const auto& [sourceLayerID, layerFeatures] : featureInfoMap) {
        HashMap::put(env, result, SourceLayerIDTag::New(env, sourceLayerID), makeFeatureList(env, layerFeatures));
    }
    return Cast(env, Class<Map>::Singleton(env), result);
}

} // namespace

void RenderingStats::registerNative(JNIEnv& env) {
    Class<RenderingStats>::Singleton(env);
    Class<SourceLayerIDTag>::Singleton(env);
    Class<NDCBoundTag>::Singleton(env);
    Class<TileIDTag>::Singleton(env);
    Class<FeatureInfoTag>::Singleton(env);
}

Local<Object<RenderingStats>> RenderingStats::Create(JNIEnv& env) {
    const auto& javaClass = Class<RenderingStats>::Singleton(env);
    static const auto constructor = javaClass.GetConstructor(env);
    return javaClass.New(env, constructor);
}

void RenderingStats::Update(JNIEnv& env, Object<RenderingStats>& renderingStatsObj, const gfx::RenderingStats& stats) {
    const auto& renderingStatsClass = Class<RenderingStats>::Singleton(env);

#define SetField(name, type)                                                        \
    static const auto name##Field = renderingStatsClass.GetField<type>(env, #name); \
    renderingStatsObj.Set(env, name##Field, static_cast<type>(stats.name))

    SetField(encodingTime, jdouble);
    SetField(renderingTime, jdouble);
    SetField(numFrames, jint);
    SetField(numDrawCalls, jint);
    SetField(totalDrawCalls, jint);
    SetField(numCreatedTextures, jint);
    SetField(numActiveTextures, jint);
    SetField(numTextureBindings, jint);
    SetField(numTextureUpdates, jint);
    SetField(textureUpdateBytes, jlong);
    SetField(totalBuffers, jlong);
    SetField(totalBufferObjs, jlong);
    SetField(bufferUpdates, jlong);
    SetField(bufferObjUpdates, jlong);
    SetField(bufferUpdateBytes, jlong);
    SetField(numBuffers, jint);
    SetField(numFrameBuffers, jint);
    SetField(numIndexBuffers, jint);
    SetField(indexUpdateBytes, jlong);
    SetField(numVertexBuffers, jint);
    SetField(vertexUpdateBytes, jlong);
    SetField(numUniformBuffers, jint);
    SetField(numUniformUpdates, jint);
    SetField(uniformUpdateBytes, jlong);
    SetField(memTextures, jint);
    SetField(memBuffers, jint);
    SetField(memIndexBuffers, jint);
    SetField(memVertexBuffers, jint);
    SetField(memUniformBuffers, jint);
    SetField(stencilClears, jint);
    SetField(stencilUpdates, jint);
#undef SetField

    renderingStatsObj.Set(env,
                          renderingStatsClass.GetField<Object<Map>>(env, "renderedFeatures"),
                          makeFeatureInfoMap(env, stats.frameRenderedFeatures));
}

} // namespace mln::android
