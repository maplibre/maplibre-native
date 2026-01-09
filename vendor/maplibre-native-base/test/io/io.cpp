#include "mapbox/io/io.hpp"

#include <gtest/gtest.h>

#include <cassert>
#include <string>

#include "io_delete.hpp"
#include "mapbox/util/expected.hpp"
#include "test_defines.hpp"

TEST(io, ReadWriteFiles) {
    const std::string path(std::string(TEST_BINARY_PATH) + "/foo.txt");
    const std::string copyPath(std::string(TEST_BINARY_PATH) + "/bar.txt");
    const std::string invalidPath("invalid");
    const std::string unauthorizedPath("/root/unauthorized");
    const std::string bar("bar");

    mapbox::base::expected<void, std::string> voidExpected = mapbox::base::io::writeFile(path, bar);
    EXPECT_TRUE(voidExpected);

    voidExpected = mapbox::base::io::writeFile(unauthorizedPath, bar);
    EXPECT_FALSE(voidExpected);
    EXPECT_EQ(voidExpected.error(), std::string("Failed to write file '/root/unauthorized'"));

    nonstd::expected<std::string, std::string> stringExpected = mapbox::base::io::readFile(path);
    EXPECT_TRUE(stringExpected);
    EXPECT_EQ(*stringExpected, bar);

    stringExpected = mapbox::base::io::readFile(invalidPath);
    EXPECT_FALSE(stringExpected);
    EXPECT_EQ(stringExpected.error(), std::string("Failed to read file 'invalid'"));

    voidExpected = mapbox::base::io::copyFile(path, copyPath);
    EXPECT_TRUE(voidExpected);

    stringExpected = mapbox::base::io::readFile(copyPath);
    EXPECT_EQ(*stringExpected, bar);

    voidExpected = mapbox::base::io::copyFile(path, unauthorizedPath);
    EXPECT_FALSE(voidExpected);
    EXPECT_EQ(voidExpected.error(), std::string("Failed to write file '/root/unauthorized'"));

    voidExpected = mapbox::base::io::copyFile(invalidPath, path);
    EXPECT_FALSE(voidExpected);
    EXPECT_EQ(voidExpected.error(), std::string("Failed to read file 'invalid'"));

    deleteTests(path, copyPath, invalidPath);
}
