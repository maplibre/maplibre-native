#pragma once

#include <mln/style/source.hpp>
#include <mln/style/conversion.hpp>

#include <memory>
#include <optional>

namespace mln {
namespace style {
namespace conversion {

template <>
struct Converter<std::unique_ptr<Source>> {
public:
    std::optional<std::unique_ptr<Source>> operator()(const Convertible& value,
                                                      Error& error,
                                                      const std::string& id) const;
};

} // namespace conversion
} // namespace style
} // namespace mln
