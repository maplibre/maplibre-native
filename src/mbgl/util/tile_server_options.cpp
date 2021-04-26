
#include <mbgl/util/tile_server_options.hpp>


namespace mbgl {

    class TileServerOptions::Impl {
    public:
        std::string baseURL;
        std::string uriSchemeAlias;
        std::string sourceTemplate;
        std::string styleTemplate;
        std::string spritesTemplate;
        std::string glyphsTemplate;
        std::string tileTemplate;
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

    TileServerOptions TileServerOptions::MapboxConfiguration() {
        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.mapbox.com")
            .withUriSchemeAlias("mapbox")
            .withAccessTokenParameterName("access_token")
            .withSourceTemplate("/v4/{domain}.json")
            .withStyleTemplate("/styles/v1{path}")
            .withSpritesTemplate("/styles/v1{directory}{filename}/sprite{extension}")
            .withGlyphsTemplate("/fonts/v1{path}")
            .withTileTemplate("/v4{path}");
        return options;
    }

    TileServerOptions TileServerOptions::MapTilerConfiguration() {
        TileServerOptions options = TileServerOptions()
            .withBaseURL("https://api.maptiler.com")
            .withUriSchemeAlias("maptiler")
            .withAccessTokenParameterName("key")
            .withSourceTemplate("/tiles/{path}/tiles.json")
            .withStyleTemplate("/maps/{path}")
            .withSpritesTemplate("/maps/{path}/sprite{scale}.{format}")
            .withGlyphsTemplate("/fonts/{fontstack}/{start}-{end}.pbf")
            .withTileTemplate("/tiles/{path}");
        return options;
    }

}  // namespace mbgl
