#include <mbgl/renderer/group_by_layout.hpp>
#include <mbgl/style/conversion/stringify.hpp>
#include <mbgl/util/rapidjson.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace mbgl {

std::string createLayoutKey(const style::Layer::Impl& impl) {
    using namespace style::conversion;

    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartArray();
    writer.Uint64(reinterpret_cast<uint64_t>(impl.getTypeInfo()));
    writer.String(impl.source);
    writer.String(impl.sourceLayer);
    writer.Double(impl.minZoom);
    writer.Double(impl.maxZoom);
    writer.Uint(static_cast<uint32_t>(impl.visibility));
    stringify(writer, impl.filter);
    impl.stringifyLayout(writer);
    writer.EndArray();

    return s.GetString();
}

GroupMap groupLayers(const std::vector<Immutable<style::LayerProperties>>& layers) {
    MLN_TRACE_FUNC();

    mbgl::unordered_map<std::string, std::vector<Immutable<style::LayerProperties>>> groupMap;
    groupMap.reserve(layers.size());

    for (auto layer : layers) {
        groupMap[createLayoutKey(*layer->baseImpl)].push_back(std::move(layer));
    }
    return groupMap;
}

} // namespace mbgl
