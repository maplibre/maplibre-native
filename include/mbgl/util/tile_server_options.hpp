#pragma once

#include <string>
#include <memory>
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
        TileServerOptions& operator=(TileServerOptions& options);
        
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
        TileServerOptions& withSourceTemplate(std::string sourceTemplate, optional<std::string> versionPrefix);

        /**
         * @brief Gets the previously set (or default) source template.
         *
         * @return const std::string& source template.
         */
        const std::string& sourceTemplate() const;
        
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

        /**
         * @brief Get the default configuration
         */
        static TileServerOptions DefaultConfiguration();

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
