#pragma once

#include <mbgl/i18n/collator.hpp>

#include <string>
#include <optional>

namespace mbgl {
namespace style {
namespace expression {

class Collator {
public:
    Collator(bool caseSensitive, bool diacriticSensitive, const std::optional<std::string>& locale = std::nullopt);

    bool operator==(const Collator& other) const;

    int compare(const std::string& lhs, const std::string& rhs) const;

    std::string resolvedLocale() const;

private:
    platform::Collator collator;
};

} // namespace expression
} // namespace style
} // namespace mbgl
