#include <mbgl/util/rapidjson.hpp>
#include <mbgl/util/string.hpp>

namespace mln {

std::string formatJSONParseError(const JSDocument& doc) {
    return std::string{rapidjson::GetParseError_En(doc.GetParseError())} + " at offset " +
           util::toString(doc.GetErrorOffset());
}

} // namespace mln
