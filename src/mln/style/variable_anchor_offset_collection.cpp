#include <mln/style/variable_anchor_offset_collection.hpp>
#include <mln/style/conversion/stringify.hpp>
#include <mln/style/expression/value.hpp>
#include <mln/util/enum.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace mln {

using namespace std;
using namespace style;
using namespace style::expression;

std::array<float, 2> VariableAnchorOffsetCollection::getOffsetByAnchor(const SymbolAnchorType& anchorType) const {
    for (const auto& entry : anchorOffsets) {
        if (anchorType == entry.anchorType) {
            return entry.offset;
        }
    }

    return {0, 0};
}

std::string VariableAnchorOffsetCollection::toString() const {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartArray();
    for (const auto& entry : anchorOffsets) {
        writer.String(Enum<SymbolAnchorType>::toString(entry.anchorType));
        writer.StartArray();
        writer.Double(entry.offset[0]);
        writer.Double(entry.offset[1]);
        writer.EndArray();
    }

    writer.EndArray();
    string stringValue = buffer.GetString();
    return stringValue;
}

mln::Value VariableAnchorOffsetCollection::serialize() const {
    std::vector<mln::Value> serialized;
    for (const auto& entry : anchorOffsets) {
        serialized.emplace_back(Enum<SymbolAnchorType>::toString(entry.anchorType));
        std::vector<mln::Value> offset{entry.offset[0], entry.offset[1]};
        serialized.emplace_back(offset);
    }

    return serialized;
}

} // namespace mln
