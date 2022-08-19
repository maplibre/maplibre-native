#pragma once

#include <memory>
#include <string>

namespace mbgl {

/**
 * @brief Holds values for client options.
 */
class ClientOptions final {
public:
    /**
     * @brief Constructs a ClientOptions object with default values.
     */
    ClientOptions();
    ~ClientOptions();

    ClientOptions(ClientOptions&&) noexcept;
    ClientOptions& operator=(const ClientOptions& options);
    ClientOptions& operator=(ClientOptions&& options);

    ClientOptions clone() const;

    /**
     * @brief Sets the client name.
     *
     * @param name Client name.
     * @return ClientOptions for chaining options together.
     */
    ClientOptions& withName(std::string name);

    /**
     * @brief Gets the previously set (or default) client name.
     *
     * @return client name
     */
    const std::string& name() const;

    /**
     * @brief Sets the client version.
     *
     * @param version Client version.
     * @return ClientOptions for chaining options together.
     */
    ClientOptions& withVersion(std::string version);

    /**
     * @brief Gets the previously set (or default) client version.
     *
     * @return client version
     */
    const std::string& version() const;

private:
    ClientOptions(const ClientOptions&);

    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace mbgl
