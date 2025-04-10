#pragma once

#include <cstdint>
#include <functional>

#define INVALID_ID (uint32_t(~0))

#define MAKEID_TYPE(name)                                          \
    typedef struct name {                                          \
        uint32_t id;                                               \
        name() {                                                   \
            id = INVALID_ID;                                       \
        }                                                          \
        name(uint32_t nid) {                                       \
            id = nid;                                              \
        }                                                          \
        bool isValid() const {                                     \
            return id != INVALID_ID;                               \
        }                                                          \
        friend bool operator<(const name& lhs, const name& rhs) {  \
            return lhs.id < rhs.id;                                \
        }                                                          \
        friend bool operator==(const name& lhs, const name& rhs) { \
            return lhs.id == rhs.id;                               \
        }                                                          \
        friend bool operator!=(const name& lhs, const name& rhs) { \
            return lhs.id != rhs.id;                               \
        }                                                          \
    } name;

template <class EntityID>
struct IDHasher {
    size_t operator()(const EntityID& eid) const { return eid.id; }
};

MAKEID_TYPE(RouteID);
MAKEID_TYPE(RouteSegmentID);
