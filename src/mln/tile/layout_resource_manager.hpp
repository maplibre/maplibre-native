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

// Bytes that a layout loads by URL before it builds its buckets, shared by every tile that uses the URL.
// Layer types turn the bytes into their own objects once, through getOrCreate, and those objects live as long
// as the resource.
class LayoutResource {
public:
    explicit LayoutResource(std::shared_ptr<const std::string> data_)
        : data(std::move(data_)) {}

    // Null when the URL could not be loaded.
    const std::shared_ptr<const std::string> data;

    // What `create` makes from the bytes for `owner`, made on the first call and returned by every later one,
    // from any thread. A failed load or a null result is kept too, so it is not made again.
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
    // Some or all of the requested resources, possibly in several calls.
    virtual void onLayoutResourcesAvailable(LayoutResourceMap) = 0;
};

// Loads layout resources through the file source on the render thread and hands them to the tiles that asked.
// Each URL is requested once; a failed load is handed out as a resource without data and requested again by
// the next tile that asks for it.
class LayoutResourceManager {
public:
    LayoutResourceManager();
    ~LayoutResourceManager();

    void getResources(LayoutResourceRequestor&, const LayoutResourceDependencies&, FileSource*);
    void removeRequestor(LayoutResourceRequestor&);

private:
    struct Entry {
        std::shared_ptr<const LayoutResource> resource;
        std::unique_ptr<AsyncRequest> request;
        std::set<LayoutResourceRequestor*> requestors;
        // The last response was an error; the next tile that asks requests the URL again.
        bool failed = false;
    };

    void onResponse(const std::string& url, const Response&);

    std::map<std::string, Entry> entries;
};

} // namespace mln
