#include <mln/storage/database_file_source.hpp>
#include <mln/storage/file_source_manager.hpp>
#include <mln/storage/offline_database.hpp>
#include <mln/storage/sqlite3.hpp>
#include <mln/test/fixture_log_observer.hpp>
#include <mln/util/io.hpp>
#include <mln/util/scoped.hpp>
#include <mln/storage/resource.hpp>
#include <mln/storage/resource_options.hpp>
#include <mln/test/util.hpp>
#include <mln/util/run_loop.hpp>
#include <mln/util/timer.hpp>

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

// Exercise the real database actor and timer: a failed best-effort LRU update must
// not keep retrying while idle, but the next successful read must retry it.
TEST(DatabaseFileSource, FailedAccessedFlushWaitsForNextRead) {
    util::RunLoop loop;
    FixtureLog log;
    const std::string path = "test/fixtures/offline_database/maintenance.db";
    const auto cleanup = [&] {
        util::deleteFile(path);
        util::deleteFile(path + "-wal");
        util::deleteFile(path + "-journal");
    };
    cleanup();
    const Scoped cleanupGuard([&] { cleanup(); });
    const Resource resource{
        Resource::Unknown, "https://example.com/maintenance", {}, Resource::LoadingMethod::CacheOnly};
    Response response;
    response.data = std::make_shared<std::string>("cached");
    {
        OfflineDatabase seed(path, ResourceOptions{}.tileServerOptions());
        seed.put(resource, response);
    }
    auto sql = mapbox::sqlite::Database::open(path, mapbox::sqlite::ReadWriteCreate);
    sql.exec("UPDATE resources SET accessed = 0");
    sql.exec(
        "CREATE TRIGGER fail_accessed BEFORE UPDATE OF accessed ON resources "
        "BEGIN SELECT RAISE(ABORT, 'accessed failure'); END");
    DatabaseFileSource source(ResourceOptions{}.withCachePath(path), ClientOptions{});
    util::Timer timeout;
    timeout.start(Seconds(5), Duration::zero(), [&] {
        ADD_FAILURE() << "Timed out waiting for accessed maintenance";
        loop.stop();
    });
    util::Timer failurePoll;
    util::Timer idle;
    util::Timer recovery;
    std::unique_ptr<AsyncRequest> request;
    request = source.request(resource, [&](const Response& cached) {
        EXPECT_FALSE(cached.error);
        ASSERT_TRUE(cached.data);
        EXPECT_EQ(*response.data, *cached.data);
    });
    const FixtureLog::Message warning{EventSeverity::Warning,
                                      Event::Database,
                                      static_cast<int64_t>(mapbox::sqlite::ResultCode::Constraint),
                                      "Can't flush accessed timestamps"};
    bool failureSeen = false;
    failurePoll.start(Milliseconds(20), Milliseconds(20), [&] {
        if (failureSeen) return;
        const auto failures = log.count(warning, true);
        if (!failures) return;
        failureSeen = true;
        EXPECT_EQ(1u, failures);
        idle.start(Milliseconds(800), Duration::zero(), [&] {
            failurePoll.stop();
            EXPECT_EQ(0u, log.count(warning, true)) << "Failed flush retried while idle";
            sql.exec("DROP TRIGGER fail_accessed");
            request = source.request(resource, [&](const Response& cached) {
                EXPECT_FALSE(cached.error);
                ASSERT_TRUE(cached.data);
                EXPECT_EQ(*response.data, *cached.data);
                recovery.start(Milliseconds(500), Duration::zero(), [&] {
                    mapbox::sqlite::Statement statement{sql, "SELECT accessed FROM resources"};
                    mapbox::sqlite::Query query{statement};
                    ASSERT_TRUE(query.run());
                    EXPECT_GT(query.get<int64_t>(0), 0);
                    EXPECT_EQ(0u, log.uncheckedCount());
                    loop.stop();
                });
            });
        });
    });
    loop.run();
}
