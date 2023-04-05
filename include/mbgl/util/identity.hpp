#pragma once

#include <cstdint>

namespace mbgl {
namespace util {

/**
    A simple unique identifier
 */
class SimpleIdentity final {
public:
    SimpleIdentity();
    SimpleIdentity(const SimpleIdentity&) = default;
    ~SimpleIdentity() = default;

    bool operator<(const SimpleIdentity& other) const { return uniqueID < other.uniqueID; }
    bool operator==(const SimpleIdentity& other) const { return uniqueID == other.uniqueID; }
    bool operator!=(const SimpleIdentity& other) const { return uniqueID != other.uniqueID; }

private:
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
