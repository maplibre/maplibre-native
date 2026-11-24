#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/storage/sqlite3.hpp>
#include <mln/test/util.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/timer.hpp>

#include <cstdint>
#include <cstdio>
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

int64_t accessedTime(const std::string& path) {
    mapbox::sqlite::Database db = mapbox::sqlite::Database::open(path, mapbox::sqlite::ReadOnly);
    mapbox::sqlite::Statement stmt{db, "SELECT accessed FROM resources WHERE url LIKE '%/cancelled'"};
    mapbox::sqlite::Query query{stmt};
    return query.run() ? query.get<int64_t>(0) : -1;
}

} // namespace

// A lookup cancelled while it waits behind other work on the database thread
// must not read the database. The read is visible from outside because it
// stamps the row's access time.
TEST(DatabaseFileSource, CancelledRequestDoesNotReadTheDatabase) {
    util::RunLoop loop;

    const std::string path = "test/fixtures/offline_database/cancelled_request.db";
    std::remove(path.c_str());
    std::remove((path + "-journal").c_str());

    std::shared_ptr<FileSource> dbfs = FileSourceManager::get()->getFileSource(FileSourceType::Database,
                                                                               ResourceOptions{}.withCachePath(path));

    const Resource resource{
        Resource::Unknown, "http://127.0.0.1:3000/cancelled", {}, Resource::LoadingMethod::CacheOnly};
    Response response{};
    response.data = std::make_shared<std::string>("Cached value");

    bool answered = false;
    int64_t accessedAfterCancelled = -1;
    int64_t accessedAfterDelivered = -1;
    std::unique_ptr<AsyncRequest> delivered;

    dbfs->forward(resource, response, [&] {
        // Hold the database thread, clear the access time, then queue a
        // lookup and cancel it before the thread can run it.
        dbfs->pause();
        {
            mapbox::sqlite::Database db = mapbox::sqlite::Database::open(path, mapbox::sqlite::ReadWriteCreate);
            db.exec("UPDATE resources SET accessed = 0");
        }
        ASSERT_NE(-1, accessedTime(path));
        auto cancelled = dbfs->request(resource, [&](const Response&) { answered = true; });
        cancelled.reset();
        dbfs->resume();

        // A write of another resource is queued behind the cancelled lookup,
        // so its callback runs after the thread has dealt with it.
        const Resource marker{
            Resource::Unknown, "http://127.0.0.1:3000/marker", {}, Resource::LoadingMethod::CacheOnly};
        dbfs->forward(marker, response, [&] {
            accessedAfterCancelled = accessedTime(path);

            // The same lookup, not cancelled, does stamp the access time,
            // which shows the check above can tell the difference.
            delivered = dbfs->request(resource, [&](const Response& res) {
                EXPECT_EQ(nullptr, res.error);
                accessedAfterDelivered = accessedTime(path);
                delivered.reset();
                loop.stop();
            });
        });
    });
    loop.run();

    EXPECT_FALSE(answered);
    EXPECT_EQ(0, accessedAfterCancelled);
    EXPECT_GT(accessedAfterDelivered, 0);

    dbfs.reset();
    std::remove(path.c_str());
}
