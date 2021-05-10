
#include <mbgl/util/tile_server_options.hpp>
#include <mbgl/util/optional.hpp>

namespace mbgl {

    class TileServerOptions::Impl {
    public:
        std::string baseURL;
        optional<std::string> versionPrefix;
        std::string uriSchemeAlias;
        std::string sourceTemplate;
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
        std::string accessTokenParameterName;
    };


    TileServerOptions::TileServerOptions() : impl_(std::make_unique<Impl>()) {}
    TileServerOptions::~TileServerOptions() = default;

    // movable
    TileServerOptions::TileServerOptions(TileServerOptions &&) noexcept = default;
    TileServerOptions& TileServerOptions::operator=(TileServerOptions &&) noexcept = default;

    // copyable
    TileServerOptions::TileServerOptions(const TileServerOptions& options)
        : impl_(std::make_unique<Impl>(*options.impl_)) {}
    TileServerOptions& TileServerOptions::operator=(TileServerOptions& options) {
        swap(impl_, options.impl_);
        return *this;
    }

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

    TileServerOptions& TileServerOptions::withSourceTemplate(std::string sourceTemplate, optional<std::string> versionPrefix) {
        impl_->sourceTemplate = std::move(sourceTemplate);
        impl_->sourceVersionPrefix = std::move(versionPrefix);
        return *this;
    }

    const std::string& TileServerOptions::sourceTemplate() const {
        return impl_->sourceTemplate;
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

    TileServerOptions& TileServerOptions::withAccessTokenParameterName(std::string accessTokenParameterName) {
        impl_->accessTokenParameterName = std::move(accessTokenParameterName);
        return *this;
    }

    const std::string& TileServerOptions::accessTokenParameterName() const {
        return impl_->accessTokenParameterName;
    }

    TileServerOptions TileServerOptions::MapboxConfiguration() {
        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.mapbox.com")
            .withUriSchemeAlias("mapbox")
            .withAccessTokenParameterName("access_token")
            .withSourceTemplate("/{domain}.json", {"/v4"})
            .withStyleTemplate("/styles/v1{path}", "styles", {})
            .withSpritesTemplate("/styles/v1{directory}{filename}/sprite{extension}", "sprites", {})
            .withGlyphsTemplate("/fonts/v1{path}", "fonts", {})
            .withTileTemplate("{path}", "tiles", {"/v4"});
        return options;
    }

    TileServerOptions TileServerOptions::MapTilerConfiguration() {
        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.maptiler.com")
            .withUriSchemeAlias("maptiler")
            .withAccessTokenParameterName("key")
            .withSourceTemplate("/tiles/{path}/tiles.json", {})
            .withStyleTemplate("/maps/{path}", "maps", {})
            .withSpritesTemplate("/maps/{path}/sprite{scale}.{format}", "", {})
            .withGlyphsTemplate("/fonts/{fontstack}/{start}-{end}.pbf", "fonts", {})
            .withTileTemplate("/tiles/{path}", "tiles", {});
        return options;
    }

}  // namespace mbgl
