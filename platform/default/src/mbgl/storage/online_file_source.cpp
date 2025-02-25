#include <mbgl/platform/settings.hpp>
#include <mbgl/storage/file_source_request.hpp>
#include <mbgl/storage/http_file_source.hpp>
#include <mbgl/storage/network_status.hpp>
#include <mbgl/storage/online_file_source.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/storage/resource_transform.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/tile_server_options.hpp>

#include <mbgl/actor/mailbox.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/chrono.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/http_timeout.hpp>
#include <mbgl/util/mapbox.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/util/timer.hpp>

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <utility>

namespace mbgl {

// For testing only
constexpr const char* ONLINE_STATUS_KEY = "online-status";

class OnlineFileSourceThread;

struct OnlineFileRequest {
    OnlineFileRequest(Resource resource_, std::function<void(Response)>&& callback_, OnlineFileSourceThread& impl_);
    ~OnlineFileRequest();

    void networkIsReachableAgain();
    void activate();
    void schedule(Duration timeout);
    void completed(Response);

    void setTransformedURL(const std::string& url);
    ActorRef<OnlineFileRequest> actor();
    void onCancel(std::function<void()>);

    Duration getUpdateInterval(std::optional<Timestamp> expires) const;
    OnlineFileSourceThread& impl;
    Resource resource;
    std::unique_ptr<AsyncRequest> request;
    util::Timer timer;
    std::function<void(Response)> callback;

    std::function<void()> cancelCallback = nullptr;
    std::shared_ptr<Mailbox> mailbox;

    // Counts the number of times a response was already expired when received.
    // We're using this to add a delay when making a new request so we don't
    // keep retrying immediately in case of a server serving expired tiles.
    uint32_t expiredRequests = 0;

    // Counts the number of subsequent failed requests. We're using this value
    // for exponential backoff when retrying requests.
    uint32_t failedRequests = 0;
    Response::Error::Reason failedRequestReason = Response::Error::Reason::Success;
    std::optional<Timestamp> retryAfter;
};

class OnlineFileSourceThread {
public:
    OnlineFileSourceThread(const ResourceOptions& resourceOptions_, const ClientOptions& clientOptions_)
        : resourceOptions(resourceOptions_.clone()),
          clientOptions(clientOptions_.clone()),
          httpFileSource(resourceOptions_, clientOptions_) {
        NetworkStatus::Subscribe(&reachability);
        setMaximumConcurrentRequests(util::DEFAULT_MAXIMUM_CONCURRENT_REQUESTS);
    }

    ~OnlineFileSourceThread() { NetworkStatus::Unsubscribe(&reachability); }

    void request(AsyncRequest* req, Resource resource, const ActorRef<FileSourceRequest>& ref) {
        auto callback = [ref](const Response& res) {
            ref.invoke(&FileSourceRequest::setResponse, res);
        };
        tasks[req] = std::make_unique<OnlineFileRequest>(std::move(resource), std::move(callback), *this);
    }

    void cancel(AsyncRequest* req) {
        auto it = tasks.find(req);
        assert(it != tasks.end());
        remove(it->second.get());
        tasks.erase(it);
    }

    void add(OnlineFileRequest* req) {
        allRequests.insert(req);
        if (resourceTransform) {
            // Request the ResourceTransform actor a new url and replace the
            // resource url with the transformed one before proceeding to
            // schedule the request.
            resourceTransform.transform(
                req->resource.kind, req->resource.url, [ref = req->actor()](const std::string& url) {
                    ref.invoke(&OnlineFileRequest::setTransformedURL, url);
                });
        } else {
            req->activate();
        }
    }

    void remove(OnlineFileRequest* req) {
        allRequests.erase(req);
        if (activeRequests.erase(req)) {
            activatePendingRequest();
        } else {
            pendingRequests.remove(req);
        }
    }

    void activateOrQueueRequest(OnlineFileRequest* req) {
        assert(allRequests.find(req) != allRequests.end());
        assert(activeRequests.find(req) == activeRequests.end());
        assert(!req->request);

        if (activeRequests.size() >= getMaximumConcurrentRequests()) {
            queueRequest(req);
        } else {
            activateRequest(req);
        }
    }

    void queueRequest(OnlineFileRequest* req) { pendingRequests.insert(req); }

    void activateRequest(OnlineFileRequest* req) {
        auto callback = [=, this](const Response& response) {
            activeRequests.erase(req);
            req->request.reset();
            req->completed(response);
            activatePendingRequest();
        };

        activeRequests.insert(req);

        if (online) {
            req->request = httpFileSource.request(req->resource, callback);
        } else {
            Response response;
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::Connection,
                                                               "Online connectivity is disabled.");
            callback(response);
        }
    }

    void activatePendingRequest() {
        auto req = pendingRequests.pop();

        if (req) {
            activateRequest(*req);
        }
    }

    bool isPending(OnlineFileRequest* req) { return pendingRequests.contains(req); }

    bool isActive(OnlineFileRequest* req) { return activeRequests.find(req) != activeRequests.end(); }

    void setResourceTransform(ResourceTransform transform) { resourceTransform = std::move(transform); }

    void setResourceOptions(ResourceOptions options) { resourceOptions = options; }

    const ResourceOptions& getResourceOptions() const { return resourceOptions; }

    void setClientOptions(ClientOptions options) { clientOptions = options; }

    const ClientOptions& getClientOptions() const { return clientOptions; }

    void setOnlineStatus(bool status) {
        online = status;
        if (online) {
            networkIsReachableAgain();
        }
    }

    uint32_t getMaximumConcurrentRequests() const { return maximumConcurrentRequests; }

    void setMaximumConcurrentRequests(uint32_t maximumConcurrentRequests_) {
        maximumConcurrentRequests = maximumConcurrentRequests_;
    }

    void setAPIBaseURL(std::string t) {
        resourceOptions.withTileServerOptions(TileServerOptions().withBaseURL(std::move(t)));
    }

    const std::string& getAPIBaseURL() const { return resourceOptions.tileServerOptions().baseURL(); }

    void setApiKey(std::string t) { resourceOptions.withApiKey(std::move(t)); }
    const std::string& getApiKey() const { return resourceOptions.apiKey(); }

private:
    friend struct OnlineFileRequest;

    void networkIsReachableAgain() {
        // Notify regular priority requests.
        for (auto& req : allRequests) {
            if (req->resource.priority == Resource::Priority::Regular) {
                req->networkIsReachableAgain();
            }
        }

        // Notify low priority requests.
        for (auto& req : allRequests) {
            if (req->resource.priority == Resource::Priority::Low) {
                req->networkIsReachableAgain();
            }
        }
    }

    // Using Pending Requests as an priority queue which processes
    // file requests in a FIFO manner but prefers regular requests
    // over offline requests with a low priority such that low priority
    // requests do not throttle regular requests.
    //
    // The order of a queue is therefore:
    //
    // hi0 -- hi1 -- hi2 -- hi3 -- lo0 -- lo1 --lo2
    //                              ^
    //                              firstLowPriorityRequest

    struct PendingRequests {
        PendingRequests()
            : firstLowPriorityRequest(queue.begin()) {}

        std::list<OnlineFileRequest*> queue;
        std::list<OnlineFileRequest*>::iterator firstLowPriorityRequest;

        void remove(const OnlineFileRequest* request) {
            auto it = std::find(queue.begin(), queue.end(), request);
            if (it != queue.end()) {
                if (it == firstLowPriorityRequest) {
                    firstLowPriorityRequest++;
                }
                queue.erase(it);
            }
        }

        void insert(OnlineFileRequest* request) {
            if (request->resource.priority == Resource::Priority::Regular) {
                firstLowPriorityRequest = queue.insert(firstLowPriorityRequest, request);
                firstLowPriorityRequest++;
            } else {
                if (firstLowPriorityRequest == queue.end()) {
                    firstLowPriorityRequest = queue.insert(queue.end(), request);
                } else {
                    queue.insert(queue.end(), request);
                }
            }
        }

        std::optional<OnlineFileRequest*> pop() {
            if (queue.empty()) {
                return {};
            }

            if (queue.begin() == firstLowPriorityRequest) {
                firstLowPriorityRequest++;
            }

            OnlineFileRequest* next = queue.front();
            queue.pop_front();
            return {next};
        }

        bool contains(OnlineFileRequest* request) const {
            return (std::find(queue.begin(), queue.end(), request) != queue.end());
        }
    };

    ResourceTransform resourceTransform;

    ResourceOptions resourceOptions;
    ClientOptions clientOptions;

    /**
     * The lifetime of a request is:
     *
     * 1. Waiting for timeout (revalidation or retry)
     * 2. Pending (waiting for room in the active set)
     * 3. Active (open network connection)
     * 4. Back to #1
     *
     * Requests in any state are in `allRequests`. Requests in the pending state are in
     * `pendingRequests`. Requests in the active state are in `activeRequests`.
     */
    std::set<OnlineFileRequest*> allRequests;

    PendingRequests pendingRequests;

    std::set<OnlineFileRequest*> activeRequests;

    bool online = true;
    uint32_t maximumConcurrentRequests = util::DEFAULT_MAXIMUM_CONCURRENT_REQUESTS;
    HTTPFileSource httpFileSource;
    util::AsyncTask reachability{std::bind(&OnlineFileSourceThread::networkIsReachableAgain, this)};
    std::map<AsyncRequest*, std::unique_ptr<OnlineFileRequest>> tasks;
};

class OnlineFileSource::Impl {
public:
    Impl(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
        : cachedResourceOptions(resourceOptions.clone()),
          cachedClientOptions(clientOptions.clone()),
          thread(std::make_unique<util::Thread<OnlineFileSourceThread>>(
              util::makeThreadPrioritySetter(platform::EXPERIMENTAL_THREAD_PRIORITY_NETWORK),
              "OnlineFileSource",
              resourceOptions.clone(),
              clientOptions.clone())) {}

    std::unique_ptr<AsyncRequest> request(std::function<void(Response)> callback, Resource res) {
        auto req = std::make_unique<FileSourceRequest>(std::move(callback));
        req->onCancel(
            [actorRef = thread->actor(), req = req.get()]() { actorRef.invoke(&OnlineFileSourceThread::cancel, req); });
        thread->actor().invoke(&OnlineFileSourceThread::request, req.get(), std::move(res), req->actor());
        return req;
    }

    void pause() { thread->pause(); }

    void resume() { thread->resume(); }

    void setResourceTransform(ResourceTransform transform) {
        thread->actor().invoke(&OnlineFileSourceThread::setResourceTransform, std::move(transform));
    }

    void setResourceOptions(ResourceOptions options) {
        thread->actor().invoke(&OnlineFileSourceThread::setResourceOptions, options.clone());
        {
            std::lock_guard<std::mutex> lock(resourceOptionsMutex);
            cachedResourceOptions = options;
        }
    }

    ResourceOptions getResourceOptions() {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return cachedResourceOptions.clone();
    }

    void setClientOptions(ClientOptions options) {
        thread->actor().invoke(&OnlineFileSourceThread::setClientOptions, options.clone());
        {
            std::lock_guard<std::mutex> lock(clientOptionsMutex);
            cachedClientOptions = options;
        }
    }

    ClientOptions getClientOptions() {
        std::lock_guard<std::mutex> lock(clientOptionsMutex);
        return cachedClientOptions.clone();
    }

    void setOnlineStatus(bool status) { thread->actor().invoke(&OnlineFileSourceThread::setOnlineStatus, status); }

    void setMaximumConcurrentRequests(const mapbox::base::Value& value) {
        if (auto* maximumConcurrentRequests = value.getUint()) {
            assert(*maximumConcurrentRequests < std::numeric_limits<uint32_t>::max());
            const auto maxConcurrentRequests = static_cast<uint32_t>(*maximumConcurrentRequests);
            thread->actor().invoke(&OnlineFileSourceThread::setMaximumConcurrentRequests, maxConcurrentRequests);
            {
                std::lock_guard<std::mutex> lock(maximumConcurrentRequestsMutex);
                cachedMaximumConcurrentRequests = maxConcurrentRequests;
            }
        } else {
            Log::Error(Event::General, "Invalid max-concurrent-requests property value type.");
        }
    }

    uint32_t getMaximumConcurrentRequests() const {
        std::lock_guard<std::mutex> lock(maximumConcurrentRequestsMutex);
        return cachedMaximumConcurrentRequests;
    }

    void setApiKey(const mapbox::base::Value& value) {
        if (auto* apiKey = value.getString()) {
            thread->actor().invoke(&OnlineFileSourceThread::setApiKey, *apiKey);
            {
                std::lock_guard<std::mutex> lock(resourceOptionsMutex);
                cachedResourceOptions.withApiKey(*apiKey);
            }
        } else {
            Log::Error(Event::General, "Invalid apiKey property value type.");
        }
    }

    std::string getApiKey() const {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return cachedResourceOptions.apiKey();
    }

    void setAPIBaseURL(const mapbox::base::Value& value) {
        if (auto* baseURL = value.getString()) {
            thread->actor().invoke(&OnlineFileSourceThread::setAPIBaseURL, *baseURL);
            {
                std::lock_guard<std::mutex> lock(resourceOptionsMutex);
                cachedResourceOptions.withTileServerOptions(
                    cachedResourceOptions.tileServerOptions().clone().withBaseURL(*baseURL));
            }
        } else {
            Log::Error(Event::General, "Invalid base-url property value type.");
        }
    }

    std::string getAPIBaseURL() const {
        std::lock_guard<std::mutex> lock(resourceOptionsMutex);
        return cachedResourceOptions.tileServerOptions().baseURL();
    }

private:
    mutable std::mutex resourceOptionsMutex;
    mutable std::mutex clientOptionsMutex;
    ResourceOptions cachedResourceOptions;
    ClientOptions cachedClientOptions;

    mutable std::mutex maximumConcurrentRequestsMutex;
    uint32_t cachedMaximumConcurrentRequests = util::DEFAULT_MAXIMUM_CONCURRENT_REQUESTS;
    const std::unique_ptr<util::Thread<OnlineFileSourceThread>> thread;
};

OnlineFileRequest::OnlineFileRequest(Resource resource_,
                                     std::function<void(Response)>&& callback_,
                                     OnlineFileSourceThread& impl_)
    : impl(impl_),
      resource(std::move(resource_)),
      callback(std::move(callback_)) {
    impl.add(this);
}

OnlineFileRequest::~OnlineFileRequest() {
    if (mailbox) {
        mailbox->close();
    }
}

Timestamp interpolateExpiration(const Timestamp& current, std::optional<Timestamp> prior, bool& expired) {
    auto now = util::now();
    if (current > now) {
        return current;
    }

    if (!bool(prior)) {
        expired = true;
        return current;
    }

    // Expiring date is going backwards,
    // fallback to exponential backoff.
    if (current < *prior) {
        expired = true;
        return current;
    }

    auto delta = current - *prior;

    // Server is serving the same expired resource
    // over and over, fallback to exponential backoff.
    if (delta == Duration::zero()) {
        expired = true;
        return current;
    }

    // Assume that either the client or server clock is wrong and
    // try to interpolate a valid expiration date (from the client POV)
    // observing a minimum timeout.
    return now + std::max<Seconds>(delta, util::CLOCK_SKEW_RETRY_TIMEOUT);
}

void OnlineFileRequest::activate() {
    // Force an immediate first request if we don't have an expiration time.
    Duration timeout = Duration::zero();
    if (resource.priorExpires) {
        timeout = getUpdateInterval(resource.priorExpires);
    }
    schedule(timeout);
}

void OnlineFileRequest::schedule(Duration timeout) {
    if (impl.isPending(this) || impl.isActive(this)) {
        // There's already a request in progress; don't start another one.
        return;
    }

    if (timeout == Duration::max()) {
        return;
    }

    // Emulate a Connection error when the Offline mode is forced with
    // a really long timeout. The request will get re-triggered when
    // the NetworkStatus is set back to Online.
    if (NetworkStatus::Get() == NetworkStatus::Status::Offline) {
        failedRequestReason = Response::Error::Reason::Connection;
        failedRequests = 1;
        timeout = Duration::max();
    }

    timer.start(timeout, Duration::zero(), [&] { impl.activateOrQueueRequest(this); });
}

Duration OnlineFileRequest::getUpdateInterval(std::optional<Timestamp> expires) const {
    // Calculate a timeout that depends on how many
    // consecutive errors we've encountered, and on the expiration time, if present.
    Duration errorRetryTimeout = http::errorRetryTimeout(failedRequestReason, failedRequests, retryAfter);
    Duration expirationTimeout = std::max(http::expirationTimeout(std::move(expires), expiredRequests),
                                          resource.minimumUpdateInterval);
    return std::min(errorRetryTimeout, expirationTimeout);
}

namespace {

inline std::string sanitizeURL(std::string& url) {
    std::size_t queryIndex = url.find('?');
    return (queryIndex != std::string::npos) ? std::string(url, 0, queryIndex) : url;
}

} // namespace

void OnlineFileRequest::completed(Response response) {
    // If we didn't get various caching headers in the response, continue using the
    // previous values. Otherwise, update the previous values to the new values.

    if (!response.modified) {
        response.modified = resource.priorModified;
    } else {
        resource.priorModified = response.modified;
    }

    if (response.notModified && resource.priorData) {
        // When the priorData field is set, it indicates that we had to
        // revalidate the request and that the requestor hasn't gotten data yet.
        // If we get a 304 response, this means that we have send the cached
        // data to give the requestor a chance to actually obtain the data.
        response.data = std::move(resource.priorData);
        response.notModified = false;
    }

    bool isExpired = false;

    if (response.expires) {
        auto prior = resource.priorExpires;
        resource.priorExpires = response.expires;
        response.expires = interpolateExpiration(*response.expires, prior, isExpired);
    }

    if (isExpired) {
        expiredRequests++;
    } else {
        expiredRequests = 0;
    }

    if (!response.etag) {
        response.etag = resource.priorEtag;
    } else {
        resource.priorEtag = response.etag;
    }

    if (response.error) {
        if (response.error->reason == Response::Error::Reason::NotFound) {
            Log::Error(Event::General, "The resource `" + sanitizeURL(resource.url) + "` not found");
        }
        failedRequests++;
        failedRequestReason = response.error->reason;
        retryAfter = response.error->retryAfter;
    } else {
        failedRequests = 0;
        failedRequestReason = Response::Error::Reason::Success;
    }

    schedule(getUpdateInterval(response.expires));

    // Calling the callback may result in `this` being deleted. It needs to be
    // done last, and needs to make a local copy of the callback to ensure that
    // it remains valid for the duration of the call.
    auto callback_ = callback;
    callback_(response);
}

void OnlineFileRequest::networkIsReachableAgain() {
    // We need all requests to fail at least once before we are going to start retrying
    // them, and we only immediately restart request that failed due to connection issues.
    if (failedRequestReason == Response::Error::Reason::Connection) {
        schedule(Duration::zero());
    }
}

void OnlineFileRequest::setTransformedURL(const std::string& url) {
    resource.url = url;
    activate();
}

ActorRef<OnlineFileRequest> OnlineFileRequest::actor() {
    if (!mailbox) {
        // Lazy constructed because this can be costly and
        // the ResourceTransform is not used by many apps.
        mailbox = std::make_shared<Mailbox>(*Scheduler::GetCurrent());
    }

    return ActorRef<OnlineFileRequest>(*this, mailbox);
}

void OnlineFileRequest::onCancel(std::function<void()> callback_) {
    cancelCallback = std::move(callback_);
}

OnlineFileSource::OnlineFileSource(const ResourceOptions& resourceOptions, const ClientOptions& clientOptions)
    : impl(std::make_unique<Impl>(resourceOptions, clientOptions)) {}

OnlineFileSource::~OnlineFileSource() = default;

std::unique_ptr<AsyncRequest> OnlineFileSource::request(const Resource& resource,
                                                        std::function<void(Response)> callback) {
    Resource res = resource;
    const TileServerOptions options = impl->getResourceOptions().tileServerOptions();

    switch (resource.kind) {
        case Resource::Kind::Unknown:
        case Resource::Kind::Image:
            break;

        case Resource::Kind::Style:
            res.url = mbgl::util::mapbox::normalizeStyleURL(options, resource.url, impl->getApiKey());
            break;

        case Resource::Kind::Source:
            res.url = util::mapbox::normalizeSourceURL(options, resource.url, impl->getApiKey());
            break;

        case Resource::Kind::Glyphs:
            res.url = util::mapbox::normalizeGlyphsURL(options, resource.url, impl->getApiKey());
            break;

        case Resource::Kind::SpriteImage:
        case Resource::Kind::SpriteJSON:
            res.url = util::mapbox::normalizeSpriteURL(options, resource.url, impl->getApiKey());
            break;

        case Resource::Kind::Tile:
            res.url = util::mapbox::normalizeTileURL(options, resource.url, impl->getApiKey());
            break;
    }

    // Log::Error(Event::General, "Original: " + resource.url);
    // Log::Error(Event::General, "Normalized: " + res.url);

    return impl->request(std::move(callback), std::move(res));
}

bool OnlineFileSource::canRequest(const Resource& resource) const {
    return resource.hasLoadingMethod(Resource::LoadingMethod::Network) &&
           resource.url.rfind(mbgl::util::ASSET_PROTOCOL, 0) == std::string::npos &&
           resource.url.rfind(mbgl::util::FILE_PROTOCOL, 0) == std::string::npos;
}

void OnlineFileSource::pause() {
    impl->pause();
}

void OnlineFileSource::resume() {
    impl->resume();
}

void OnlineFileSource::setProperty(const std::string& key, const mapbox::base::Value& value) {
    if (key == API_KEY_KEY) {
        impl->setApiKey(value);
    } else if (key == API_BASE_URL_KEY) {
        impl->setAPIBaseURL(value);
    } else if (key == MAX_CONCURRENT_REQUESTS_KEY) {
        impl->setMaximumConcurrentRequests(value);
    } else if (key == ONLINE_STATUS_KEY) {
        // For testing only
        if (auto* boolValue = value.getBool()) {
            impl->setOnlineStatus(*boolValue);
        }
    } else {
        std::string message = "Resource provider does not support property " + key;
        Log::Error(Event::General, message.c_str());
    }
}

mapbox::base::Value OnlineFileSource::getProperty(const std::string& key) const {
    if (key == API_KEY_KEY) {
        return impl->getApiKey();
    } else if (key == API_BASE_URL_KEY) {
        return impl->getAPIBaseURL();
    } else if (key == MAX_CONCURRENT_REQUESTS_KEY) {
        return impl->getMaximumConcurrentRequests();
    }
    std::string message = "Resource provider does not support property " + key;
    Log::Error(Event::General, message.c_str());
    return {};
}

void OnlineFileSource::setResourceTransform(ResourceTransform transform) {
    impl->setResourceTransform(std::move(transform));
}

void OnlineFileSource::setResourceOptions(ResourceOptions options) {
    impl->setResourceOptions(options.clone());
}

ResourceOptions OnlineFileSource::getResourceOptions() {
    return impl->getResourceOptions();
}

void OnlineFileSource::setClientOptions(ClientOptions options) {
    impl->setClientOptions(options.clone());
}

ClientOptions OnlineFileSource::getClientOptions() {
    return impl->getClientOptions();
}

} // namespace mbgl
