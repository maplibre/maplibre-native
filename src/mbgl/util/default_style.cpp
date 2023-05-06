#include <mbgl/util/default_style.hpp>

namespace mbgl {
namespace util {

DefaultStyle::DefaultStyle(std::string url_, std::string name_, int version_)
    : url(url_),
      name(name_),
      currentVersion(version_) {}

const std::string& DefaultStyle::getUrl() const {
    return url;
}

const std::string& DefaultStyle::getName() const {
    return name;
}

int DefaultStyle::getCurrentVersion() const {
    return currentVersion;
}

} // end namespace util
} // end namespace mbgl
