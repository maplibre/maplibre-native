#pragma once

#include <mln/map/map.hpp>
#include <mln/storage/file_source.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/util/async_request.hpp>
#include <mln/util/client_options.hpp>
#include <mln/util/image.hpp>

#include <exception>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#include <nan.h>
#pragma GCC diagnostic pop

namespace mln {
class HeadlessFrontend;
} // namespace mln

namespace node_mbgl {

struct NodeMapObserver : public mln::MapObserver {
    void onDidFailLoadingMap(mln::MapLoadError, const std::string&) final;
};

class RenderRequest;

class NodeMap : public Nan::ObjectWrap {
public:
    struct RenderOptions;
    class RenderWorker;

    NodeMap(v8::Local<v8::Object>);
    ~NodeMap() override;

    static Nan::Persistent<v8::Function> constructor;
    static Nan::Persistent<v8::Object> parseError;

    static void Init(v8::Local<v8::Object>);

    static void New(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void Load(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void Loaded(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void Render(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void Release(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void Cancel(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void AddSource(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void RemoveSource(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void AddLayer(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void RemoveLayer(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void AddImage(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void RemoveImage(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetLayerZoomRange(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetLayerProperty(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetFilter(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetSize(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetCenter(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetZoom(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetBearing(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetPitch(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetLight(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetAxonometric(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetXSkew(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void SetYSkew(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void DumpDebugLogs(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void QueryRenderedFeatures(const Nan::FunctionCallbackInfo<v8::Value>&);

    static void SetFeatureState(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void GetFeatureState(const Nan::FunctionCallbackInfo<v8::Value>&);
    static void RemoveFeatureState(const Nan::FunctionCallbackInfo<v8::Value>&);

    static v8::Local<v8::Value> ParseError(const char* msg);

    void startRender();
    void startRender(const RenderOptions& options);
    void renderFinished();

    void release();
    void cancel();

    static RenderOptions ParseOptions(v8::Local<v8::Object>);

    const float pixelRatio;
    mln::MapMode mode;
    bool crossSourceCollisions;
    NodeMapObserver mapObserver;
    std::unique_ptr<mln::HeadlessFrontend> frontend;
    std::unique_ptr<mln::Map> map;

    std::exception_ptr error;
    mln::PremultipliedImage image;
    std::unique_ptr<RenderRequest> req;

    // Async for delivering the notifications of render completion.
    uv_async_t* async;

    bool loaded = false;
};

struct NodeFileSource : public mln::FileSource {
    NodeFileSource(NodeMap* nodeMap_)
        : nodeMap(nodeMap_) {}
    ~NodeFileSource() override = default;
    std::unique_ptr<mln::AsyncRequest> request(const mln::Resource&, mln::FileSource::Callback) final;
    bool canRequest(const mln::Resource&) const override;
    void setResourceOptions(mln::ResourceOptions) override;
    mln::ResourceOptions getResourceOptions() override;
    void setClientOptions(mln::ClientOptions) override;
    mln::ClientOptions getClientOptions() override;
    NodeMap* nodeMap;
    mln::ResourceOptions _resourceOptions;
    mln::ClientOptions _clientOptions;
};

} // namespace node_mbgl
