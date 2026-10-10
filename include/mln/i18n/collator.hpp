#pragma once

#include <memory>
#include <string>
#include <optional>

namespace mln {
namespace platform {

// Implementations report failures in the constructor and compare(), such as
// text they cannot convert, by throwing an exception derived from
// std::exception. Style expressions evaluate such a failure as an expression
// error. std::bad_alloc is not treated as an expression error.
class Collator {
public:
    explicit Collator(bool caseSensitive,
                      bool diacriticSensitive,
                      const std::optional<std::string>& locale = std::nullopt);
    int compare(const std::string& lhs, const std::string& rhs) const;
    std::string resolvedLocale() const;
    bool operator==(const Collator& other) const;

private:
    class Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace platform
} // namespace mln
