#pragma once

#include <mln/style/layer.hpp>
#include <mln/style/sprite.hpp>
#include <mln/style/source.hpp>
#include <mln/style/light.hpp>

#include <mln/text/glyph.hpp>

#include <mln/util/constants.hpp>
#include <mln/util/rapidjson.hpp>
#include <mln/util/font_stack.hpp>
#include <mln/util/geo.hpp>

#include <vector>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <forward_list>

namespace mln {
namespace style {

using StyleParseResult = std::exception_ptr;

class Parser {
public:
    ~Parser();

    StyleParseResult parse(const std::string&);

    std::vector<Sprite> sprites;
    std::string glyphURL;
    std::shared_ptr<FontFaces> fontFaces;

    std::vector<std::unique_ptr<Source>> sources;
    std::vector<std::unique_ptr<Layer>> layers;

    TransitionOptions transition{{util::DEFAULT_TRANSITION_DURATION}};
    Light light;

    std::string name;
    LatLng latLng;
    double zoom = 0;
    double bearing = 0;
    double pitch = 0;
    double roll = 0;
    double centerAltitude = 0;

    // Statically evaluate layer properties to determine what font stacks are used.
    std::set<FontStack> fontStacks() const;

private:
    void parseTransition(const JSValue&);
    void parseLight(const JSValue&);
    void parseSources(const JSValue&);
    void parseSprites(const JSValue&);
    void parseLayers(const JSValue&);
    void parseLayer(const std::string& id, const JSValue&, std::unique_ptr<Layer>&);

    std::unordered_map<std::string, std::pair<const JSValue&, std::unique_ptr<Layer>>> layersMap;

    // Store a stack of layer IDs we're parsing right now. This is to prevent reference cycles.
    std::forward_list<std::string> stack;
};

} // namespace style
} // namespace mln
