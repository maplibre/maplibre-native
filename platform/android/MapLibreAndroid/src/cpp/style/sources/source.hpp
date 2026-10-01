#pragma once

#include <mln/util/noncopyable.hpp>
#include <mln/map/map.hpp>
#include <mln/style/source.hpp>

#include "../value.hpp"
#include "../../android_renderer_frontend.hpp"
#include "../../gson/json_object.hpp"

#include <jni/jni.hpp>

namespace mln {
namespace android {

class Source : private mln::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/sources/Source"; };

    static void registerNative(jni::JNIEnv&);

    static const jni::Object<Source>& peerForCoreSource(jni::JNIEnv&, mln::style::Source&);
    static const jni::Object<Source>& peerForCoreSource(jni::JNIEnv&,
                                                        mln::style::Source&,
                                                        AndroidRendererFrontend&,
                                                        mln::Map&);

    /*
     * Called when a Java object is created for a core source that belongs to a map.
     */
    Source(jni::JNIEnv&, mln::style::Source&, const jni::Object<Source>&, AndroidRendererFrontend*);

    /*
     * Called when a Java object is created for a new core source that does not
     * belong to a map.
     */
    Source(jni::JNIEnv&, std::unique_ptr<mln::style::Source>);

    virtual ~Source();

    virtual void addToMap(JNIEnv&, const jni::Object<Source>&, mln::Map&, AndroidRendererFrontend&);

    virtual bool removeFromMap(JNIEnv&, const jni::Object<Source>&, mln::Map&);

    void releaseJavaPeer();

    jni::Local<jni::String> getId(jni::JNIEnv&);

    jni::Local<jni::String> getAttribution(jni::JNIEnv&);

    void setPrefetchZoomDelta(jni::JNIEnv& env, jni::Integer& delta);

    jni::Local<jni::Integer> getPrefetchZoomDelta(jni::JNIEnv&);

    void setMaxOverscaleFactorForParentTiles(jni::JNIEnv& env, jni::Integer& delta);

    jni::Local<jni::Integer> getMaxOverscaleFactorForParentTiles(jni::JNIEnv&);

    void addToStyle(JNIEnv& env, const jni::Object<Source>& obj, mln::style::Style& style);

    jni::Local<jni::Boolean> isVolatile(JNIEnv&);

    void setVolatile(JNIEnv&, jni::Boolean&);

    void setMinimumTileUpdateInterval(JNIEnv&, jni::Long&);

    jni::Local<jni::Long> getMinimumTileUpdateInterval(JNIEnv&);

    jni::jboolean setFeatureState(JNIEnv&,
                                  const jni::String& sourceLayerId,
                                  const jni::String& featureId,
                                  const jni::Object<gson::JsonObject>& state);

    jni::Local<jni::Object<gson::JsonObject>> getFeatureState(JNIEnv&,
                                                              const jni::String& sourceLayerId,
                                                              const jni::String& featureId);

    jni::jboolean removeFeatureState(JNIEnv&,
                                     const jni::String& sourceLayerId,
                                     const jni::String& featureId,
                                     const jni::String& stateKey);

    void bindToMap(AndroidRendererFrontend&, mln::Map&);

protected:
    // Set on newly created sources until added to the map.
    std::unique_ptr<mln::style::Source> ownedSource;

    // Raw pointer that is valid at all times.
    mln::style::Source& source;

    // Set when the source is added to a map.
    jni::Global<jni::Object<Source>> javaPeer;

    // RendererFrontend pointer is valid only when added to the map.
    AndroidRendererFrontend* rendererFrontend{nullptr};

    // Map pointer is valid only when added to the map.
    mln::Map* map{nullptr};
};

} // namespace android
} // namespace mln
