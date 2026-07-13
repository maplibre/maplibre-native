#include "io_delete.hpp"

#include <gtest/gtest.h>

#include <cassert>
#include <mapbox/io/io.hpp>
#include <string>

void deleteTests(const std::string& path, const std::string& copyPath, const std::string& invalidPath) {
    nonstd::expected<void, std::string> voidExpected;

    voidExpected = mapbox::base::io::deleteFile(path);
    EXPECT_TRUE(voidExpected);

    voidExpected = mapbox::base::io::deleteFile(copyPath);
    EXPECT_TRUE(voidExpected);

    voidExpected = mapbox::base::io::deleteFile(invalidPath);
    EXPECT_FALSE(voidExpected);
    EXPECT_EQ(voidExpected.error(), std::string("Failed to delete file 'invalid'"));
}
