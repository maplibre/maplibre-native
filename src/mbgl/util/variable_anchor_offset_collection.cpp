#include <mbgl/util/variable_anchor_offset_collection.hpp>
#include <mbgl/style/conversion/stringify.hpp>
#include <mbgl/style/expression/value.hpp>
#include <mbgl/util/enum.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace mbgl {

using namespace std;
using namespace style;
using namespace style::expression;

bool VariableAnchorOffsetCollection::empty() const {
    return anchorOffsets.size() == 0;
};

std::vector<AnchorOffsetPair> VariableAnchorOffsetCollection::getOffsets() const {
    return anchorOffsets;
}

std::array<float, 2> VariableAnchorOffsetCollection::getOffsetByAnchor(const SymbolAnchorType& anchorType) const {
    for (const auto& pair : anchorOffsets) {
        if (anchorType == pair.first) {
            return pair.second;
        }
    }

    return {0, 0};
}

// Avoid quoting when convert to string in expression
std::string VariableAnchorOffsetCollection::toString() const {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    writer.StartArray();
    for (const auto& pair : anchorOffsets) {
        writer.String(Enum<SymbolAnchorType>::toString(pair.first));
        writer.StartArray();
        writer.Double(pair.second[0]);
        writer.Double(pair.second[1]);
        writer.EndArray();
    }

    writer.EndArray();
    string stringValue = buffer.GetString();
    return stringValue;
}

mbgl::Value VariableAnchorOffsetCollection::serialize() const {
    std::vector<mbgl::Value> serialized;
    for (const auto& pair : anchorOffsets) {
        serialized.emplace_back(Enum<SymbolAnchorType>::toString(pair.first));
        std::vector<mbgl::Value> offset{pair.second[0], pair.second[1]};
        serialized.emplace_back(offset);
    }

    return serialized;
}

} // namespace mbgl
