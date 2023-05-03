#pragma once

#include <mbgl/storage/response.hpp>
#include <mbgl/util/chrono.hpp>

#include <optional>

namespace mbgl {
namespace http {

Duration errorRetryTimeout(Response::Error::Reason failedRequestReason,
                           uint32_t failedRequests,
                           std::optional<Timestamp> retryAfter = std::nullopt);

Duration expirationTimeout(std::optional<Timestamp> expires, uint32_t expiredRequests);

} // namespace http
} // namespace mbgl
