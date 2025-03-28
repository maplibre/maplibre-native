#pragma once

#include <cassert>
#include <vector>

namespace mbgl {

class MapLodShift {
public:
    MapLodShift() = default;

    MapLodShift(const std::vector<double>& data) {
        assert(data.size() % 2 == 0);
        std::size_t size = data.size() / 2;
        zoom.resize(size);
        shift.resize(size);
        for (std::size_t i = 0; i < size; i++) {
            zoom[i] = data[i * 2];
            shift[i] = data[i * 2 + 1];
        }
    }

    double get(double z) const {
        if (zoom.empty()) {
            return 0;
        }
        if (z <= zoom.front()) {
            return shift.front();
        }
        if (z >= zoom.back()) {
            return shift.back();
        }
        for (std::size_t i = 1; i < zoom.size(); i++) {
            if (z < zoom[i]) {
                double t = (z - zoom[i - 1]) / (zoom[i] - zoom[i - 1]);
                return shift[i - 1] + t * (shift[i] - shift[i - 1]);
            }
        }
        return 0;
    }

private:
    std::vector<double> zoom;
    std::vector<double> shift;
};

} // namespace mbgl
