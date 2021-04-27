
#include <mbgl/util/tile_server_options.hpp>
#include <mbgl/util/optional.hpp>

namespace mbgl {

    class TileServerOptions::Impl {
    public:
        std::string baseURL;
        std::string uriSchemeAlias;
        std::string sourceTemplate;
        std::string styleTemplate;
        optional<std::string> styleDomainConstraint = {};
        std::string spritesTemplate;
        optional<std::string> spritesDomainConstraint = {};
        std::string glyphsTemplate;
        optional<std::string> glyphsDomainConstraint = {};
        std::string tileTemplate;
        optional<std::string> tileDomainConstraint = {};
        std::string accessTokenParameterName;
    };

    // These requires the complete type of Impl.
    TileServerOptions::TileServerOptions() : impl_(std::make_unique<Impl>()) {}
    TileServerOptions::~TileServerOptions() = default;

    // movable
    TileServerOptions::TileServerOptions(TileServerOptions&& ) noexcept = default;
    TileServerOptions& TileServerOptions::operator=(TileServerOptions &&) noexcept = default;

    // copyable
    TileServerOptions::TileServerOptions(const TileServerOptions& options)
        : impl_(std::make_unique<Impl>(*options.impl_)) {}
    TileServerOptions& TileServerOptions::operator=(const TileServerOptions& options) {
        if (this != &options) {
            *impl_ = *options.impl_;
        }
        return *this;
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

    TileServerOptions& TileServerOptions::withSourceTemplate(std::string sourceTemplate) {
        impl_->sourceTemplate = std::move(sourceTemplate);
        return *this;
    }

    const std::string& TileServerOptions::sourceTemplate() const {
        return impl_->sourceTemplate;
    }

    TileServerOptions& TileServerOptions::withStyleTemplate(std::string styleTemplate, optional<std::string> domainConstraint) {
        impl_->styleTemplate = std::move(styleTemplate);
        impl_->styleDomainConstraint = std::move(domainConstraint);
        return *this;
    }

    const std::string& TileServerOptions::styleTemplate() const {
        return impl_->styleTemplate;
    }

    const optional<std::string>& TileServerOptions::styleDomainConstraint() const {
        return impl_->styleDomainConstraint;
    }

    TileServerOptions& TileServerOptions::withSpritesTemplate(std::string spritesTemplate, optional<std::string> domainConstraint) {
        impl_->spritesTemplate = std::move(spritesTemplate);
        impl_->spritesDomainConstraint = std::move(domainConstraint);
        return *this;
    }

    const std::string& TileServerOptions::spritesTemplate() const {
        return impl_->spritesTemplate;
    }

    const optional<std::string>& TileServerOptions::spritesDomainConstraint() const {
        return impl_->spritesDomainConstraint;
    }

    TileServerOptions& TileServerOptions::withGlyphsTemplate(std::string glyphsTemplate, optional<std::string> domainConstraint) {
        impl_->glyphsTemplate = std::move(glyphsTemplate);
        impl_->glyphsDomainConstraint = std::move(domainConstraint);
        return *this;
    }

    const std::string& TileServerOptions::glyphsTemplate() const {
        return impl_->glyphsTemplate;
    }

    const optional<std::string>& TileServerOptions::glyphsDomainConstraint() const {
        return impl_->glyphsDomainConstraint;
    }

    TileServerOptions& TileServerOptions::withTileTemplate(std::string tileTemplate, optional<std::string> domainConstraint) {
        impl_->tileTemplate = std::move(tileTemplate);
        impl_->tileDomainConstraint = std::move(domainConstraint);
        return *this;
    }

    const std::string& TileServerOptions::tileTemplate() const {
        return impl_->tileTemplate;
    }

    const optional<std::string>& TileServerOptions::tileDomainConstraint() const {
        return impl_->tileDomainConstraint;
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
            .withSourceTemplate("/v4/{domain}.json")
            .withStyleTemplate("/styles/v1{path}", {"styles"})
            .withSpritesTemplate("/styles/v1{directory}{filename}/sprite{extension}", {"sprites"})
            .withGlyphsTemplate("/fonts/v1{path}", {"fonts"})
        .withTileTemplate("/v4{path}", {"tiles"});
        return options;
    }

    TileServerOptions TileServerOptions::MapTilerConfiguration() {
        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.maptiler.com")
            .withUriSchemeAlias("maptiler")
            .withAccessTokenParameterName("key")
            .withSourceTemplate("/tiles/{path}/tiles.json")
            .withStyleTemplate("/maps/{path}", {"maps"})
            .withSpritesTemplate("/maps/{path}/sprite{scale}.{format}", {})
            .withGlyphsTemplate("/fonts/{fontstack}/{start}-{end}.pbf", {"fonts"})
            .withTileTemplate("/tiles/{path}", {"tiles"});
        return options;
    }

}  // namespace mbgl
