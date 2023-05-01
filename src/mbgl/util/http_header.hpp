#pragma once

#include <mbgl/util/chrono.hpp>

#include <string>
#include <optional>

namespace mbgl {
namespace http {

class CacheControl {
public:
    static CacheControl parse(const std::string&);

    std::optional<uint64_t> maxAge;
    bool mustRevalidate = false;

    std::optional<Timestamp> toTimePoint() const;
};

std::optional<Timestamp> parseRetryHeaders(const std::optional<std::string>& retryAfter,
                                           const std::optional<std::string>& xRateLimitReset);

} // namespace http
} // namespace mbgl
