#include <mln/storage/database_file_source.hpp>
#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/test/util.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/timer.hpp>

#include <cstdio>
#include <random>
#include <string>

#include <gtest/gtest.h>

using namespace mln;

TEST(DatabaseFileSource, PauseResume) {
    util::RunLoop loop;

    std::shared_ptr<FileSource> dbfs = FileSourceManager::get()->getFileSource(FileSourceType::Database,
                                                                               ResourceOptions{});
    dbfs->pause();

    const Resource res{Resource::Unknown, "http://127.0.0.1:3000/test", {}, Resource::LoadingMethod::CacheOnly};
    auto req = dbfs->request(res, [&](const Response&) { loop.stop(); });

    util::Timer resumeTimer;
    resumeTimer.start(Milliseconds(5), Duration::zero(), [dbfs] { dbfs->resume(); });

    loop.run();
}

TEST(DatabaseFileSource, VolatileResource) {
    util::RunLoop loop;

    std::shared_ptr<FileSource> dbfs = FileSourceManager::get()->getFileSource(FileSourceType::Database,
                                                                               ResourceOptions{});

    Resource resource{Resource::Unknown, "http://127.0.0.1:3000/test", {}, Resource::LoadingMethod::CacheOnly};
    Response response{};
    response.data = std::make_shared<std::string>("Cached value");
    std::unique_ptr<mln::AsyncRequest> req;

    dbfs->forward(resource, response, [&] {
        req = dbfs->request(resource, [&](Response res1) {
            EXPECT_EQ(nullptr, res1.error);
            ASSERT_TRUE(res1.data.get());
            EXPECT_FALSE(res1.noContent);
            EXPECT_EQ("Cached value", *res1.data);
            resource.storagePolicy = Resource::StoragePolicy::Volatile;
            req = dbfs->request(resource, [&](Response res2) {
                req.reset();
                ASSERT_TRUE(res2.error.get());
                EXPECT_TRUE(res2.noContent);
                EXPECT_EQ(Response::Error::Reason::NotFound, res2.error->reason);
                EXPECT_EQ("Not found in offline database", res2.error->message);
                loop.stop();
            });
        });
    });
    loop.run();
}

TEST(DatabaseFileSource, VolatileResourceIsNotStored) {
    util::RunLoop loop;

    std::shared_ptr<FileSource> dbfs = FileSourceManager::get()->getFileSource(FileSourceType::Database,
                                                                               ResourceOptions{});

    Resource resource{
        Resource::Unknown, "http://127.0.0.1:3000/volatile-not-stored", {}, Resource::LoadingMethod::CacheOnly};
    resource.storagePolicy = Resource::StoragePolicy::Volatile;
    Response response{};
    response.data = std::make_shared<std::string>("Volatile value");

    // Forwarding a volatile response must not write it to the database,
    // so a regular request for the same resource finds nothing.
    dbfs->forward(resource, response, nullptr);
    resource.storagePolicy = Resource::StoragePolicy::Permanent;
    std::unique_ptr<mln::AsyncRequest> req = dbfs->request(resource, [&](Response res) {
        req.reset();
        loop.stop();
        EXPECT_TRUE(res.noContent);
        ASSERT_TRUE(res.error.get());
        EXPECT_EQ(Response::Error::Reason::NotFound, res.error->reason);
    });
    loop.run();
}

namespace {

Resource cachedResource(int index) {
    return {Resource::Unknown,
            "http://127.0.0.1:3000/lru/" + std::to_string(index),
            {},
            Resource::LoadingMethod::CacheOnly};
}

} // namespace

// A lookup cancelled while it waits behind other work on the database thread
// must not touch the database. Reading a cached resource marks it as recently
// used, which protects it from eviction, so a cancelled lookup that still
// reads would keep a resource nobody asked for in the cache.
TEST(DatabaseFileSource, CancelledRequestDoesNotRefreshTheCache) {
    util::RunLoop loop;

    const std::string path = "test/fixtures/offline_database/cancelled_request.db";
    std::remove(path.c_str());
    std::remove((path + "-journal").c_str());

    auto dbfs = std::static_pointer_cast<DatabaseFileSource>(
        FileSourceManager::get()->getFileSource(FileSourceType::Database, ResourceOptions{}.withCachePath(path)));

    // Random bytes, so that compression does not shrink them.
    std::string bytes(1024, 0);
    std::mt19937 random;
    for (char& c : bytes) {
        c = static_cast<char>(random());
    }
    Response response{};
    response.data = std::make_shared<std::string>(std::move(bytes));

    // Eviction removes the oldest 50 entries at a time and recency is kept in
    // whole seconds, so store 50 resources, then 10 more a second later.
    constexpr int oldCount = 50;
    constexpr int newCount = 10;
    constexpr int cancelledIndex = 0;
    constexpr int deliveredIndex = 1;
    const Duration pause = Milliseconds(1100);

    bool answered = false;
    bool cancelledFound = true;
    bool deliveredFound = false;
    std::unique_ptr<AsyncRequest> cancelled;
    std::unique_ptr<AsyncRequest> delivered;
    std::unique_ptr<AsyncRequest> probe;
    util::Timer firstTimer;
    util::Timer secondTimer;

    // The steps below run in the reverse of the order they are defined in.

    // 4. Look both resources up again: the cancelled one should be gone.
    auto probeCache = [&] {
        probe = dbfs->request(cachedResource(cancelledIndex), [&](const Response& first) {
            cancelledFound = !first.error && first.data;
            probe = dbfs->request(cachedResource(deliveredIndex), [&](const Response& second) {
                deliveredFound = !second.error && second.data;
                probe.reset();
                loop.stop();
            });
        });
    };

    // 3. Hold the database thread, queue a lookup of one old resource and
    // cancel it before the thread can run it. A lookup of a second old
    // resource is answered, which shows that a read does protect a resource
    // from eviction. Then shrink the cache so that eviction runs once.
    auto lookUp = [&] {
        dbfs->pause();
        cancelled = dbfs->request(cachedResource(cancelledIndex), [&](const Response&) { answered = true; });
        delivered = dbfs->request(cachedResource(deliveredIndex), [&](const Response&) {
            delivered.reset();
            dbfs->setMaximumAmbientCacheSize(20 * 1024, [&](std::exception_ptr) { probeCache(); });
        });
        cancelled.reset();
        dbfs->resume();
    };

    // 2. A second later, store the newer resources.
    auto storeNew = [&] {
        for (int i = 0; i < newCount - 1; ++i) {
            dbfs->forward(cachedResource(oldCount + i), response, nullptr);
        }
        dbfs->forward(cachedResource(oldCount + newCount - 1), response, [&] {
            secondTimer.start(pause, Duration::zero(), lookUp);
        });
    };

    // 1. Store the older resources.
    for (int i = 0; i < oldCount - 1; ++i) {
        dbfs->forward(cachedResource(i), response, nullptr);
    }
    dbfs->forward(cachedResource(oldCount - 1), response, [&] { firstTimer.start(pause, Duration::zero(), storeNew); });
    loop.run();

    EXPECT_FALSE(answered);
    EXPECT_FALSE(cancelledFound) << "the cancelled lookup kept its resource in the cache";
    EXPECT_TRUE(deliveredFound);

    dbfs.reset();
    std::remove(path.c_str());
}
