#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/style/source.hpp>

#include "../value.hpp"
#include "../../android_renderer_frontend.hpp"

#include <jni/jni.hpp>
#include <functional>

namespace mbgl {
namespace android {

class Source : private mbgl::util::noncopyable {
public:
    static constexpr auto Name() { return "org/maplibre/android/style/sources/Source"; };

    static void registerNative(jni::JNIEnv&);

    static const jni::Object<Source>& peerForCoreSource(jni::JNIEnv&, mbgl::style::Source&);
    static const jni::Object<Source>& peerForCoreSource(jni::JNIEnv&, mbgl::style::Source&, AndroidRendererFrontend&);

    /*
     * Called when a Java object is created for a core source that belongs to a map.
     */
    Source(jni::JNIEnv&, mbgl::style::Source&, const jni::Object<Source>&, AndroidRendererFrontend*);

    /*
     * Called when a Java object is created for a new core source that does not
     * belong to a map.
     */
    Source(jni::JNIEnv&, std::unique_ptr<mbgl::style::Source>);

    virtual ~Source();

    virtual void addToMap(JNIEnv&, const jni::Object<Source>&, mbgl::Map&, AndroidRendererFrontend&);

    virtual bool removeFromMap(JNIEnv&, const jni::Object<Source>&, mbgl::Map&);

    void releaseJavaPeer();

    jni::Local<jni::String> getId(jni::JNIEnv&);

    jni::Local<jni::String> getAttribution(jni::JNIEnv&);

    void setPrefetchZoomDelta(jni::JNIEnv& env, jni::Integer& delta);

    jni::Local<jni::Integer> getPrefetchZoomDelta(jni::JNIEnv&);

    void setMaxOverscaleFactorForParentTiles(jni::JNIEnv& env, jni::Integer& delta);

    jni::Local<jni::Integer> getMaxOverscaleFactorForParentTiles(jni::JNIEnv&);

    void addToStyle(JNIEnv& env, const jni::Object<Source>& obj, mbgl::style::Style& style);

    jni::Local<jni::Boolean> isVolatile(JNIEnv&);

    void setVolatile(JNIEnv&, jni::Boolean&);

    void setMinimumTileUpdateInterval(JNIEnv&, jni::Long&);

    jni::Local<jni::Long> getMinimumTileUpdateInterval(JNIEnv&);

protected:
    template <typename T = style::Source>
    T& getSource() { return *ownedSource->as<T>(); }

    std::weak_ptr<std::reference_wrapper<style::Source>> getWeakSource() const { return source; }

private:
    // Set on newly created sources until added to the map.
    std::unique_ptr<mbgl::style::Source> ownedSource;

    // Reference that is valid at all times.
    std::shared_ptr<std::reference_wrapper<mbgl::style::Source>> source;

protected:
    // Set when the source is added to a map.
    jni::Global<jni::Object<Source>> javaPeer;

    // RendererFrontend pointer is valid only when added to the map.
    AndroidRendererFrontend* rendererFrontend{nullptr};
};

} // namespace android
} // namespace mbgl
