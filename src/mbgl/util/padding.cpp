#include <mbgl/util/padding.hpp>
#include <mbgl/util/string.hpp>

namespace mbgl {

    std::optional<Padding> Padding::parse(const std::string& /*s*/) {
        //BUGBUG wip
        assert(false);
        return {};
    }

    std::string Padding::toString() const {
        //BUGBUG match format with Web
        //BUGBUG check if there is already code to dump arrays
        return "[" +
            util::toString(top, true) + "," +
            util::toString(right, true) + "," +
            util::toString(bottom, true) + "," +
            util::toString(left, true) + "]";
    }

//    std::array<double, 4> Padding::toArray() const {
//        return {{values[0], values[1], values[2], values[3]}};
//    }

//    mbgl::Value Color::toObject() const {
//        return mapbox::base::ValueObject{{"r", static_cast<double>(r)},
//                                         {"g", static_cast<double>(g)},
//                                         {"b", static_cast<double>(b)},
//                                         {"a", static_cast<double>(a)}};
//    }

    mbgl::Value Padding::serialize() const {
        return std::vector<mbgl::Value>{ top, right, bottom, left };
    }

} // namespace mbgl
