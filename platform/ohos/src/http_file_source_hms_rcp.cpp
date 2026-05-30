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

#include <RemoteCommunicationKit/rcp.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace mbgl {
namespace {

constexpr uint32_t RCP_ERROR_UNSUPPORTED_PROTOCOL = 1007900001;
constexpr uint32_t RCP_ERROR_INVALID_URL = 1007900003;
constexpr uint32_t RCP_ERROR_COULDNT_RESOLVE_PROXY = 1007900005;
constexpr uint32_t RCP_ERROR_COULDNT_RESOLVE_HOST = 1007900006;
constexpr uint32_t RCP_ERROR_COULDNT_CONNECT = 1007900007;
constexpr uint32_t RCP_ERROR_INVALID_SERVER_RESPONSE = 1007900008;
constexpr uint32_t RCP_ERROR_ACCESS_REMOTE_DENIED = 1007900009;
constexpr uint32_t RCP_ERROR_HTTP2_FRAMING = 1007900016;
constexpr uint32_t RCP_ERROR_PARTIAL_FILE = 1007900018;
constexpr uint32_t RCP_ERROR_UPLOAD = 1007900025;
constexpr uint32_t RCP_ERROR_OPEN_LOCAL_DATA = 1007900026;
constexpr uint32_t RCP_ERROR_OUT_OF_MEMORY = 1007900027;
constexpr uint32_t RCP_ERROR_TIMEOUT = 1007900028;
constexpr uint32_t RCP_ERROR_TOO_MANY_REDIRECTS = 1007900047;
constexpr uint32_t RCP_ERROR_SERVER_RETURNED_NOTHING = 1007900052;
constexpr uint32_t RCP_ERROR_SEND = 1007900055;
constexpr uint32_t RCP_ERROR_RECV = 1007900056;
constexpr uint32_t RCP_ERROR_LOCAL_SSL = 1007900058;
constexpr uint32_t RCP_ERROR_SSL_CIPHER = 1007900059;
constexpr uint32_t RCP_ERROR_SSL = 1007900060;
constexpr uint32_t RCP_ERROR_INVALID_ENCODING = 1007900061;
constexpr uint32_t RCP_ERROR_FILE_TOO_LARGE = 1007900063;
constexpr uint32_t RCP_ERROR_REMOTE_DISK_FULL = 1007900070;
constexpr uint32_t RCP_ERROR_REMOTE_FILE_EXISTS = 1007900073;
constexpr uint32_t RCP_ERROR_CA_CERT = 1007900077;
constexpr uint32_t RCP_ERROR_REMOTE_FILE_NOT_FOUND = 1007900078;
constexpr uint32_t RCP_ERROR_AUTHENTICATION = 1007900094;
constexpr uint32_t RCP_ERROR_CANCELED = 1007900992;
constexpr uint32_t RCP_ERROR_SESSION_CLOSED = 1007900993;
constexpr uint32_t RCP_ERROR_GET_SYSTEM_PROXY = 1007900995;
constexpr uint32_t RCP_ERROR_PROXY_TYPE_UNSUPPORTED = 1007900996;
constexpr uint32_t RCP_ERROR_INVALID_CONTENT_TYPE = 1007900997;
constexpr uint32_t RCP_ERROR_METHOD_UNSUPPORTED = 1007900998;
constexpr uint32_t RCP_ERROR_INTERNAL = 1007900999;
constexpr std::size_t MaxConcurrentRequests = 128;

class RequestState;

struct CallbackContext {
    std::size_t slot = 0;
};

std::array<CallbackContext, MaxConcurrentRequests> makeCallbackContexts() {
    std::array<CallbackContext, MaxConcurrentRequests> contexts{};
    for (std::size_t i = 0; i < contexts.size(); ++i) {
        contexts[i].slot = i;
    }
    return contexts;
}

CallbackContext* callbackContext(std::size_t slot) {
    static auto contexts = makeCallbackContexts();
    return slot < contexts.size() ? &contexts[slot] : nullptr;
}

void dispatchResponse(void* ctx, Rcp_Response* response, uint32_t errCode);

std::string toLowerASCII(const char* value) {
    std::string result = value ? value : "";
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::optional<std::string> headerValue(Rcp_Headers* headers, const char* name) {
    if (!headers) {
        return std::nullopt;
    }

    const auto expected = toLowerASCII(name);
    Rcp_HeaderEntry* entries = HMS_Rcp_GetHeaderEntries(headers);
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

        HMS_Rcp_DestroyHeaderEntries(entries);
        return value;
    }

    HMS_Rcp_DestroyHeaderEntries(entries);
    return std::nullopt;
}

Response::Error::Reason errorReason(uint32_t errCode) {
    switch (errCode) {
        case RCP_ERROR_COULDNT_RESOLVE_PROXY:
        case RCP_ERROR_COULDNT_RESOLVE_HOST:
        case RCP_ERROR_COULDNT_CONNECT:
        case RCP_ERROR_TIMEOUT:
        case RCP_ERROR_RECV:
        case RCP_ERROR_SSL:
        case RCP_ERROR_CA_CERT:
            return Response::Error::Reason::Connection;
        default:
            return Response::Error::Reason::Other;
    }
}

const char* errorCodeName(uint32_t errCode) {
    switch (errCode) {
        case RCP_ERROR_UNSUPPORTED_PROTOCOL:
            return "RCP_ERROR_UNSUPPORTED_PROTOCOL";
        case RCP_ERROR_INVALID_URL:
            return "RCP_ERROR_INVALID_URL";
        case RCP_ERROR_COULDNT_RESOLVE_PROXY:
            return "RCP_ERROR_COULDNT_RESOLVE_PROXY";
        case RCP_ERROR_COULDNT_RESOLVE_HOST:
            return "RCP_ERROR_COULDNT_RESOLVE_HOST";
        case RCP_ERROR_COULDNT_CONNECT:
            return "RCP_ERROR_COULDNT_CONNECT";
        case RCP_ERROR_INVALID_SERVER_RESPONSE:
            return "RCP_ERROR_INVALID_SERVER_RESPONSE";
        case RCP_ERROR_ACCESS_REMOTE_DENIED:
            return "RCP_ERROR_ACCESS_REMOTE_DENIED";
        case RCP_ERROR_HTTP2_FRAMING:
            return "RCP_ERROR_HTTP2_FRAMING";
        case RCP_ERROR_PARTIAL_FILE:
            return "RCP_ERROR_PARTIAL_FILE";
        case RCP_ERROR_UPLOAD:
            return "RCP_ERROR_UPLOAD";
        case RCP_ERROR_OPEN_LOCAL_DATA:
            return "RCP_ERROR_OPEN_LOCAL_DATA";
        case RCP_ERROR_OUT_OF_MEMORY:
            return "RCP_ERROR_OUT_OF_MEMORY";
        case RCP_ERROR_TIMEOUT:
            return "RCP_ERROR_TIMEOUT";
        case RCP_ERROR_TOO_MANY_REDIRECTS:
            return "RCP_ERROR_TOO_MANY_REDIRECTS";
        case RCP_ERROR_SERVER_RETURNED_NOTHING:
            return "RCP_ERROR_SERVER_RETURNED_NOTHING";
        case RCP_ERROR_SEND:
            return "RCP_ERROR_SEND";
        case RCP_ERROR_RECV:
            return "RCP_ERROR_RECV";
        case RCP_ERROR_LOCAL_SSL:
            return "RCP_ERROR_LOCAL_SSL";
        case RCP_ERROR_SSL_CIPHER:
            return "RCP_ERROR_SSL_CIPHER";
        case RCP_ERROR_SSL:
            return "RCP_ERROR_SSL";
        case RCP_ERROR_INVALID_ENCODING:
            return "RCP_ERROR_INVALID_ENCODING";
        case RCP_ERROR_FILE_TOO_LARGE:
            return "RCP_ERROR_FILE_TOO_LARGE";
        case RCP_ERROR_REMOTE_DISK_FULL:
            return "RCP_ERROR_REMOTE_DISK_FULL";
        case RCP_ERROR_REMOTE_FILE_EXISTS:
            return "RCP_ERROR_REMOTE_FILE_EXISTS";
        case RCP_ERROR_CA_CERT:
            return "RCP_ERROR_CA_CERT";
        case RCP_ERROR_REMOTE_FILE_NOT_FOUND:
            return "RCP_ERROR_REMOTE_FILE_NOT_FOUND";
        case RCP_ERROR_AUTHENTICATION:
            return "RCP_ERROR_AUTHENTICATION";
        case RCP_ERROR_CANCELED:
            return "RCP_ERROR_CANCELED";
        case RCP_ERROR_SESSION_CLOSED:
            return "RCP_ERROR_SESSION_CLOSED";
        case RCP_ERROR_GET_SYSTEM_PROXY:
            return "RCP_ERROR_GET_SYSTEM_PROXY";
        case RCP_ERROR_PROXY_TYPE_UNSUPPORTED:
            return "RCP_ERROR_PROXY_TYPE_UNSUPPORTED";
        case RCP_ERROR_INVALID_CONTENT_TYPE:
            return "RCP_ERROR_INVALID_CONTENT_TYPE";
        case RCP_ERROR_METHOD_UNSUPPORTED:
            return "RCP_ERROR_METHOD_UNSUPPORTED";
        case RCP_ERROR_INTERNAL:
            return "RCP_ERROR_INTERNAL";
        default:
            return nullptr;
    }
}

std::string errorMessage(const Resource& resource, uint32_t errCode) {
    std::string message{"HarmonyOS RCP request failed with "};
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

class RcpConfigurationStorage {
public:
    RcpConfigurationStorage() {
        configuration.transferConfiguration.autoRedirect = true;
        configuration.transferConfiguration.timeout.connectMs = 60000;
        configuration.transferConfiguration.timeout.transferMs = 60000;
        configuration.transferConfiguration.pathPreference = RCP_PATH_PREFERENCE_AUTO;

        configuration.proxyConfiguration.proxyType = RCP_PROXY_SYSTEM;
        configuration.proxyConfiguration.customProxy.url = empty();
        configuration.proxyConfiguration.customProxy.createTunnel = RCP_PROXY_TUNNEL_AUTO;
        initializeSecurity(configuration.proxyConfiguration.customProxy.securityConfiguration);

        configuration.dnsConfiguration.dnsOverHttps.url = empty();
        initializeSecurity(configuration.securityConfiguration);
    }

    Rcp_Configuration* get() { return &configuration; }

    const char* empty() const { return emptyString.data(); }

private:
    void initializeSecurity(Rcp_SecurityConfiguration& security) {
        security.remoteValidationType = RCP_REMOTE_VALIDATION_SYSTEM;
        security.certificateAuthority.content = emptyString.data();
        security.certificateAuthority.filePath = emptyString.data();
        security.certificateAuthority.folderPath = emptyString.data();
        security.certificate.content = emptyString.data();
        security.certificate.filePath = emptyString.data();
        security.certificate.key = emptyString.data();
        security.certificate.keyPassword = emptyString.data();
        security.certificate.type = RCP_CERT_PEM;
        security.serverAuthentication.credential.username = emptyString.data();
        security.serverAuthentication.credential.password = emptyString.data();
        security.serverAuthentication.authenticationType = RCP_AUTHENTICATION_AUTO;
    }

    std::array<char, 1> emptyString{{'\0'}};
    Rcp_Configuration configuration{};
};

void applyCacheHeaders(Response& response, Rcp_Headers* headers) {
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

Response makeResponse(const Resource& resource, Rcp_Response* rcpResponse, uint32_t errCode) {
    if (errCode != 0) {
        return makeNetworkError(resource, errCode);
    }

    Response response;
    if (!rcpResponse) {
        response.error = std::make_unique<Response::Error>(Response::Error::Reason::Other,
                                                           "HarmonyOS RCP returned no response");
        return response;
    }

    applyCacheHeaders(response, rcpResponse->headers);

    const int code = static_cast<int>(rcpResponse->statusCode);
    if (code == 200 || code == 206) {
        if (rcpResponse->body.buffer && rcpResponse->body.length > 0) {
            response.data = std::make_shared<std::string>(rcpResponse->body.buffer, rcpResponse->body.length);
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
            http::parseRetryHeaders(headerValue(rcpResponse->headers, "retry-after"),
                                    headerValue(rcpResponse->headers, "x-rate-limit-reset")));
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
    // RCP callbacks carry user data, but the SDK may still report cancellation
    // after AsyncRequest destruction. Use stable slot contexts instead of a
    // RequestState* so late callbacks cannot dereference freed request state.
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

class RcpSession final {
public:
    RcpSession() {
        Rcp_SessionConfiguration configuration{};
        configuration.type = RCP_SESSION_TYPE_HTTP;
        configuration.baseUrl = requestConfiguration.empty();
        configuration.requestConfiguration = requestConfiguration.get();
        uint32_t errCode = 0;
        session = HMS_Rcp_CreateSession(&configuration, &errCode);
        if (!session) {
            throw std::runtime_error(std::string{"Could not create HarmonyOS RCP session: "} + util::toString(errCode));
        }
    }

    ~RcpSession() {
        std::scoped_lock lock(mutex);
        if (session) {
            HMS_Rcp_CloseSession(&session);
        }
    }

    uint32_t fetch(Rcp_Request* request, const Rcp_ResponseCallbackObject* callbackObject) {
        Rcp_Session* activeSession = nullptr;
        {
            std::scoped_lock lock(mutex);
            activeSession = session;
        }
        if (!activeSession) {
            return RCP_ERROR_CANCELED;
        }
        // Do not hold the session mutex across HMS_Rcp_Fetch; callbacks may cancel
        // other requests and would deadlock if they need the same mutex.
        return HMS_Rcp_Fetch(activeSession, request, callbackObject);
    }

    void cancel(Rcp_Request* request) {
        Rcp_Session* activeSession = nullptr;
        {
            std::scoped_lock lock(mutex);
            activeSession = session;
        }
        if (activeSession && request) {
            HMS_Rcp_CancelRequest(activeSession, request);
        }
    }

private:
    std::mutex mutex;
    RcpConfigurationStorage requestConfiguration;
    Rcp_Session* session = nullptr;
};

class RequestState : public std::enable_shared_from_this<RequestState> {
public:
    RequestState(util::RunLoop& runLoop_,
                 std::shared_ptr<RcpSession> session_,
                 Resource resource_,
                 FileSource::Callback callback_)
        : runLoop(runLoop_),
          session(std::move(session_)),
          resource(std::move(resource_)),
          callback(std::move(callback_)),
          callbackObject{.callback = dispatchResponse, .usrCtx = nullptr} {}

    void keepAlive() { self = shared_from_this(); }

    bool setSlot(std::size_t slot_) {
        auto* context = callbackContext(slot_);
        if (context == nullptr) {
            return false;
        }

        std::scoped_lock lock(mutex);
        if (slot) {
            return false;
        }
        slot = slot_;
        callbackObject.usrCtx = context;
        return true;
    }

    void setRequest(Rcp_Request* request_) {
        std::scoped_lock lock(mutex);
        request = request_;
        if (request) {
            request->configuration = requestConfiguration.get();
        }
    }

    void cancelOnRunLoop() {
        Rcp_Request* requestToDestroy = nullptr;
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

        if (requestToDestroy) {
            session->cancel(requestToDestroy);
        }
        destroyRequest(requestToDestroy);
        release();
    }

    void cancel() { cancelOnRunLoop(); }

    bool isCanceled() const {
        std::scoped_lock lock(mutex);
        return canceled;
    }

    void complete(Rcp_Response* rcpResponse, uint32_t errCode) {
        Response response = makeResponse(resource, rcpResponse, errCode);
        if (rcpResponse && rcpResponse->destroyResponse) {
            rcpResponse->destroyResponse(rcpResponse);
        }

        runLoop.invoke([state = shared_from_this(), response = std::move(response)]() mutable {
            FileSource::Callback localCallback;
            Rcp_Request* requestToDestroy = nullptr;
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

    void abandon(Rcp_Response* response) {
        auto keepAlive = shared_from_this();
        if (response && response->destroyResponse) {
            response->destroyResponse(response);
        }

        Rcp_Request* requestToDestroy = nullptr;
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

    const Resource& getResource() const { return resource; }

    const Rcp_ResponseCallbackObject* getCallbackObject() const { return &callbackObject; }

    uint32_t fetch() {
        Rcp_Request* requestToFetch = nullptr;
        {
            std::scoped_lock lock(mutex);
            requestToFetch = request;
        }
        if (!requestToFetch) {
            return RCP_ERROR_OUT_OF_MEMORY;
        }

        return session->fetch(requestToFetch, &callbackObject);
    }

private:
    Rcp_Request* takeRequestLocked() {
        auto* current = request;
        request = nullptr;
        return current;
    }

    void destroyRequest(Rcp_Request* requestToDestroy) {
        if (requestToDestroy) {
            requestToDestroy->configuration = nullptr;
            HMS_Rcp_DestroyRequest(requestToDestroy);
        }
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
    std::shared_ptr<RcpSession> session;
    Resource resource;
    FileSource::Callback callback;
    Rcp_ResponseCallbackObject callbackObject{};
    RcpConfigurationStorage requestConfiguration;
    Rcp_Request* request = nullptr;
    mutable std::mutex mutex;
    std::optional<std::size_t> slot;
    bool canceled = false;
    bool finished = false;
    std::shared_ptr<RequestState> self;
};

void dispatchResponse(void* ctx, Rcp_Response* response, uint32_t errCode) {
    auto* context = static_cast<CallbackContext*>(ctx);
    if (context == nullptr) {
        if (response && response->destroyResponse) {
            response->destroyResponse(response);
        }
        return;
    }

    auto state = CallbackSlots::get(context->slot);
    if (!state) {
        if (response && response->destroyResponse) {
            response->destroyResponse(response);
        }
        return;
    }

    if (errCode == RCP_ERROR_CANCELED || state->isCanceled()) {
        state->abandon(response);
        return;
    }
    state->complete(response, errCode);
}

class HTTPRequest final : public AsyncRequest {
public:
    HTTPRequest(std::shared_ptr<RcpSession> session_,
                Resource resource,
                std::string userAgent_,
                FileSource::Callback callback)
        : userAgent(std::move(userAgent_)),
          state(std::make_shared<RequestState>(
              *util::RunLoop::Get(), std::move(session_), std::move(resource), std::move(callback))) {
        state->keepAlive();
        const auto slot = CallbackSlots::acquire(state);
        if (!slot || !state->setSlot(*slot)) {
            if (slot) {
                CallbackSlots::release(*slot);
            }
            state->complete(nullptr, RCP_ERROR_OUT_OF_MEMORY);
            return;
        }

        Rcp_Request* request = HMS_Rcp_CreateRequest(state->getResource().url.c_str());
        if (!request) {
            state->complete(nullptr, RCP_ERROR_OUT_OF_MEMORY);
            return;
        }
        state->setRequest(request);

        request->method = RCP_METHOD_GET;
        request->headers = HMS_Rcp_CreateHeaders();
        if (request->headers) {
            HMS_Rcp_SetHeaderValue(request->headers, "Accept-Encoding", "gzip, deflate");
            HMS_Rcp_SetHeaderValue(request->headers, "User-Agent", userAgent.c_str());
            if (state->getResource().dataRange) {
                const auto range = std::string{"bytes="} + util::toString(state->getResource().dataRange->first) + "-" +
                                   util::toString(state->getResource().dataRange->second);
                HMS_Rcp_SetHeaderValue(request->headers, "Range", range.c_str());
            }
            if (state->getResource().priorEtag) {
                HMS_Rcp_SetHeaderValue(request->headers, "If-None-Match", state->getResource().priorEtag->c_str());
            } else if (state->getResource().priorModified) {
                const auto modified = util::rfc1123(*state->getResource().priorModified);
                HMS_Rcp_SetHeaderValue(request->headers, "If-Modified-Since", modified.c_str());
            }
        }

        const uint32_t result = state->fetch();
        if (result != 0) {
            state->complete(nullptr, result);
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
          clientOptions(clientOptions_.clone()),
          session(std::make_shared<RcpSession>()) {}

    std::shared_ptr<RcpSession> getSession() const { return session; }

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
    std::shared_ptr<RcpSession> session;
};

HTTPFileSource::HTTPFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<Impl>(resourceOptions, clientOptions)) {}

HTTPFileSource::~HTTPFileSource() = default;

std::unique_ptr<AsyncRequest> HTTPFileSource::request(const Resource& resource, Callback callback) {
    return std::make_unique<HTTPRequest>(impl->getSession(), resource, impl->getUserAgent(), std::move(callback));
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
