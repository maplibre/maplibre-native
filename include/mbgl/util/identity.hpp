#pragma once

#include <cstdint>

namespace mbgl {
namespace util {

/**
    A simple unique identifier
 */
class SimpleIdentity final {
private:
    constexpr SimpleIdentity(std::int64_t id) : uniqueID(id) { }
public:
    SimpleIdentity();
    SimpleIdentity(const SimpleIdentity&) = default;
    ~SimpleIdentity() = default;

    static const SimpleIdentity Empty;

    bool operator<(const SimpleIdentity& other) const { return uniqueID < other.uniqueID; }
    bool operator==(const SimpleIdentity& other) const { return uniqueID == other.uniqueID; }
    bool operator!=(const SimpleIdentity& other) const { return uniqueID != other.uniqueID; }

    operator bool() const { return uniqueID != emptyID; }
    bool operator!() const { return uniqueID == emptyID; }

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

    const util::SimpleIdentity& getId() const { return uniqueID; }
    
protected:
    util::SimpleIdentity uniqueID;
};


} // namespace util
} // namespace mbgl
