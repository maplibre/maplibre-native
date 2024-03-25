#pragma once

#include <mbgl/util/string.hpp>

#include <cstdint>

namespace mbgl {
namespace util {

/**
    A simple unique identifier
 */
class SimpleIdentity final {
private:
    constexpr SimpleIdentity(std::int64_t id) noexcept
        : uniqueID(id) {}

public:
    SimpleIdentity() noexcept;
    SimpleIdentity(const SimpleIdentity&) = default;
    ~SimpleIdentity() = default;

    static const SimpleIdentity Empty;

    bool operator<(const SimpleIdentity& other) const noexcept { return uniqueID < other.uniqueID; }
    bool operator==(const SimpleIdentity& other) const noexcept { return uniqueID == other.uniqueID; }
    bool operator!=(const SimpleIdentity& other) const noexcept { return uniqueID != other.uniqueID; }

    std::int64_t id() const noexcept { return uniqueID; }
    bool isEmpty() const noexcept { return uniqueID != emptyID; }

    operator bool() const noexcept { return isEmpty(); }
    bool operator!() const noexcept { return !isEmpty(); }

private:
    static constexpr std::int64_t emptyID = 0;
    std::int64_t uniqueID;
};

/**
    Base class for objects that inherit an automatically-assigned unique identity
 */
class SimpleIdentifiable {
protected:
    SimpleIdentifiable() = default;
    SimpleIdentifiable(const SimpleIdentifiable&) = default;
    virtual ~SimpleIdentifiable() = default;

    const util::SimpleIdentity& getId() const noexcept { return uniqueID; }

protected:
    util::SimpleIdentity uniqueID;
};

inline std::string toString(const util::SimpleIdentity& ident) {
    return "{" + std::to_string(ident.id()) + "}";
}

} // namespace util
} // namespace mbgl
