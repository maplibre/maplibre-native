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
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace mbgl {
namespace {

constexpr std::size_t kMaxActiveRequests = 128;
constexpr std::size_t kCallbackGenerations = 64;
constexpr std::size_t kCallbackTokens = kMaxActiveRequests * kCallbackGenerations;

class RequestState;

void destroyResponse(Http_Response* response) {
    if (response && response->destroyResponse) {
        // The current OpenHarmony wrapper exposes cookies as a pointer into an
        // internal string, but its destroy callback frees the pointer.
        response->cookies = nullptr;
        response->destroyResponse(&response);
    }
}

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

    Http_HeaderEntry* entriesToDestroy = entries;
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

        OH_Http_DestroyHeaderEntries(&entriesToDestroy);
        return value;
    }

    OH_Http_DestroyHeaderEntries(&entriesToDestroy);
    return std::nullopt;
}

Response::Error::Reason errorReason(uint32_t errCode) {
    switch (errCode) {
        case OH_HTTP_RESOLVE_PROXY_FAILED:
        case OH_HTTP_RESOLVE_HOST_FAILED:
        case OH_HTTP_CONNECT_SERVER_FAILED:
        case OH_HTTP_OPERATION_TIMEOUT:
        case OH_HTTP_RECEIVE_DATA_FAILED:
        case OH_HTTP_INVALID_SSL_PEER_CERT:
        case OH_HTTP_SSL_CA_NOT_EXIST:
            return Response::Error::Reason::Connection;
        default:
            return Response::Error::Reason::Other;
    }
}

const char* errorCodeName(uint32_t errCode) {
    switch (errCode) {
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

Response makeResponse(const Resource& resource, Http_Response* httpResponse, uint32_t errCode) {
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

class SlotPool {
public:
    static void enqueueOrStart(const std::shared_ptr<RequestState>& state);
    static void release(std::size_t token, const RequestState* state);
    static void handleResponse(std::size_t token, Http_Response* response, uint32_t errCode);
    static void handleCanceled(std::size_t token);
    static Http_ResponseCallback responseCallback(std::size_t token);
    static Http_OnVoidCallback canceledCallback(std::size_t token);

private:
    struct Slot {
        std::weak_ptr<RequestState> state;
        uint64_t generation = 0;
    };

    static std::shared_ptr<RequestState> stateForToken(std::size_t token);
    static std::optional<std::size_t> assignSlotLocked(const std::shared_ptr<RequestState>& state);
    static void startNextPending();

    static std::mutex mutex;
    static std::array<Slot, kMaxActiveRequests> slots;
    static std::deque<std::weak_ptr<RequestState>> pending;
    static std::size_t nextSlot;
};

template <std::size_t Token>
void responseThunk(Http_Response* response, uint32_t errCode) {
    SlotPool::handleResponse(Token, response, errCode);
}

template <std::size_t Token>
void canceledThunk() {
    SlotPool::handleCanceled(Token);
}

template <std::size_t... Tokens>
constexpr auto makeResponseCallbacks(std::index_sequence<Tokens...>) {
    return std::array<Http_ResponseCallback, sizeof...(Tokens)>{&responseThunk<Tokens>...};
}

template <std::size_t... Tokens>
constexpr auto makeCanceledCallbacks(std::index_sequence<Tokens...>) {
    return std::array<Http_OnVoidCallback, sizeof...(Tokens)>{&canceledThunk<Tokens>...};
}

constexpr auto responseCallbacks = makeResponseCallbacks(std::make_index_sequence<kCallbackTokens>{});
constexpr auto canceledCallbacks = makeCanceledCallbacks(std::make_index_sequence<kCallbackTokens>{});

class RequestState : public std::enable_shared_from_this<RequestState> {
public:
    RequestState(util::RunLoop& runLoop_, Resource resource_, std::string userAgent_, FileSource::Callback callback_)
        : runLoop(runLoop_),
          resource(std::move(resource_)),
          userAgent(std::move(userAgent_)),
          callback(std::move(callback_)) {}

    void begin() {
        {
            std::scoped_lock lock(mutex);
            self = shared_from_this();
        }
        SlotPool::enqueueOrStart(shared_from_this());
    }

    void setSlot(std::size_t token_) {
        std::scoped_lock lock(mutex);
        token = token_;
    }

    bool isCanceled() const {
        std::scoped_lock lock(mutex);
        return canceled;
    }

    void startNativeRequest() {
        const auto activeToken = getToken();
        if (!activeToken) {
            releaseSelf();
            return;
        }

        {
            std::scoped_lock lock(mutex);
            if (canceled) {
                finished = true;
            } else {
                starting = true;
            }
        }
        if (isFinished()) {
            SlotPool::release(*activeToken, this);
            releaseSelf();
            return;
        }

        Http_Request* newRequest = OH_Http_CreateRequest(resource.url.c_str());
        if (!newRequest) {
            complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
            return;
        }

        Http_Headers* newHeaders = OH_Http_CreateHeaders();
        if (!newHeaders) {
            OH_Http_Destroy(&newRequest);
            complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
            return;
        }

        if (!setHeader(newHeaders, "Accept-Encoding", "gzip, deflate") ||
            !setHeader(newHeaders, "User-Agent", userAgent.c_str())) {
            OH_Http_DestroyHeaders(&newHeaders);
            OH_Http_Destroy(&newRequest);
            complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
            return;
        }

        if (resource.dataRange) {
            const auto range = std::string{"bytes="} + util::toString(resource.dataRange->first) + "-" +
                               util::toString(resource.dataRange->second);
            if (!setHeader(newHeaders, "Range", range.c_str())) {
                OH_Http_DestroyHeaders(&newHeaders);
                OH_Http_Destroy(&newRequest);
                complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
                return;
            }
        }

        if (resource.priorEtag) {
            if (!setHeader(newHeaders, "If-None-Match", resource.priorEtag->c_str())) {
                OH_Http_DestroyHeaders(&newHeaders);
                OH_Http_Destroy(&newRequest);
                complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
                return;
            }
        } else if (resource.priorModified) {
            const auto modified = util::rfc1123(*resource.priorModified);
            if (!setHeader(newHeaders, "If-Modified-Since", modified.c_str())) {
                OH_Http_DestroyHeaders(&newHeaders);
                OH_Http_Destroy(&newRequest);
                complete(nullptr, OH_HTTP_OUT_OF_MEMORY);
                return;
            }
        }

        Http_EventsHandler handler{};
        handler.onCanceled = SlotPool::canceledCallback(*activeToken);

        {
            std::scoped_lock lock(mutex);
            if (canceled) {
                OH_Http_DestroyHeaders(&newHeaders);
                OH_Http_Destroy(&newRequest);
                finished = true;
            } else {
                proxy.proxyType = HTTP_PROXY_SYSTEM;
                options.method = NET_HTTP_METHOD_GET;
                options.headers = newHeaders;
                options.readTimeout = 60000;
                options.connectTimeout = 60000;
                options.httpProtocol = OH_HTTP_NONE;
                options.httpProxy = &proxy;
                request = newRequest;
                headers = newHeaders;
                request->options = &options;
                starting = true;
            }
        }

        if (isFinished()) {
            SlotPool::release(*activeToken, this);
            releaseSelf();
            return;
        }

        const int result = OH_Http_Request(newRequest, SlotPool::responseCallback(*activeToken), handler);
        {
            std::scoped_lock lock(mutex);
            starting = false;
        }
        if (isCanceled()) {
            if (result == OH_HTTP_RESULT_OK) {
                cancelStartedRequest();
            } else {
                finish(std::nullopt);
            }
            return;
        }
        if (result != OH_HTTP_RESULT_OK) {
            complete(nullptr, static_cast<uint32_t>(result));
        }
    }

    void cancel() {
        Http_Request* requestToDestroy = nullptr;
        Http_Headers* headersToDestroy = nullptr;
        std::optional<std::size_t> activeToken;
        bool shouldReleasePending = false;
        bool shouldWaitForNativeCallback = false;
        {
            std::scoped_lock lock(mutex);
            canceled = true;
            callback = nullptr;
            if (finished) {
                return;
            }
            if (awaitingNativeCallback) {
                return;
            }
            if (starting) {
                return;
            }
            activeToken = token;
            requestToDestroy = request;
            request = nullptr;
            headersToDestroy = headers;
            headers = nullptr;
            options.headers = nullptr;
            if (!activeToken) {
                finished = true;
                shouldReleasePending = true;
            } else if (requestToDestroy) {
                // The callback token cannot be reused until net_http reports
                // the cancellation; callbacks do not carry request context.
                awaitingNativeCallback = true;
                shouldWaitForNativeCallback = true;
            } else {
                finished = true;
            }
        }

        if (requestToDestroy) {
            OH_Http_Destroy(&requestToDestroy);
        }
        if (headersToDestroy) {
            OH_Http_DestroyHeaders(&headersToDestroy);
        }
        if (activeToken && !shouldWaitForNativeCallback) {
            SlotPool::release(*activeToken, this);
        }
        if (shouldReleasePending) {
            releaseSelf();
        } else if (activeToken && !shouldWaitForNativeCallback) {
            releaseSelf();
        }
    }

    void complete(Http_Response* httpResponse, uint32_t errCode) {
        auto response = makeResponse(resource, httpResponse, errCode);
        destroyResponse(httpResponse);
        finish(std::move(response));
    }

    void completeCanceled() { finish(std::nullopt); }

private:
    static bool setHeader(Http_Headers* headers, const char* name, const char* value) {
        return OH_Http_SetHeaderValue(headers, name, value) == OH_HTTP_RESULT_OK;
    }

    std::optional<std::size_t> getToken() const {
        std::scoped_lock lock(mutex);
        return token;
    }

    bool isFinished() const {
        std::scoped_lock lock(mutex);
        return finished;
    }

    void cancelStartedRequest() {
        Http_Request* requestToDestroy = nullptr;
        Http_Headers* headersToDestroy = nullptr;
        {
            std::scoped_lock lock(mutex);
            if (finished || awaitingNativeCallback) {
                return;
            }
            requestToDestroy = request;
            request = nullptr;
            headersToDestroy = headers;
            headers = nullptr;
            options.headers = nullptr;
            if (requestToDestroy) {
                awaitingNativeCallback = true;
            } else {
                finished = true;
            }
        }

        if (requestToDestroy) {
            OH_Http_Destroy(&requestToDestroy);
        }
        if (headersToDestroy) {
            OH_Http_DestroyHeaders(&headersToDestroy);
        }
        if (!requestToDestroy) {
            const auto activeToken = getToken();
            if (activeToken) {
                SlotPool::release(*activeToken, this);
            }
            releaseSelf();
        }
    }

    void finish(std::optional<Response> response) {
        auto keepAlive = shared_from_this();

        const auto activeToken = getToken();
        Http_Request* requestToDestroy = nullptr;
        Http_Headers* headersToDestroy = nullptr;
        bool shouldCallback = false;
        {
            std::scoped_lock lock(mutex);
            if (finished) {
                return;
            }
            finished = true;
            starting = false;
            requestToDestroy = request;
            request = nullptr;
            headersToDestroy = headers;
            headers = nullptr;
            options.headers = nullptr;
            awaitingNativeCallback = false;
            shouldCallback = response && !canceled && static_cast<bool>(callback);
        }

        if (requestToDestroy) {
            OH_Http_Destroy(&requestToDestroy);
        }
        if (headersToDestroy) {
            OH_Http_DestroyHeaders(&headersToDestroy);
        }
        if (activeToken) {
            SlotPool::release(*activeToken, this);
        }

        if (shouldCallback) {
            runLoop.invoke([state = shared_from_this(), response = std::move(*response)]() mutable {
                FileSource::Callback localCallback;
                {
                    std::scoped_lock lock(state->mutex);
                    if (!state->canceled) {
                        localCallback = std::move(state->callback);
                    }
                    state->callback = nullptr;
                }

                state->releaseSelf();
                if (localCallback) {
                    localCallback(response);
                }
            });
        } else {
            releaseSelf();
        }
    }

    void releaseSelf() {
        std::scoped_lock lock(mutex);
        self.reset();
    }

    util::RunLoop& runLoop;
    Resource resource;
    std::string userAgent;
    FileSource::Callback callback;

    mutable std::mutex mutex;
    std::shared_ptr<RequestState> self;
    std::optional<std::size_t> token;
    Http_Request* request = nullptr;
    Http_Headers* headers = nullptr;
    Http_RequestOptions options{};
    Http_Proxy proxy{};
    bool canceled = false;
    bool finished = false;
    bool starting = false;
    bool awaitingNativeCallback = false;
};

std::mutex SlotPool::mutex;
std::array<SlotPool::Slot, kMaxActiveRequests> SlotPool::slots;
std::deque<std::weak_ptr<RequestState>> SlotPool::pending;
std::size_t SlotPool::nextSlot = 0;

std::optional<std::size_t> SlotPool::assignSlotLocked(const std::shared_ptr<RequestState>& state) {
    for (std::size_t offset = 0; offset < slots.size(); ++offset) {
        const auto index = (nextSlot + offset) % slots.size();
        if (!slots[index].state.expired()) {
            continue;
        }

        const auto token = index * kCallbackGenerations + (slots[index].generation % kCallbackGenerations);
        slots[index].state = state;
        nextSlot = (index + 1) % slots.size();
        state->setSlot(token);
        return token;
    }
    return std::nullopt;
}

void SlotPool::enqueueOrStart(const std::shared_ptr<RequestState>& state) {
    std::optional<std::size_t> token;
    {
        std::scoped_lock lock(mutex);
        token = assignSlotLocked(state);
        if (!token) {
            pending.emplace_back(state);
            return;
        }
    }

    state->startNativeRequest();
}

void SlotPool::release(std::size_t token, const RequestState* state) {
    const auto index = token / kCallbackGenerations;
    if (index >= slots.size()) {
        return;
    }

    {
        std::scoped_lock lock(mutex);
        auto current = slots[index].state.lock();
        if (current.get() != state) {
            return;
        }

        slots[index].state.reset();
        ++slots[index].generation;
    }

    startNextPending();
}

void SlotPool::startNextPending() {
    std::shared_ptr<RequestState> next;
    {
        std::scoped_lock lock(mutex);
        while (!pending.empty()) {
            next = pending.front().lock();
            pending.pop_front();
            if (!next || next->isCanceled()) {
                next.reset();
                continue;
            }
            if (assignSlotLocked(next)) {
                break;
            }
            pending.emplace_front(next);
            next.reset();
            break;
        }
    }

    if (next) {
        next->startNativeRequest();
    }
}

std::shared_ptr<RequestState> SlotPool::stateForToken(std::size_t token) {
    const auto index = token / kCallbackGenerations;
    const auto generation = token % kCallbackGenerations;
    if (index >= slots.size()) {
        return nullptr;
    }

    std::scoped_lock lock(mutex);
    if ((slots[index].generation % kCallbackGenerations) != generation) {
        return nullptr;
    }
    return slots[index].state.lock();
}

void SlotPool::handleResponse(std::size_t token, Http_Response* response, uint32_t errCode) {
    auto state = stateForToken(token);
    if (!state) {
        destroyResponse(response);
        return;
    }
    state->complete(response, errCode);
}

void SlotPool::handleCanceled(std::size_t token) {
    auto state = stateForToken(token);
    if (!state) {
        return;
    }
    state->completeCanceled();
}

Http_ResponseCallback SlotPool::responseCallback(std::size_t token) {
    return token < responseCallbacks.size() ? responseCallbacks[token] : nullptr;
}

Http_OnVoidCallback SlotPool::canceledCallback(std::size_t token) {
    return token < canceledCallbacks.size() ? canceledCallbacks[token] : nullptr;
}

class HTTPRequest final : public AsyncRequest {
public:
    HTTPRequest(Resource resource, std::string userAgent, FileSource::Callback callback)
        : state(std::make_shared<RequestState>(
              *util::RunLoop::Get(), std::move(resource), std::move(userAgent), std::move(callback))) {
        state->begin();
    }

    ~HTTPRequest() override { state->cancel(); }

private:
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
