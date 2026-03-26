#include <mbgl/renderer/image_manager.hpp>

#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/image_manager_observer.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

#include <sstream>

namespace mbgl {

namespace {
ImageManagerObserver nullObserver;
} // namespace

ImageManager::ImageManager() = default;

ImageManager::~ImageManager() = default;

void ImageManager::setObserver(ImageManagerObserver* observer_) {
    observer = observer_ ? observer_ : &nullObserver;
}

void ImageManager::setLoaded(bool loaded_) {
    std::scoped_lock readWriteLock(rwLock);
    if (loaded == loaded_) {
        return;
    }

    loaded = loaded_;

    if (loaded) {
        for (const auto& entry : requestors) {
            checkMissingAndNotify(*entry.first, entry.second);
        }

        requestors.clear();
    }
}

bool ImageManager::isLoaded() const {
    return loaded;
}

void ImageManager::addImage(Immutable<style::Image::Impl> image_) {
    std::scoped_lock readLock(rwLock);
    assert(!images.contains(image_->id));

    // Increase cache size if requested image was provided.
    if (requestedImages.contains(image_->id)) {
        requestedImagesCacheSize += image_->image.bytes();
    }

    availableImages.emplace(image_->id);
    images.emplace(image_->id, std::move(image_));
}

bool ImageManager::updateImage(Immutable<style::Image::Impl> image_) {
    std::scoped_lock readWriteLock(rwLock);

    auto oldImage = images.find(image_->id);
    assert(oldImage != images.end());
    if (oldImage == images.end()) return false;

    auto sizeChanged = oldImage->second->image.size != image_->image.size;

    if (sizeChanged) {
        // Update cache size if requested image size has changed.
        if (requestedImages.contains(image_->id)) {
            int64_t diff = image_->image.bytes() - oldImage->second->image.bytes();
            assert(static_cast<int64_t>(requestedImagesCacheSize + diff) >= 0ll);
            requestedImagesCacheSize += diff;
        }
        updatedImageVersions.erase(image_->id);
    } else {
        updatedImageVersions[image_->id]++;
    }

    oldImage->second = std::move(image_);

    return sizeChanged;
}

void ImageManager::removeImage(const std::string& id) {
    std::scoped_lock readWriteLock(rwLock);
    auto it = images.find(id);
    assert(it != images.end());

    // Reduce cache size for requested images.
    auto requestedIt = requestedImages.find(it->second->id);
    if (requestedIt != requestedImages.end()) {
        assert(requestedImagesCacheSize >= it->second->image.bytes());
        requestedImagesCacheSize -= it->second->image.bytes();
        requestedImages.erase(requestedIt);
    }

    images.erase(it);
    availableImages.erase(id);
    updatedImageVersions.erase(id);
}

const style::Image::Impl* ImageManager::getImage(const std::string& id) const {
    std::scoped_lock readWriteLock(rwLock);
    if (auto* image = getSharedImage(id)) {
        return image->get();
    }
    return nullptr;
}

const Immutable<style::Image::Impl>* ImageManager::getSharedImage(const std::string& id) const {
    std::scoped_lock readWriteLock(rwLock);
    const auto it = images.find(id);
    if (it != images.end()) {
        return &(it->second);
    }
    return nullptr;
}

void ImageManager::getImages(ImageRequestor& requestor, ImageRequestPair&& pair) {
    // remove previous requests from this tile
    removeRequestor(requestor);

    std::scoped_lock readWriteLock(rwLock);

    // If all the icon dependencies are already present ((i.e. if they've been addeded via
    // runtime styling), then notify the requestor immediately. Otherwise, if the
    // sprite has not loaded, then wait for it. When the sprite has loaded check
    // if all icons are available. If any are missing, call `onStyleImageMissing`
    // to give the user a chance to provide the icon. If they are not provided
    // by the next frame we'll assume they are permanently missing.
    if (!isLoaded()) {
        bool hasAllDependencies = true;
        for (const auto& dependency : pair.first) {
            if (!images.contains(dependency.first)) {
                hasAllDependencies = false;
                break;
            }
        }

        if (hasAllDependencies) {
            notify(requestor, pair);
        } else {
            requestors.emplace(&requestor, std::move(pair));
        }
    } else {
        checkMissingAndNotify(requestor, pair);
    }
}

void ImageManager::removeRequestor(ImageRequestor& requestor) {
    std::scoped_lock readWriteLock(rwLock);

    requestors.erase(&requestor);
    missingImageRequestors.erase(&requestor);
    for (auto& requestedImage : requestedImages) {
        requestedImage.second.erase(&requestor);
    }
}

void ImageManager::notifyIfMissingImageAdded() {
    std::scoped_lock readWriteLock(rwLock);

    for (auto it = missingImageRequestors.begin(); it != missingImageRequestors.end();) {
        ImageRequestor& requestor = *it->first;
        if (!requestor.hasPendingRequests()) {
            notify(requestor, it->second);
            it = missingImageRequestors.erase(it);
        } else {
            ++it;
        }
    }
}

void ImageManager::reduceMemoryUse() {
    std::scoped_lock readLock(rwLock);

    std::vector<std::string> unusedIDs;
    unusedIDs.reserve(requestedImages.size());

    for (const auto& pair : requestedImages) {
        if (pair.second.empty() && images.contains(pair.first)) {
            unusedIDs.push_back(pair.first);
        }
    }

    if (!unusedIDs.empty()) {
        observer->onRemoveUnusedStyleImages(unusedIDs);
    }
}

void ImageManager::reduceMemoryUseIfCacheSizeExceedsLimit() {
    if (requestedImagesCacheSize > util::DEFAULT_ON_DEMAND_IMAGES_CACHE_SIZE) {
        MLN_TRACE_FUNC();
        reduceMemoryUse();
    }
}

std::set<std::string> ImageManager::getAvailableImages() const {
    MLN_TRACE_FUNC();
    std::scoped_lock readWriteLock(rwLock);

    {
        MLN_TRACE_ZONE(copy);
        return availableImages;
    }
}

void ImageManager::clear() {
    std::scoped_lock readWriteLock(rwLock);

    assert(requestors.empty());
    assert(missingImageRequestors.empty());

    images.clear();
    availableImages.clear();
    updatedImageVersions.clear();
    requestedImages.clear();
    loaded = false;
}

void ImageManager::checkMissingAndNotify(ImageRequestor& requestor, const ImageRequestPair& pair) {
    ImageDependencies missingDependencies;

    for (const auto& dependency : pair.first) {
        if (!images.contains(dependency.first)) {
            missingDependencies.emplace(dependency);
        }
    }

    if (!missingDependencies.empty()) {
        ImageRequestor* requestorPtr = &requestor;
        assert(!missingImageRequestors.contains(requestorPtr));
        missingImageRequestors.emplace(requestorPtr, pair);

        for (const auto& dependency : missingDependencies) {
            const std::string& missingImage = dependency.first;
            assert(observer != nullptr);

            auto existingRequestorsIt = requestedImages.find(missingImage);
            if (existingRequestorsIt != requestedImages.end()) { // Already asked client about this image.
                std::set<ImageRequestor*>& existingRequestors = existingRequestorsIt->second;
                // existingRequestors is empty if all the previous requestors are deleted.
                if (!existingRequestors.empty() &&
                    (*existingRequestors.begin())
                        ->hasPendingRequest(missingImage)) { // Still waiting for the client response for this image.
                    requestorPtr->addPendingRequest(missingImage);
                    existingRequestors.emplace(requestorPtr);
                    continue;
                }
                // The request for this image has been already delivered
                // to the client, so we do not treat it as pending.
                existingRequestors.emplace(requestorPtr);
                // TODO: we could `continue;` here, but we need to call
                // `observer->onStyleImageMissing`, so that rendering is
                // re-launched from the handler at Map::Impl.
            } else {
                requestedImages[missingImage].emplace(requestorPtr);
                requestor.addPendingRequest(missingImage);
            }

            auto removePendingRequests = [this, missingImage] {
                std::scoped_lock readWriteLock(rwLock);
                auto existingRequest = requestedImages.find(missingImage);
                if (existingRequest == requestedImages.end()) {
                    return;
                }

                for (auto* req : existingRequest->second) {
                    req->removePendingRequest(missingImage);
                }
            };
            observer->onStyleImageMissing(missingImage,
                                          Scheduler::GetCurrent()->bindOnce(std::move(removePendingRequests)));
        }
    } else {
        // Associate requestor with an image that was provided by the client.
        for (const auto& dependency : pair.first) {
            if (requestedImages.contains(dependency.first)) {
                requestedImages[dependency.first].emplace(&requestor);
            }
        }
        notify(requestor, pair);
    }
}

void ImageManager::notify(ImageRequestor& requestor, const ImageRequestPair& pair) const {
    ImageMap iconMap;
    ImageMap patternMap;
    ImageVersionMap versionMap;

    iconMap.reserve(pair.first.size());
    patternMap.reserve(pair.first.size());
    versionMap.reserve(pair.first.size());

    for (const auto& dependency : pair.first) {
        auto it = images.find(dependency.first);
        if (it != images.end()) {
            dependency.second == ImageType::Pattern ? patternMap.emplace(*it) : iconMap.emplace(*it);

            auto versionIt = updatedImageVersions.find(dependency.first);
            if (versionIt != updatedImageVersions.end()) {
                versionMap.emplace(versionIt->first, versionIt->second);
            }
        }
    }

    requestor.onImagesAvailable(std::move(iconMap), std::move(patternMap), std::move(versionMap), pair.second);
}

void ImageManager::dumpDebugLogs() const {
    Log::Info(Event::General, "ImageManager::loaded: " + std::string(loaded ? "1" : "0"));
}

ImageRequestor::ImageRequestor(std::shared_ptr<ImageManager> imageManager_)
    : imageManager(std::move(imageManager_)) {}

ImageRequestor::~ImageRequestor() {
    imageManager->removeRequestor(*this);
}

} // namespace mbgl
