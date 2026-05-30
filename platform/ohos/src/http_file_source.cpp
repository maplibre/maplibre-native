#include "http_user_agent.hpp"

#include <mbgl/storage/http_file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/http_header.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/util.hpp>

#include <network/netstack/net_http.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace mbgl {
namespace {

constexpr std::size_t MaxConcurrentRequests = 128;

class RequestState;

void dispatchResponse(std::size_t slot, Http_Response* response, uint32_t errCode);
void dispatchData(std::size_t slot, const char* data, std::size_t length);
void dispatchCanceled(std::size_t slot);

template <std::size_t Slot>
void onResponse(Http_Response* response, uint32_t errCode) {
    dispatchResponse(Slot, response, errCode);
}

template <std::size_t Slot>
void onDataReceive(const char* data, std::size_t length) {
    dispatchData(Slot, data, length);
}

template <std::size_t Slot>
void onCanceled() {
    dispatchCanceled(Slot);
}

template <std::size_t... Slots>
constexpr auto makeResponseCallbacks(std::index_sequence<Slots...>) {
    return std::array<Http_ResponseCallback, sizeof...(Slots)>{&onResponse<Slots>...};
}

template <std::size_t... Slots>
constexpr auto makeDataCallbacks(std::index_sequence<Slots...>) {
    return std::array<Http_OnDataReceiveCallback, sizeof...(Slots)>{&onDataReceive<Slots>...};
}

template <std::size_t... Slots>
constexpr auto makeCanceledCallbacks(std::index_sequence<Slots...>) {
    return std::array<Http_OnVoidCallback, sizeof...(Slots)>{&onCanceled<Slots>...};
}

constexpr auto responseCallbacks = makeResponseCallbacks(std::make_index_sequence<MaxConcurrentRequests>{});
constexpr auto dataCallbacks = makeDataCallbacks(std::make_index_sequence<MaxConcurrentRequests>{});
constexpr auto canceledCallbacks = makeCanceledCallbacks(std::make_index_sequence<MaxConcurrentRequests>{});

std::string toLowerASCII(const char* value) {
    std::string result = value ? value : "";
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::optional<std::string> headerValue(Http_Headers* headers, const char* name) {
    if (!headers) {
        return std::nullopt;
    }

    const auto expected = toLowerASCII(name);
    Http_HeaderEntry* entries = OH_Http_GetHeaderEntries(headers);
    if (!entries) {
        return std::nullopt;
    }

    for (auto* entry = entries; entry; entry = entry->next) {
        if (toLowerASCII(entry->key) != expected || !entry->value) {
            continue;
        }

        std::string value;
        for (auto* item = entry->value; item; item = item->next) {
            if (!item->value) {
                continue;
            }
            if (!value.empty()) {
                value += ", ";
            }
            value += item->value;
        }

        OH_Http_DestroyHeaderEntries(&entries);
        return value;
    }

    OH_Http_DestroyHeaderEntries(&entries);
    return std::nullopt;
}

Response::Error::Reason errorReason(uint32_t errCode) {
    switch (errCode) {
        case OH_HTTP_RESOLVE_PROXY_FAILED:
        case OH_HTTP_RESOLVE_HOST_FAILED:
        case OH_HTTP_CONNECT_SERVER_FAILED:
        case OH_HTTP_OPERATION_TIMEOUT:
        case OH_HTTP_RECEIVE_DATA_FAILED:
        case OH_HTTP_SSL_CERTIFICATE_ERROR:
        case OH_HTTP_INVALID_SSL_PEER_CERT:
        case OH_HTTP_SSL_CA_NOT_EXIST:
            return Response::Error::Reason::Connection;
        default:
            return Response::Error::Reason::Other;
    }
}

const char* errorCodeName(uint32_t errCode) {
    switch (errCode) {
        case OH_HTTP_RESULT_OK:
            return "OH_HTTP_RESULT_OK";
        case OH_HTTP_PARAMETER_ERROR:
            return "OH_HTTP_PARAMETER_ERROR";
        case OH_HTTP_PERMISSION_DENIED:
            return "OH_HTTP_PERMISSION_DENIED";
        case OH_HTTP_UNSUPPORTED_PROTOCOL:
            return "OH_HTTP_UNSUPPORTED_PROTOCOL";
        case OH_HTTP_INVALID_URL:
            return "OH_HTTP_INVALID_URL";
        case OH_HTTP_RESOLVE_PROXY_FAILED:
            return "OH_HTTP_RESOLVE_PROXY_FAILED";
        case OH_HTTP_RESOLVE_HOST_FAILED:
            return "OH_HTTP_RESOLVE_HOST_FAILED";
        case OH_HTTP_CONNECT_SERVER_FAILED:
            return "OH_HTTP_CONNECT_SERVER_FAILED";
        case OH_HTTP_INVALID_SERVER_RESPONSE:
            return "OH_HTTP_INVALID_SERVER_RESPONSE";
        case OH_HTTP_ACCESS_REMOTE_DENIED:
            return "OH_HTTP_ACCESS_REMOTE_DENIED";
        case OH_HTTP_HTTP2_FRAMING_ERROR:
            return "OH_HTTP_HTTP2_FRAMING_ERROR";
        case OH_HTTP_TRANSFER_PARTIAL_FILE:
            return "OH_HTTP_TRANSFER_PARTIAL_FILE";
        case OH_HTTP_WRITE_DATA_FAILED:
            return "OH_HTTP_WRITE_DATA_FAILED";
        case OH_HTTP_UPLOAD_FAILED:
            return "OH_HTTP_UPLOAD_FAILED";
        case OH_HTTP_OPEN_LOCAL_DATA_FAILED:
            return "OH_HTTP_OPEN_LOCAL_DATA_FAILED";
        case OH_HTTP_OUT_OF_MEMORY:
            return "OH_HTTP_OUT_OF_MEMORY";
        case OH_HTTP_OPERATION_TIMEOUT:
            return "OH_HTTP_OPERATION_TIMEOUT";
        case OH_HTTP_TOO_MANY_REDIRECTIONS:
            return "OH_HTTP_TOO_MANY_REDIRECTIONS";
        case OH_HTTP_SERVER_RETURNED_NOTHING:
            return "OH_HTTP_SERVER_RETURNED_NOTHING";
        case OH_HTTP_SEND_DATA_FAILED:
            return "OH_HTTP_SEND_DATA_FAILED";
        case OH_HTTP_RECEIVE_DATA_FAILED:
            return "OH_HTTP_RECEIVE_DATA_FAILED";
        case OH_HTTP_SSL_CERTIFICATE_ERROR:
            return "OH_HTTP_SSL_CERTIFICATE_ERROR";
        case OH_HTTP_SSL_CIPHER_USED_ERROR:
            return "OH_HTTP_SSL_CIPHER_USED_ERROR";
        case OH_HTTP_INVALID_SSL_PEER_CERT:
            return "OH_HTTP_INVALID_SSL_PEER_CERT";
        case OH_HTTP_INVALID_ENCODING_FORMAT:
            return "OH_HTTP_INVALID_ENCODING_FORMAT";
        case OH_HTTP_FILE_TOO_LARGE:
            return "OH_HTTP_FILE_TOO_LARGE";
        case OH_HTTP_REMOTE_DISK_FULL:
            return "OH_HTTP_REMOTE_DISK_FULL";
        case OH_HTTP_REMOTE_FILE_EXISTS:
            return "OH_HTTP_REMOTE_FILE_EXISTS";
        case OH_HTTP_SSL_CA_NOT_EXIST:
            return "OH_HTTP_SSL_CA_NOT_EXIST";
        case OH_HTTP_REMOTE_FILE_NOT_FOUND:
            return "OH_HTTP_REMOTE_FILE_NOT_FOUND";
        case OH_HTTP_AUTHENTICATION_ERROR:
            return "OH_HTTP_AUTHENTICATION_ERROR";
        case OH_HTTP_ACCESS_DOMAIN_NOT_ALLOWED:
            return "OH_HTTP_ACCESS_DOMAIN_NOT_ALLOWED";
        case OH_HTTP_UNKNOWN_ERROR:
            return "OH_HTTP_UNKNOWN_ERROR";
        default:
            return nullptr;
    }
}

std::string errorMessage(const Resource& resource, uint32_t errCode) {
    std::string message{"OpenHarmony HTTP request failed with "};
    if (const char* name = errorCodeName(errCode)) {
        message += name;
        message += " (";
        message += util::toString(errCode);
        message += ")";
    } else {
        message += "error ";
        message += util::toString(errCode);
    }
    if (!resource.url.empty()) {
        message += " for ";
        message += resource.url;
    }
    return message;
}

Response makeNetworkError(const Resource& resource, uint32_t errCode) {
    Response response;
    response.error = std::make_unique<Response::Error>(errorReason(errCode), errorMessage(resource, errCode));
    return response;
}

void applyCacheHeaders(Response& response, Http_Headers* headers) {
    response.etag = headerValue(headers, "etag");
    if (const auto modified = headerValue(headers, "last-modified")) {
        response.modified = util::parseTimestamp(modified->c_str());
    }
    if (const auto expires = headerValue(headers, "expires")) {
        response.expires = util::parseTimestamp(expires->c_str());
    }
    if (const auto cacheControl = headerValue(headers, "cache-control")) {
        const auto cc = http::CacheControl::parse(*cacheControl);
        if (cc.maxAge) {
            response.expires = cc.toTimePoint();
        }
        response.mustRevalidate = cc.mustRevalidate;
    }
}

Response makeResponse(const Resource& resource,
                      Http_Response* httpResponse,
                      uint32_t errCode,
                      std::shared_ptr<std::string> streamedData) {
    if (errCode != OH_HTTP_RESULT_OK) {
        return makeNetworkError(resource, errCode);
    }

    Response response;
    if (!httpResponse) {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other,
                                                           "OpenHarmony HTTP returned no response");
        return response;
    }

    applyCacheHeaders(response, httpResponse->headers);

    const int code = static_cast<int>(httpResponse->responseCode);
    if (code == 200 || code == 206) {
        if (httpResponse->body.buffer && httpResponse->body.length > 0) {
            response.data = std::make_shared<std::string>(httpResponse->body.buffer, httpResponse->body.length);
        } else if (streamedData) {
            response.data = std::move(streamedData);
        } else {
            response.data = std::make_shared<std::string>();
        }
    } else if (code == 204 || (code == 404 && resource.kind == Resource::Kind::Tile)) {
        response.noContent = true;
    } else if (code == 304) {
        response.notModified = true;
    } else if (code == 404) {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound, "HTTP status code 404");
    } else if (code == 429) {
        response.error = std::make_unique<Response::Error>(
            Response::Error::Reason::RateLimit,
            "HTTP status code 429",
            http::parseRetryHeaders(headerValue(httpResponse->headers, "retry-after"),
                                    headerValue(httpResponse->headers, "x-rate-limit-reset")));
    } else if (code >= 500 && code < 600) {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Server,
                                                           std::string{"HTTP status code "} + util::toString(code));
    } else {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other,
                                                           std::string{"HTTP status code "} + util::toString(code));
    }

    return response;
}

class CallbackSlots {
public:
    // net_http callbacks have no user-data parameter, so each request receives
    // a stable callback function set indexed by this slot table.
    static std::optional<std::size_t> acquire(const std::shared_ptr<RequestState>& state) {
        std::scoped_lock lock(mutex);
        for (std::size_t i = 0; i < slots.size(); ++i) {
            if (slots[i].expired()) {
                slots[i] = state;
                return i;
            }
        }
        return std::nullopt;
    }

    static std::shared_ptr<RequestState> get(std::size_t slot) {
        std::scoped_lock lock(mutex);
        if (slot >= slots.size()) {
            return nullptr;
        }
        return slots[slot].lock();
    }

    static void release(std::size_t slot) {
        std::scoped_lock lock(mutex);
        if (slot < slots.size()) {
            slots[slot].reset();
        }
    }

private:
    static std::mutex mutex;
    static std::array<std::weak_ptr<RequestState>, MaxConcurrentRequests> slots;
};

std::mutex CallbackSlots::mutex;
std::array<std::weak_ptr<RequestState>, MaxConcurrentRequests> CallbackSlots::slots;

class RequestState : public std::enable_shared_from_this<RequestState> {
public:
    RequestState(util::RunLoop& runLoop_, Resource resource_, FileSource::Callback callback_)
        : runLoop(runLoop_),
          resource(std::move(resource_)),
          callback(std::move(callback_)) {}

    void keepAlive() { self = shared_from_this(); }

    bool setSlot(std::size_t slot_) {
        std::scoped_lock lock(mutex);
        if (slot) {
            return false;
        }
        slot = slot_;
        return true;
    }

    void setRequest(Http_Request* request_) {
        std::scoped_lock lock(mutex);
        request = request_;
    }

    const Resource& getResource() const { return resource; }

    void appendData(const char* data, std::size_t length) {
        if (!data || length == 0) {
            return;
        }

        std::scoped_lock lock(mutex);
        if (finished || canceled) {
            return;
        }
        if (!streamedData) {
            streamedData = std::make_shared<std::string>();
        }
        streamedData->append(data, length);
    }

    void cancel() {
        Http_Request* requestToDestroy = nullptr;
        {
            std::scoped_lock lock(mutex);
            if (finished) {
                return;
            }
            canceled = true;
            finished = true;
            callback = nullptr;
            requestToDestroy = takeRequestLocked();
        }

        destroyRequest(requestToDestroy);
        release();
    }

    void complete(Http_Response* httpResponse, uint32_t errCode) {
        std::shared_ptr<std::string> localStreamedData;
        {
            std::scoped_lock lock(mutex);
            localStreamedData = streamedData;
        }

        Response response = makeResponse(resource, httpResponse, errCode, std::move(localStreamedData));
        if (httpResponse && httpResponse->destroyResponse) {
            Http_Response* responseToDestroy = httpResponse;
            httpResponse->destroyResponse(&responseToDestroy);
        }

        runLoop.invoke([state = shared_from_this(), response = std::move(response)]() mutable {
            FileSource::Callback localCallback;
            Http_Request* requestToDestroy = nullptr;
            {
                std::scoped_lock lock(state->mutex);
                if (state->finished) {
                    return;
                }
                state->finished = true;
                requestToDestroy = state->takeRequestLocked();
                if (!state->canceled) {
                    localCallback = std::move(state->callback);
                }
                state->callback = nullptr;
            }

            state->destroyRequest(requestToDestroy);
            state->release();

            if (localCallback) {
                localCallback(response);
            }
        });
    }

    void handleCanceled() {
        Http_Request* requestToDestroy = nullptr;
        {
            std::scoped_lock lock(mutex);
            if (finished) {
                return;
            }
            canceled = true;
            finished = true;
            callback = nullptr;
            requestToDestroy = takeRequestLocked();
        }

        destroyRequest(requestToDestroy);
        release();
    }

private:
    Http_Request* takeRequestLocked() {
        auto* current = request;
        request = nullptr;
        return current;
    }

    void destroyRequest(Http_Request* requestToDestroy) {
        if (!requestToDestroy) {
            return;
        }

        if (requestToDestroy->options && requestToDestroy->options->headers) {
            Http_Headers* headers = requestToDestroy->options->headers;
            requestToDestroy->options->headers = nullptr;
            OH_Http_DestroyHeaders(&headers);
        }

        OH_Http_Destroy(&requestToDestroy);
    }

    void release() {
        std::optional<std::size_t> slotToRelease;
        {
            std::scoped_lock lock(mutex);
            slotToRelease = slot;
            slot.reset();
            self.reset();
        }

        if (slotToRelease) {
            CallbackSlots::release(*slotToRelease);
        }
    }

    util::RunLoop& runLoop;
    Resource resource;
    FileSource::Callback callback;
    Http_Request* request = nullptr;
    mutable std::mutex mutex;
    std::optional<std::size_t> slot;
    bool canceled = false;
    bool finished = false;
    std::shared_ptr<std::string> streamedData;
    std::shared_ptr<RequestState> self;
};

void dispatchResponse(std::size_t slot, Http_Response* response, uint32_t errCode) {
    if (auto state = CallbackSlots::get(slot)) {
        state->complete(response, errCode);
    } else if (response && response->destroyResponse) {
        Http_Response* responseToDestroy = response;
        response->destroyResponse(&responseToDestroy);
    }
}

void dispatchData(std::size_t slot, const char* data, std::size_t length) {
    if (auto state = CallbackSlots::get(slot)) {
        state->appendData(data, length);
    }
}

void dispatchCanceled(std::size_t slot) {
    if (auto state = CallbackSlots::get(slot)) {
        state->handleCanceled();
    }
}

class HTTPRequest final : public AsyncRequest {
public:
    HTTPRequest(Resource resource, std::string userAgent_, FileSource::Callback callback)
        : userAgent(std::move(userAgent_)),
          state(std::make_shared<RequestState>(*util::RunLoop::Get(), std::move(resource), std::move(callback))) {
        state->keepAlive();
        const auto slot = CallbackSlots::acquire(state);
        if (!slot || !state->setSlot(*slot)) {
            state->complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
            return;
        }

        Http_Request* request = OH_Http_CreateRequest(state->getResource().url.c_str());
        if (!request || !request->options) {
            state->setRequest(request);
            state->complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
            return;
        }
        state->setRequest(request);

        request->options->method = NET_HTTP_METHOD_GET;
        if (state->getResource().dataRange) {
            request->options->resumeFrom = static_cast<int64_t>(state->getResource().dataRange->first);
            request->options->resumeTo = static_cast<int64_t>(state->getResource().dataRange->second);
        }

        request->options->headers = OH_Http_CreateHeaders();
        if (request->options->headers) {
            OH_Http_SetHeaderValue(request->options->headers, "Accept-Encoding", "gzip, deflate");
            OH_Http_SetHeaderValue(request->options->headers, "User-Agent", userAgent.c_str());
            if (state->getResource().priorEtag) {
                OH_Http_SetHeaderValue(
                    request->options->headers, "If-None-Match", state->getResource().priorEtag->c_str());
            } else if (state->getResource().priorModified) {
                const auto modified = util::rfc1123(*state->getResource().priorModified);
                OH_Http_SetHeaderValue(request->options->headers, "If-Modified-Since", modified.c_str());
            }
        }

        Http_EventsHandler handler{};
        handler.onDataReceive = dataCallbacks[*slot];
        handler.onCanceled = canceledCallbacks[*slot];

        const int result = OH_Http_Request(request, responseCallbacks[*slot], handler);
        if (result != OH_HTTP_RESULT_OK) {
            state->complete(nullptr, static_cast<uint32_t>(result));
        }
    }

    ~HTTPRequest() override { state->cancel(); }

private:
    std::string userAgent;
    std::shared_ptr<RequestState> state;
};

} // namespace

class HTTPFileSource::Impl {
public:
    Impl(const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()) {}

    void setResourceOptions(ResourceOptions options) {
        std::scoped_lock lock(mutex);
        resourceOptions = options.clone();
    }

    ResourceOptions getResourceOptions() {
        std::scoped_lock lock(mutex);
        return resourceOptions.clone();
    }

    void setClientOptions(ClientOptions options) {
        std::scoped_lock lock(mutex);
        clientOptions = options.clone();
    }

    ClientOptions getClientOptions() {
        std::scoped_lock lock(mutex);
        return clientOptions.clone();
    }

    std::string getUserAgent() {
        std::scoped_lock lock(mutex);
        return ohos::buildUserAgent(clientOptions);
    }

private:
    std::mutex mutex;
    ResourceOptions resourceOptions;
    ClientOptions clientOptions;
};

HTTPFileSource::HTTPFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<Impl>(resourceOptions, clientOptions)) {}

HTTPFileSource::~HTTPFileSource() = default;

std::unique_ptr<AsyncRequest> HTTPFileSource::request(const Resource& resource, Callback callback) {
    return std::make_unique<HTTPRequest>(resource, impl->getUserAgent(), std::move(callback));
}

void HTTPFileSource::setResourceOptions(ResourceOptions options) {
    impl->setResourceOptions(std::move(options));
}

ResourceOptions HTTPFileSource::getResourceOptions() {
    return impl->getResourceOptions();
}

void HTTPFileSource::setClientOptions(ClientOptions options) {
    impl->setClientOptions(std::move(options));
}

ClientOptions HTTPFileSource::getClientOptions() {
    return impl->getClientOptions();
}

} // namespace mbgl
