#include <mln/tile/layout_resource_manager.hpp>

#include <mln/storage/file_source.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/response.hpp>
#include <mln/util/async_request.hpp>
#include <mln/util/logging.hpp>

namespace mln {

std::shared_ptr<void> LayoutResource::getOrCreate(
    const void* owner, const std::function<std::shared_ptr<void>(const std::string&)>& create) const {
    std::lock_guard lock(mutex);
    const auto found = created.find(owner);
    if (found != created.end()) return found->second;
    auto made = data ? create(*data) : nullptr;
    created.emplace(owner, made);
    return made;
}

LayoutResourceManager::LayoutResourceManager() = default;

LayoutResourceManager::~LayoutResourceManager() = default;

void LayoutResourceManager::getResources(LayoutResourceRequestor& requestor,
                                         const LayoutResourceDependencies& dependencies,
                                         FileSource* fileSource) {
    LayoutResourceMap ready;
    for (const auto& url : dependencies) {
        auto& entry = entries[url];
        if (entry.resource && !entry.failed) {
            ready.emplace(url, entry.resource);
            continue;
        }
        if (!fileSource) {
            ready.emplace(url, std::make_shared<const LayoutResource>(nullptr));
            continue;
        }
        entry.requestors.insert(&requestor);
        if (entry.failed || !entry.request) {
            entry.resource.reset();
            entry.failed = false;
            entry.request = fileSource->request(Resource(Resource::Kind::Unknown, url),
                                                [this, url](const Response& response) { onResponse(url, response); });
        }
    }
    if (!ready.empty()) requestor.onLayoutResourcesAvailable(std::move(ready));
}

void LayoutResourceManager::removeRequestor(LayoutResourceRequestor& requestor) {
    for (auto& [url, entry] : entries) entry.requestors.erase(&requestor);
}

void LayoutResourceManager::onResponse(const std::string& url, const Response& response) {
    const auto found = entries.find(url);
    if (found == entries.end() || response.notModified) return;
    auto& entry = found->second;

    std::shared_ptr<const std::string> data;
    entry.failed = static_cast<bool>(response.error);
    if (response.error) {
        Log::Warning(Event::Style, "Failed to load layout resource '" + url + "': " + response.error->message);
    } else if (!response.noContent) {
        data = response.data;
    }
    entry.resource = std::make_shared<const LayoutResource>(std::move(data));

    const auto requestors = std::move(entry.requestors);
    entry.requestors.clear();
    for (auto* requestor : requestors) requestor->onLayoutResourcesAvailable({{url, entry.resource}});
}

} // namespace mln
