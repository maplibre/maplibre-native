
#include <mbgl/util/tile_server_options.hpp>
#include <mbgl/util/optional.hpp>

namespace mbgl {

    class TileServerOptions::Impl {
    public:
        std::string baseURL;
        optional<std::string> versionPrefix;
        std::string uriSchemeAlias;
        
        std::string sourceTemplate;
        std::string sourceDomainName;
        optional<std::string> sourceVersionPrefix;
        
        std::string styleTemplate;
        std::string styleDomainName;
        optional<std::string> styleVersionPrefix;
        
        std::string spritesTemplate;
        std::string spritesDomainName;
        optional<std::string> spritesVersionPrefix;
        
        std::string glyphsTemplate;
        std::string glyphsDomainName;
        optional<std::string> glyphsVersionPrefix;
        
        std::string tileTemplate;
        std::string tileDomainName;
        optional<std::string> tileVersionPrefix;
        
        std::string apiKeyParameterName;
        bool apiKeyRequired;
        
        std::vector<mbgl::util::DefaultStyle> defaultStyles;
        std::string defaultStyle;
    };


    TileServerOptions::TileServerOptions() : impl_(std::make_unique<Impl>()) {}
    TileServerOptions::~TileServerOptions() = default;

    // movable
    TileServerOptions::TileServerOptions(TileServerOptions &&) noexcept = default;
    TileServerOptions& TileServerOptions::operator=(TileServerOptions &&) noexcept = default;

    // copyable
    TileServerOptions::TileServerOptions(const TileServerOptions& options)
        : impl_(std::make_unique<Impl>(*options.impl_)) {}

    TileServerOptions TileServerOptions::clone() const {
        return TileServerOptions(*this);
    }

    TileServerOptions& TileServerOptions::withBaseURL(std::string url) {
        impl_->baseURL = std::move(url);
        return *this;
    }

    const std::string& TileServerOptions::baseURL() const {
        return impl_->baseURL;
    }

    TileServerOptions& TileServerOptions::withUriSchemeAlias(std::string alias) {
        impl_->uriSchemeAlias = std::move(alias);
        return *this;
    }

    const std::string& TileServerOptions::uriSchemeAlias() const {
        return impl_->uriSchemeAlias;
    }

    TileServerOptions& TileServerOptions::withSourceTemplate(std::string sourceTemplate, std::string domainName, optional<std::string> versionPrefix) {
        impl_->sourceTemplate = std::move(sourceTemplate);
        impl_->sourceVersionPrefix = std::move(versionPrefix);
        impl_->sourceDomainName = std::move(domainName);
        return *this;
    }

    const std::string& TileServerOptions::sourceTemplate() const {
        return impl_->sourceTemplate;
    }

    const std::string& TileServerOptions::sourceDomainName() const {
        return impl_->sourceDomainName;
    }

    const optional<std::string>& TileServerOptions::sourceVersionPrefix() const {
        return impl_->sourceVersionPrefix;
    }

    TileServerOptions& TileServerOptions::withStyleTemplate(std::string styleTemplate, std::string domainName, optional<std::string> versionPrefix) {
        impl_->styleTemplate = std::move(styleTemplate);
        impl_->styleDomainName = std::move(domainName);
        impl_->styleVersionPrefix = std::move(versionPrefix);
        return *this;
    }

    const std::string& TileServerOptions::styleTemplate() const {
        return impl_->styleTemplate;
    }

    const std::string& TileServerOptions::styleDomainName() const {
        return impl_->styleDomainName;
    }

    const optional<std::string>& TileServerOptions::styleVersionPrefix() const {
        return impl_->styleVersionPrefix;
    }

    TileServerOptions& TileServerOptions::withSpritesTemplate(std::string spritesTemplate, std::string domainName, optional<std::string> versionPrefix) {
        impl_->spritesTemplate = std::move(spritesTemplate);
        impl_->spritesDomainName = std::move(domainName);
        impl_->spritesVersionPrefix = std::move(versionPrefix);
        return *this;
    }

    const std::string& TileServerOptions::spritesTemplate() const {
        return impl_->spritesTemplate;
    }

    const std::string& TileServerOptions::spritesDomainName() const {
        return impl_->spritesDomainName;
    }

    const optional<std::string>& TileServerOptions::spritesVersionPrefix() const {
        return impl_->spritesVersionPrefix;
    }

    TileServerOptions& TileServerOptions::withGlyphsTemplate(std::string glyphsTemplate, std::string domainName, optional<std::string> versionPrefix) {
        impl_->glyphsTemplate = std::move(glyphsTemplate);
        impl_->glyphsDomainName = std::move(domainName);
        impl_->glyphsVersionPrefix = std::move(versionPrefix);
        return *this;
    }

    const std::string& TileServerOptions::glyphsTemplate() const {
        return impl_->glyphsTemplate;
    }

    const std::string& TileServerOptions::glyphsDomainName() const {
        return impl_->glyphsDomainName;
    }

    const optional<std::string>& TileServerOptions::glyphsVersionPrefix() const {
        return impl_->glyphsVersionPrefix;
    }

    TileServerOptions& TileServerOptions::withTileTemplate(std::string tileTemplate, std::string domainName, optional<std::string> versionPrefix) {
        impl_->tileTemplate = std::move(tileTemplate);
        impl_->tileDomainName = std::move(domainName);
        impl_->tileVersionPrefix = std::move(versionPrefix);
        return *this;
    }

    const std::string& TileServerOptions::tileTemplate() const {
        return impl_->tileTemplate;
    }

    const std::string& TileServerOptions::tileDomainName() const {
        return impl_->tileDomainName;
    }

    const optional<std::string>& TileServerOptions::tileVersionPrefix() const {
        return impl_->tileVersionPrefix;
    }

    TileServerOptions& TileServerOptions::withApiKeyParameterName(std::string apiKeyParameterName) {
        impl_->apiKeyParameterName = std::move(apiKeyParameterName);
        return *this;
    }

    const std::string& TileServerOptions::apiKeyParameterName() const {
        return impl_->apiKeyParameterName;
    }

    TileServerOptions& TileServerOptions::setRequiresApiKey(bool apiKeyRequired) {
        impl_->apiKeyRequired = apiKeyRequired;
        return *this;
    }

    bool TileServerOptions::requiresApiKey() const {
        return impl_->apiKeyRequired;
    }

    const std::vector<mbgl::util::DefaultStyle> TileServerOptions::defaultStyles() const {
        return impl_->defaultStyles;
    }

    TileServerOptions& TileServerOptions::withDefaultStyles(std::vector<mbgl::util::DefaultStyle> styles) {
        impl_->defaultStyles = std::move(styles);
        return *this;
    }

    TileServerOptions& TileServerOptions::withDefaultStyle(std::string defaultStyle) {
        impl_->defaultStyle = std::move(defaultStyle);
        return *this;
    }

    const std::string& TileServerOptions::defaultStyle() const {
        return impl_->defaultStyle;
    }

    TileServerOptions TileServerOptions::DefaultConfiguration() {
        return MapLibreConfiguration();
    }

    TileServerOptions TileServerOptions::MapLibreConfiguration() {
        std::vector<mbgl::util::DefaultStyle> styles {
                // https://demotiles.maplibre.org/style.json
                mbgl::util::DefaultStyle("maplibre://maps/style", "Basic", 1)
        };

        TileServerOptions options = TileServerOptions()
                .withBaseURL("https://demotiles.maplibre.org")
                .withUriSchemeAlias("maplibre")
                .withApiKeyParameterName("")
                .withSourceTemplate("/tiles/{domain}.json", "", {})
                .withStyleTemplate("{path}.json", "maps", {})
                .withSpritesTemplate("/{path}/sprite{scale}.{format}", "", {})
                .withGlyphsTemplate("/font/{fontstack}/{start}-{end}.pbf", "fonts", {})
                .withTileTemplate("/{path}", "tiles", {})
                .withDefaultStyles(styles)
                .withDefaultStyle("Basic")
                .setRequiresApiKey(false);
        return options;
    }

    //

    TileServerOptions TileServerOptions::MapboxConfiguration() {
        std::vector<mbgl::util::DefaultStyle> styles {
                mbgl::util::DefaultStyle("mapbox://styles/mapbox/streets-v11", "Streets", 11),
                mbgl::util::DefaultStyle("mapbox://styles/mapbox/outdoors-v11", "Outdoors", 11),
                mbgl::util::DefaultStyle("mapbox://styles/mapbox/light-v10", "Light", 10),
                mbgl::util::DefaultStyle("mapbox://styles/mapbox/dark-v10", "Dark", 10),
                mbgl::util::DefaultStyle("mapbox://styles/mapbox/satellite-v9", "Satellite", 9),
                mbgl::util::DefaultStyle("mapbox://styles/mapbox/satellite-streets-v11",
                                       "Satellite Streets", 11)
        };

        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.mapbox.com")
            .withUriSchemeAlias("mapbox")
            .withApiKeyParameterName("access_token")
            .withSourceTemplate("/{domain}.json", "", {"/v4"})
            .withStyleTemplate("/styles/v1{path}", "styles", {})
            .withSpritesTemplate("/styles/v1{directory}{filename}/sprite{extension}", "sprites", {})
            .withGlyphsTemplate("/fonts/v1{path}", "fonts", {})
            .withTileTemplate("{path}", "tiles", {"/v4"})
            .withDefaultStyles(styles)
            .withDefaultStyle("Streets")
            .setRequiresApiKey(true);
        return options;
    }

    TileServerOptions TileServerOptions::MapTilerConfiguration() {

        std::vector<mbgl::util::DefaultStyle> styles{
                mbgl::util::DefaultStyle("maptiler://maps/streets", "Streets", 1),
                mbgl::util::DefaultStyle("maptiler://maps/outdoor", "Outdoor", 1),
                mbgl::util::DefaultStyle("maptiler://maps/basic", "Basic", 1),
                mbgl::util::DefaultStyle("maptiler://maps/bright", "Bright", 1),
                mbgl::util::DefaultStyle("maptiler://maps/pastel", "Pastel", 1),
                mbgl::util::DefaultStyle("maptiler://maps/hybrid", "Satellite Hybrid", 1),
                mbgl::util::DefaultStyle("maptiler://maps/topo", "Satellite Topo", 1)
        };

        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.maptiler.com")
            .withUriSchemeAlias("maptiler")
            .withApiKeyParameterName("key")
            .withSourceTemplate("/tiles{path}/tiles.json", "sources", {})
            .withStyleTemplate("/maps{path}/style.json", "maps", {})
            .withSpritesTemplate("/maps{path}", "sprites", {})
            .withGlyphsTemplate("/fonts{path}", "fonts", {})
            .withTileTemplate("{path}", "tiles", {})
            .withDefaultStyles(styles)
            .withDefaultStyle("Streets")
            .setRequiresApiKey(true);
        return options;
    }

}  // namespace mbgl
