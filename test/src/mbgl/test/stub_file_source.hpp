#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/timer.hpp>

#include <map>
#include <unordered_map>

namespace mbgl {

class StubFileSource : public FileSource {
public:
    enum class ResponseType {
        Asynchronous = 0,
        Synchronous
    };

    StubFileSource(const ResourceOptions&, const ClientOptions&, ResponseType = ResponseType::Asynchronous);
    StubFileSource(ResponseType = ResponseType::Asynchronous);
    ~StubFileSource() override;

    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;
    bool canRequest(const Resource&) const override { return true; }
    void remove(AsyncRequest*);
    void setProperty(const std::string&, const mapbox::base::Value&) override;
    mapbox::base::Value getProperty(const std::string&) const override;

    using ResponseFunction = std::function<std::optional<Response>(const Resource&)>;

    // You can set the response callback on a global level by assigning this callback:
    ResponseFunction response = [this](const Resource& resource) {
        return defaultResponse(resource);
    };

    // Or set per-kind responses by setting these callbacks:
    ResponseFunction styleResponse;
    ResponseFunction sourceResponse;
    ResponseFunction tileResponse;
    ResponseFunction glyphsResponse;
    ResponseFunction spriteJSONResponse;
    ResponseFunction spriteImageResponse;
    ResponseFunction imageResponse;

    void setResourceOptions(ResourceOptions options) override;
    ResourceOptions getResourceOptions() override;

    void setClientOptions(ClientOptions options) override;
    ClientOptions getClientOptions() override;

private:
    friend class StubOnlineFileSource;

    // The default behavior is to throw if no per-kind callback has been set.
    std::optional<Response> defaultResponse(const Resource&);

    std::unordered_map<AsyncRequest*, std::tuple<Resource, ResponseFunction, Callback>> pending;
    ResponseType type;
    util::Timer timer;
    std::map<std::string, mapbox::base::Value> properties;

    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

} // namespace mbgl
