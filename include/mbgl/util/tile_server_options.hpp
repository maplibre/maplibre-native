#pragma once

#include <string>
#include <memory>
#include <mbgl/util/default_style.hpp>
#include <mbgl/util/optional.hpp>

namespace mbgl {

/**
 * @brief Holds values for tile server options.
 */
    class TileServerOptions final {
    public:
        /**
         * @brief Constructs a TileServerOptions object with default values.
         */
        explicit TileServerOptions();
        ~TileServerOptions();
        
        // movable
        TileServerOptions(TileServerOptions&& options) noexcept;
        TileServerOptions& operator=(TileServerOptions&& options) noexcept;
        
        // copyable
        TileServerOptions(const TileServerOptions&);

        
        TileServerOptions clone() const;

        /**
         * @brief Sets the API base URL.
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
         * @brief Sets the scheme alias for the tile server. For example maptiler:// for MapTiler.
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
         * @param domainName  The domain name.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withSourceTemplate(std::string sourceTemplate, std::string domainName, optional<std::string> versionPrefix);

        /**
         * @brief Gets the previously set (or default) source template.
         *
         * @return const std::string& source template.
         */
        const std::string& sourceTemplate() const;
        
        /**
         * @brief Gets the previously set (or default) source domain name
         *
         * @return const std::string& source domain name.
         */
        const std::string& sourceDomainName() const;
        
        /**
         * @brief Gets the previously set (or default) version prefix
         *
         * @return const optional<std::string>& version prefix.
         */
        const optional<std::string>& sourceVersionPrefix() const;

        /**
         * @brief Sets the template for styles.
         *
         * @param styleTemplate The style template.
         * @param domainName If set, the URL domain must contain the specified string to be matched as canonical style URL .
         *
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withStyleTemplate(std::string styleTemplate, std::string domainName, optional<std::string> versionPrefix);

        /**
         * @brief Gets the previously set (or default) style template.
         *
         * @return const std::string& style template.
         */
        const std::string& styleTemplate() const;
        
        /**
         * @brief Gets the previously set (or default) style domain name.
         *
         * @return const std::string& domain name.
         */
        const std::string& styleDomainName() const;
        
        /**
         * @brief Gets the previously set (or default) version prefix
         *
         * @return const optional<std::string>& version prefix.
         */
        const optional<std::string>& styleVersionPrefix() const;

        /**
         * @brief Sets the template for sprites.
         * @param domainName If set, the URL domain must contain the specified string to be matched as canonical sprite URL .
         *
         * @param spritesTemplate The sprites template.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withSpritesTemplate(std::string spritesTemplate, std::string domainName, optional<std::string> versionPrefix);

        /**
         * @brief Gets the previously set (or default) sprites template.
         *
         * @return const std::string& sprites template.
         */
        const std::string& spritesTemplate() const;
        
        /**
         * @brief Gets the previously set (or default) sprites domain name.
         *
         * @return const std::string& domain name.
         */
        const std::string& spritesDomainName() const;
        
        /**
         * @brief Gets the previously set (or default) version prefix
         *
         * @return const optional<std::string>& version prefix.
         */
        const optional<std::string>& spritesVersionPrefix() const;

        /**
         * @brief Sets the template for glyphs.
         *
         * @param glyphsTemplate The glyphs template.
         * @param domainName If set, the URL domain must contain the specified string to be matched as canonical glyphs URL .
         *
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withGlyphsTemplate(std::string glyphsTemplate, std::string domainName, optional<std::string> versionPrefix);

        /**
         * @brief Gets the previously set (or default) glyphs template.
         *
         * @return const std::string& glyphs template.
         */
        const std::string& glyphsTemplate() const;
        
        /**
         * @brief Gets the previously set (or default) glyphs domain name.
         *
         * @return const std::string& domain name.
         */
        const std::string& glyphsDomainName() const;
        
        /**
         * @brief Gets the previously set (or default) version prefix
         *
         * @return const optional<std::string>& version prefix.
         */
        const optional<std::string>& glyphsVersionPrefix() const;

        /**
         * @brief Sets the template for tiles.
         *
         * @param tileTemplate The tile template.
         * @param domainName If set, the URL domain must contain the specified string to be matched as canonical tile URL .
         *
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withTileTemplate(std::string tileTemplate, std::string domainName, optional<std::string> versionPrefix);

        /**
         * @brief Gets the previously set (or default) tile template.
         *
         * @return const std::string& tile template.
         */
        const std::string& tileTemplate() const;

        /**
         * @brief Gets the previously set (or default) tile domain name.
         *
         * @return const std::string& domain name.
         */
        const std::string& tileDomainName() const;
        
        /**
         * @brief Gets the previously set (or default) version prefix
         *
         * @return const optional<std::string>& version prefix.
         */
        const optional<std::string>& tileVersionPrefix() const;
        
        /**
         * @brief Sets the access token parameter name.
         *
         * @param apiKeyParameterName The parameter name.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withApiKeyParameterName(std::string apiKeyParameterName);

        /**
         * @brief Gets the previously set (or default) apiKeyParameterName.
         *
         * @return const std::string& apiKeyParameterName.
         */
        const std::string& apiKeyParameterName() const;
        
        TileServerOptions& setRequiresApiKey(bool apiKeyRequired);

        /**
         * @brief Whether the tile server requires API key
         *
         * @return const bool true if API key is required
         */
        bool requiresApiKey() const;
        /**
         * @brief Gets the default styles.
         */
        const std::vector<mbgl::util::DefaultStyle> defaultStyles() const;

        /**
         * @brief Sets the collection default styles.
         *
         * @param styles The style set.
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withDefaultStyles(std::vector<mbgl::util::DefaultStyle> styles);

        /**
         * @brief Sets the default style by name. The style name must exists in defaultStyles collection
         *
         * @param defaultStyle The style name
         * @return TileServerOptions for chaining options together.
         */
        TileServerOptions& withDefaultStyle(std::string defaultStyle);

        /**
         * @brief Get the default style name
         */
        const std::string& defaultStyle() const;

        /**
         * @brief Get the default configuration
         */
        static TileServerOptions DefaultConfiguration();

        /**
         * @brief Get the tile server options configured for MapLibre.
         */
        static TileServerOptions MapLibreConfiguration();

        /**
         * @brief Get the tile server options configured for Mapbox.
         */
        static TileServerOptions MapboxConfiguration();

        /**
         * @brief Get the tile server options configured for MapTiler.
         */
        static TileServerOptions MapTilerConfiguration();

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace mbgl
