#pragma once

#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/client_options.hpp>

#include <algorithm>
#include <list>

namespace mbgl {

/*
   FakeFileSource is similar to StubFileSource, but it follows a post hoc,
   "push" model rather than a pre hoc, "pull" model. You pass it to code that
   makes requests, it records what requests are made, then you can examine them,
   make assertions about them, and respond as desired.

   This is particularly useful if you want to simulate multiple responses, e.g.
   as part of a resource revalidation flow. StubFileSource allows only a single
   response.
*/
class FakeFileSource : public FileSource {
public:
    class FakeFileRequest : public AsyncRequest {
    public:
        Resource resource;
        std::function<void(Response)> callback;

        std::list<FakeFileRequest*>& list;
        std::list<FakeFileRequest*>::iterator link;

        FakeFileRequest(Resource resource_, std::function<void(Response)> callback_, std::list<FakeFileRequest*>& list_)
            : resource(std::move(resource_)),
              callback(std::move(callback_)),
              list(list_),
              link((list.push_back(this), std::prev(list.end()))) {}

        ~FakeFileRequest() override { list.erase(link); }
    };

    FakeFileSource(const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}
    FakeFileSource()
        : FakeFileSource(ResourceOptions::Default(), ClientOptions()) {}

    std::unique_ptr<AsyncRequest> request(const Resource& resource, std::function<void(Response)> callback) override {
        return std::make_unique<FakeFileRequest>(resource, std::move(callback), requests);
    }

    bool canRequest(const Resource&) const override { return true; }

    bool respond(Resource::Kind kind, const Response& response) {
        auto it = std::find_if(requests.begin(), requests.end(), [&](FakeFileRequest* fakeRequest) {
            return fakeRequest->resource.kind == kind;
        });

        const bool requestFound = (it != requests.end());

        if (requestFound) {
            // Copy the callback, in case calling it deallocates the AsyncRequest.
            auto callback_ = (*it)->callback;
            callback_(response);
        }

        return requestFound;
    }

    std::list<FakeFileRequest*> requests;

    void setResourceOptions(ResourceOptions options) override { resourceOptions = options; }
    ResourceOptions getResourceOptions() override { return resourceOptions.clone(); }

    void setClientOptions(ClientOptions options) override { clientOptions = options; }
    ClientOptions getClientOptions() override { return clientOptions.clone(); }

private:
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

class FakeOnlineFileSource : public FakeFileSource {
public:
    FakeOnlineFileSource()
        : FakeOnlineFileSource(ResourceOptions::Default(), ClientOptions()) {}
    FakeOnlineFileSource(const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : FakeFileSource(resourceOptions_, clientOptions_) {}

    std::unique_ptr<AsyncRequest> request(const Resource& resource, std::function<void(Response)> callback) override {
        return FakeFileSource::request(resource, std::move(callback));
    }

    bool respond(Resource::Kind kind, const Response& response) { return FakeFileSource::respond(kind, response); }

    mapbox::base::Value getProperty(const std::string& property) const override {
        return onlineFs->getProperty(property);
    }

    std::unique_ptr<FileSource> onlineFs = std::make_unique<OnlineFileSource>(ResourceOptions::Default(),
                                                                              ClientOptions());
};

} // namespace mbgl
