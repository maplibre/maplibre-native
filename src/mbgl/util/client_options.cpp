#include <mbgl/util/client_options.hpp>

namespace mbgl {

class ClientOptions::Impl {
public:
    std::string name;
    std::string version;
};

// These requires the complete type of Impl.
ClientOptions::ClientOptions()
    : impl_(std::make_unique<Impl>()) {}
ClientOptions::~ClientOptions() = default;
ClientOptions::ClientOptions(ClientOptions&&) noexcept = default;
ClientOptions::ClientOptions(const ClientOptions& other)
    : impl_(std::make_unique<Impl>(*other.impl_)) {}
ClientOptions& ClientOptions::operator=(const ClientOptions& other) {
    impl_ = std::make_unique<Impl>(*other.impl_);
    return *this;
}
ClientOptions& ClientOptions::operator=(ClientOptions&& options) {
    swap(impl_, options.impl_);
    return *this;
}

ClientOptions ClientOptions::clone() const {
    return ClientOptions(*this);
}

ClientOptions& ClientOptions::withName(std::string name) {
    impl_->name = std::move(name);
    return *this;
}

const std::string& ClientOptions::name() const {
    return impl_->name;
}

ClientOptions& ClientOptions::withVersion(std::string version) {
    impl_->version = std::move(version);
    return *this;
}

const std::string& ClientOptions::version() const {
    return impl_->version;
}

} // namespace mbgl
