#pragma once

#include <string>

namespace mbgl {

/**
 * @brief Holds values for tile server options.
 */
    class TileServerOptions final {
    public:
        /**
         * @brief Constructs a TileServerOptions object with default values.
         */
        TileServerOptions();
        TileServerOptions(const TileServerOptions&);

        ~TileServerOptions();
        TileServerOptions(TileServerOptions&&) noexcept;
        TileServerOptions& operator=(TileServerOptions options);

        TileServerOptions clone() const;

        /**
         * @brief Sets the API base URL. For example https://api.mapbox.com for Mapbox.
         *
         * @param baseURL API base URL.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withBaseURL(std::string baseURL);

        /**
         * @brief Gets the previously set (or default) API base URL.
         *
         * @return const std::string& API base URL.
         */
        const std::string& baseURL() const;

        /**
         * @brief Sets the scheme alias for the tile server. For example mapbox:// for Mapbox.
         *
         * @param alias The URI alias.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withUriSchemeAlias(std::string alias);

        /**
         * @brief Gets the previously set (or default) tile server URI alias.
         *
         * @return const std::string& scheme alias.
         */
        const std::string& uriSchemeAlias() const;

        /**
         * @brief Sets the template for sources.
         *
         * @param sourceTemplate The source template.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withSourceTemplate(std::string sourceTemplate);

        /**
         * @brief Gets the previously set (or default) source template.
         *
         * @return const std::string& source template.
         */
        const std::string& sourceTemplate() const;

        /**
         * @brief Sets the template for styles.
         *
         * @param styleTemplate The style template.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withStyleTemplate(std::string styleTemplate);

        /**
         * @brief Gets the previously set (or default) style template.
         *
         * @return const std::string& style template.
         */
        const std::string& styleTemplate() const;

        /**
         * @brief Sets the template for sprites.
         *
         * @param spritesTemplate The sprites template.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withSpritesTemplate(std::string spritesTemplate);

        /**
         * @brief Gets the previously set (or default) sprites template.
         *
         * @return const std::string& sprites template.
         */
        const std::string& spritesTemplate() const;

        /**
         * @brief Sets the template for glyphs.
         *
         * @param glyphsTemplate The glyphs template.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withGlyphsTemplate(std::string glyphsTemplate);

        /**
         * @brief Gets the previously set (or default) glyphs template.
         *
         * @return const std::string& glyphs template.
         */
        const std::string& glyphsTemplate() const;

        /**
         * @brief Sets the template for tiles.
         *
         * @param tileTemplate The tile template.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withTileTemplate(std::string tileTemplate);

        /**
         * @brief Gets the previously set (or default) tile template.
         *
         * @return const std::string& tile template.
         */
        const std::string& tileTemplate() const;

        /**
         * @brief Sets the access token parameter name.
         *
         * @param accessTokenParameterName The parameter name.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withAccessTokenParameterName(std::string accessTokenParameterName);

        /**
         * @brief Gets the previously set (or default) accessTokenParameterName.
         *
         * @return const std::string& accessTokenParameterName.
         */
        const std::string& accessTokenParameterName() const;

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace mbgl
