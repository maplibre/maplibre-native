
#include <mbgl/util/tile_server_options.hpp>


namespace mbgl {

    class TileServerOptions::Impl {
    public:
        std::string baseURL;
        std::string uriSchemeAlias;
        std::string sourceTemplate = "/tiles/v3/{domain}.json";
        std::string styleTemplate = "/maps/{path}/style.json";
        std::string spritesTemplate = "/maps/{path}/sprite.json";
        std::string glyphsTemplate = "/fonts/{fontstack}/{range}.pbf";
        std::string tileTemplate = "/tiles/v3/{path}";
        std::string accessTokenParameterName = "key";
    };

// These requires the complete type of Impl.
    TileServerOptions::TileServerOptions() : impl_(std::make_unique<Impl>()) {}
    TileServerOptions::~TileServerOptions() = default;
    TileServerOptions::TileServerOptions(TileServerOptions&&) noexcept = default;
    TileServerOptions::TileServerOptions(const TileServerOptions& other) : impl_(std::make_unique<Impl>(*other.impl_)) {}
    TileServerOptions& TileServerOptions::operator=(TileServerOptions options) {swap(impl_, options.impl_); return *this; }

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

    TileServerOptions& TileServerOptions::withSourceTemplate(std::string sourceTemplate) {
        impl_->sourceTemplate = std::move(sourceTemplate);
        return *this;
    }

    const std::string& TileServerOptions::sourceTemplate() const {
        return impl_->sourceTemplate;
    }

    TileServerOptions& TileServerOptions::withStyleTemplate(std::string styleTemplate) {
        impl_->styleTemplate = std::move(styleTemplate);
        return *this;
    }

    const std::string& TileServerOptions::styleTemplate() const {
        return impl_->styleTemplate;
    }

    TileServerOptions& TileServerOptions::withSpritesTemplate(std::string spritesTemplate) {
        impl_->spritesTemplate = std::move(spritesTemplate);
        return *this;
    }

    const std::string& TileServerOptions::spritesTemplate() const {
        return impl_->spritesTemplate;
    }

    TileServerOptions& TileServerOptions::withGlyphsTemplate(std::string glyphsTemplate) {
        impl_->glyphsTemplate = std::move(glyphsTemplate);
        return *this;
    }

    const std::string& TileServerOptions::glyphsTemplate() const {
        return impl_->glyphsTemplate;
    }

    TileServerOptions& TileServerOptions::withTileTemplate(std::string tileTemplate) {
        impl_->tileTemplate = std::move(tileTemplate);
        return *this;
    }

    const std::string& TileServerOptions::tileTemplate() const {
        return impl_->tileTemplate;
    }

    TileServerOptions& TileServerOptions::withAccessTokenParameterName(std::string accessTokenParameterName) {
        impl_->accessTokenParameterName = std::move(accessTokenParameterName);
        return *this;
    }

    const std::string& TileServerOptions::accessTokenParameterName() const {
        return impl_->accessTokenParameterName;
    }    

}  // namespace mbgl
