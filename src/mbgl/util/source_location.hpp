#pragma once

#include <cstdint>

#if defined(__has_include)
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace mbgl {

#if defined(__cpp_lib_source_location) && __cpp_lib_source_location >= 201907L
using source_location = std::source_location;
#define MLN_CURRENT_SOURCE_LOCATION ::mbgl::source_location::current()
#else
class source_location final {
public:
    constexpr source_location(const char* fileName_ = "",
                              const char* functionName_ = "",
                              std::uint_least32_t line_ = 0) noexcept
        : fileName(fileName_),
          functionName(functionName_),
          lineNumber(line_) {}

    constexpr std::uint_least32_t line() const noexcept { return lineNumber; }
    constexpr std::uint_least32_t column() const noexcept { return 0; }
    constexpr const char* file_name() const noexcept { return fileName; }
    constexpr const char* function_name() const noexcept { return functionName; }

private:
    const char* fileName;
    const char* functionName;
    std::uint_least32_t lineNumber;
};
#define MLN_CURRENT_SOURCE_LOCATION  \
    ::mbgl::source_location {        \
        __FILE__, __func__, __LINE__ \
    }
#endif

} // namespace mbgl
