#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace mln {

class AsyncRequest;
class FileSource;
class Response;

// The bytes of a URL that a layout needs before it builds its buckets, shared by the tiles that use the URL.
// A layer type makes its own object from the bytes once, through getOrCreate. The objects live as long as the
// resource, which lives until the last tile that uses it lets it go.
class LayoutResource {
public:
    explicit LayoutResource(std::shared_ptr<const std::string> data_)
        : data(std::move(data_)) {}

    // Null when the URL could not be loaded.
    const std::shared_ptr<const std::string> data;

    // The object that `create` makes from the bytes for `owner`: made by the first call, on any thread, and
    // returned by every later call. A null result is kept too, so `create` runs at most once per owner.
    std::shared_ptr<void> getOrCreate(const void* owner,
                                      const std::function<std::shared_ptr<void>(const std::string&)>& create) const;

private:
    mutable std::mutex mutex;
    mutable std::map<const void*, std::shared_ptr<void>> created;
};

using LayoutResourceDependencies = std::set<std::string>;
using LayoutResourceMap = std::map<std::string, std::shared_ptr<const LayoutResource>>;

class LayoutResourceRequestor {
public:
    virtual ~LayoutResourceRequestor() = default;
    // Some or all of the requested resources, in one or more calls.
    virtual void onLayoutResourcesAvailable(LayoutResourceMap) = 0;
};

// Loads layout resources through the file source and hands them to the tiles that asked for them. Runs on the
// render thread. A URL is requested once while a tile uses its resource. A failed load is handed out as a
// resource without data, and the next tile that asks requests the URL again.
class LayoutResourceManager {
public:
    LayoutResourceManager();
    ~LayoutResourceManager();

    void getResources(LayoutResourceRequestor&, const LayoutResourceDependencies&, FileSource*);
    void removeRequestor(LayoutResourceRequestor&);

private:
    struct Entry {
        // The tiles hold the resource; the entry only finds it while one of them does.
        std::weak_ptr<const LayoutResource> resource;
        std::unique_ptr<AsyncRequest> request;
        std::set<LayoutResourceRequestor*> requestors;
        bool loaded = false;
        bool failed = false;
    };

    void onResponse(const std::string& url, const Response&);
    // Drops the entries that no tile waits for or uses.
    void prune();

    std::map<std::string, Entry> entries;
};

} // namespace mln
