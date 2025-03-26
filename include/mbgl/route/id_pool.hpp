#pragma once

#include <cstdint> // uint32_t
#include <cstdlib>
#include <cstring>
#include <mbgl/util/logging.hpp>
namespace mbgl {
namespace gfx {

/***
 ** An IDpool is a pool of IDs from which you can create consecutive IDs. This is very similary to graphics APIs such as
 * opengl or vulkan where you an ID is used to represent a resource. Every ID constructed is consecutive and when an
 * ID is disposed, it returns back to the pool for re-use. Every invocation to create an ID will return the smallest
 * contiguous ID.
 */
class IDpool {
private:
    // Change to uint16_t here for a more compact implementation if 16bit or less IDs work for you.
    typedef uint32_t uint;

    struct Range {
        uint first_;
        uint last_;
    };

    Range* ranges_; // Sorted array of ranges of free IDs
    uint count_;    // Number of ranges in list
    uint capacity_; // Total capacity of range list

    IDpool& operator=(const IDpool&);
    IDpool(const IDpool&);

public:
    explicit IDpool(const uint max_id) {
        // Start with a single range, from 0 to max allowed ID (specified)
        ranges_ = static_cast<Range*>(::malloc(sizeof(Range)));
        capacity_ = 1;
        reset(max_id);
    }

    ~IDpool() { ::free(ranges_); }

    void reset(const uint max_id) {
        ranges_[0].first_ = 0;
        ranges_[0].last_ = max_id;
        count_ = 1;
    }

    bool createID(uint& id) {
        if (ranges_[0].first_ <= ranges_[0].last_) {
            id = ranges_[0].first_;

            // If current range is full and there is another one, that will become the new current range
            if (ranges_[0].first_ == ranges_[0].last_ && count_ > 1) {
                destroyRange(0);
            } else {
                ++ranges_[0].first_;
            }
            return true;
        }

        // No availble ID left
        return false;
    }

    bool createRangeID(uint& id, const uint count) {
        uint i = 0;
        do {
            const uint range_count = 1 + ranges_[i].last_ - ranges_[i].first_;
            if (count <= range_count) {
                id = ranges_[i].first_;

                // If current range is full and there is another one, that will become the new current range
                if (count == range_count && i + 1 < count_) {
                    destroyRange(i);
                } else {
                    ranges_[i].first_ += count;
                }
                return true;
            }
            ++i;
        } while (i < count_);

        // No range of free IDs was large enough to create the requested continuous ID sequence
        return false;
    }

    bool destroyID(const uint id) { return destroyRangeID(id, 1); }

    bool destroyRangeID(const uint id, const uint count) {
        const uint end_id = id + count;

        // Binary search of the range list
        uint i0 = 0;
        uint i1 = count_ - 1;

        for (;;) {
            const uint i = (i0 + i1) / 2;

            if (id < ranges_[i].first_) {
                // Before current range, check if neighboring
                if (end_id >= ranges_[i].first_) {
                    if (end_id != ranges_[i].first_)
                        return false; // Overlaps a range of free IDs, thus (at least partially) invalid IDs

                    // Neighbor id, check if neighboring previous range too
                    if (i > i0 && id - 1 == ranges_[i - 1].last_) {
                        // Merge with previous range
                        ranges_[i - 1].last_ = ranges_[i].last_;
                        destroyRange(i);
                    } else {
                        // Just grow range
                        ranges_[i].first_ = id;
                    }
                    return true;
                } else {
                    // Non-neighbor id
                    if (i != i0) {
                        // Cull upper half of list
                        i1 = i - 1;
                    } else {
                        // Found our position in the list, insert the deleted range here
                        insertRange(i);
                        ranges_[i].first_ = id;
                        ranges_[i].last_ = end_id - 1;
                        return true;
                    }
                }
            } else if (id > ranges_[i].last_) {
                // After current range, check if neighboring
                if (id - 1 == ranges_[i].last_) {
                    // Neighbor id, check if neighboring next range too
                    if (i < i1 && end_id == ranges_[i + 1].first_) {
                        // Merge with next range
                        ranges_[i].last_ = ranges_[i + 1].last_;
                        destroyRange(i + 1);
                    } else {
                        // Just grow range
                        ranges_[i].last_ += count;
                    }
                    return true;
                } else {
                    // Non-neighbor id
                    if (i != i1) {
                        // Cull bottom half of list
                        i0 = i + 1;
                    } else {
                        // Found our position in the list, insert the deleted range here
                        insertRange(i + 1);
                        ranges_[i + 1].first_ = id;
                        ranges_[i + 1].last_ = end_id - 1;
                        return true;
                    }
                }
            } else {
                // Inside a free block, not a valid ID
                return false;
            }
        }
    }

    bool isID(const uint id) const {
        // Binary search of the range list
        uint i0 = 0;
        uint i1 = count_ - 1;

        for (;;) {
            const uint i = (i0 + i1) / 2;

            if (id < ranges_[i].first_) {
                if (i == i0) return true;

                // Cull upper half of list
                i1 = i - 1;
            } else if (id > ranges_[i].last_) {
                if (i == i1) return true;

                // Cull bottom half of list
                i0 = i + 1;
            } else {
                // Inside a free block, not a valid ID
                return false;
            }
        }
    }

    uint getAvailableIDs() const {
        uint count = count_;
        uint i = 0;

        do {
            count += ranges_[i].last_ - ranges_[i].first_;
            ++i;
        } while (i < count_);

        return count;
    }

    uint getLargestContinuousRange() const {
        uint max_count = 0;
        uint i = 0;

        do {
            uint count = ranges_[i].last_ - ranges_[i].first_ + 1;
            if (count > max_count) max_count = count;

            ++i;
        } while (i < count_);

        return max_count;
    }

    void printRanges() const {
        uint i = 0;
        for (;;) {
            std::stringstream log;
            if (ranges_[i].first_ < ranges_[i].last_)
                log << ranges_[i].first_ << "-" << ranges_[i].last_;
            else if (ranges_[i].first_ == ranges_[i].last_)
                log << ranges_[i].first_;
            else
                log << "-";

            ++i;
            if (i >= count_) {
                log << "\n";
                return;
            }

            log << ", ";
            mbgl::Log::Info(mbgl::Event::General, log.str());
        }
    }

private:
    void insertRange(const uint index) {
        if (count_ >= capacity_) {
            capacity_ += capacity_;
            ranges_ = (Range*)realloc(ranges_, capacity_ * sizeof(Range));
        }

        ::memmove(ranges_ + index + 1, ranges_ + index, (count_ - index) * sizeof(Range));
        ++count_;
    }

    void destroyRange(const uint index) {
        --count_;
        ::memmove(ranges_ + index, ranges_ + index + 1, (count_ - index) * sizeof(Range));
    }
};

} // namespace gfx
} // namespace mbgl
