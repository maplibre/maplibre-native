#include <mbgl/actor/actor.hpp>
#include <mbgl/actor/scheduler.hpp>
#include <mbgl/sprite/sprite_loader.hpp>
#include <mbgl/sprite/sprite_loader_observer.hpp>
#include <mbgl/sprite/sprite_parser.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/async_request.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/std.hpp>

#include <cassert>

namespace mbgl {

namespace {
SpriteLoaderObserver nullObserver;
}

struct SpriteLoader::Data {
    std::shared_ptr<const std::string> image;
    std::shared_ptr<const std::string> json;
    std::unique_ptr<AsyncRequest> jsonRequest;
    std::unique_ptr<AsyncRequest> spriteRequest;
};

SpriteLoader::SpriteLoader(float pixelRatio_, const TaggedScheduler& threadPool_)
    : pixelRatio(pixelRatio_),
      observer(&nullObserver),
      threadPool(threadPool_) {}

SpriteLoader::~SpriteLoader() = default;

void SpriteLoader::load(const std::optional<style::Sprite> sprite, FileSource& fileSource) {
    if (!sprite) {
        // Treat a non-existent sprite as a successfully loaded empty sprite.
        observer->onSpriteLoaded(std::nullopt, {});
        return;
    }

    observer->onSpriteRequested(sprite);

    std::string id = sprite->id;
    std::string url = sprite->spriteURL;

    dataMap[id] = std::make_unique<Data>();
    dataMap[id]->jsonRequest = fileSource.request(Resource::spriteJSON(url, pixelRatio), [this, sprite](Response res) {
        std::scoped_lock lock(dataMapMutex);
        Data* data = dataMap[sprite->id].get();
        if (res.error) {
            observer->onSpriteError(sprite, std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            data->json = std::make_shared<std::string>();
            emitSpriteLoadedIfComplete(*sprite);
        } else {
            // Only trigger a sprite loaded event we got new data.
            assert(data->json != res.data);
            data->json = std::move(res.data);
            emitSpriteLoadedIfComplete(*sprite);
        }
    });

    dataMap[id]->spriteRequest = fileSource.request(
        Resource::spriteImage(url, pixelRatio), [this, sprite](Response res) {
            std::scoped_lock lock(dataMapMutex);
            Data* data = dataMap[sprite->id].get();
            if (res.error) {
                observer->onSpriteError(sprite, std::make_exception_ptr(std::runtime_error(res.error->message)));
            } else if (res.notModified) {
                return;
            } else if (res.noContent) {
                data->image = std::make_shared<std::string>();
                emitSpriteLoadedIfComplete(*sprite);
            } else {
                assert(dataMap[sprite->id]->image != res.data);
                data->image = std::move(res.data);
                emitSpriteLoadedIfComplete(*sprite);
            }
        });
}

void SpriteLoader::emitSpriteLoadedIfComplete(style::Sprite sprite) {
    Data* data = dataMap[sprite.id].get();
    assert(data);
    if (!data->image || !data->json) {
        return;
    }

    struct ParseResult {
        std::vector<Immutable<style::Image::Impl>> images;
        std::exception_ptr error;
    };

    threadPool.scheduleAndReplyValue(
        /* parseClosure */
        [sprite = sprite, image = data->image, json = data->json]() -> ParseResult {
            try {
                return {.images = parseSprite(sprite.id, *image, *json), .error = nullptr};
            } catch (...) {
                return {.images = {}, .error = std::current_exception()};
            }
        },
        /* resultClosure */
        [this, sprite = sprite, factory = weakFactory.makeWeakPtr()](ParseResult result) {
            if (auto guard = factory.lock(); factory) {
                if (result.error) {
                    observer->onSpriteError(std::optional(sprite), result.error);
                } else {
                    observer->onSpriteLoaded(std::optional(sprite), std::move(result.images));
                }
            }
        });
}

void SpriteLoader::setObserver(SpriteLoaderObserver* observer_) {
    observer = observer_;
}

} // namespace mbgl
