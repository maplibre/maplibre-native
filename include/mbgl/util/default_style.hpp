#pragma once

#include <array>
#include <string>
#include <vector>

namespace mbgl {
namespace util {

class DefaultStyle {
public:
    DefaultStyle(std::string, std::string, int);

    const std::string& getUrl() const;
    const std::string& getName() const; 
    int getCurrentVersion() const;

private:
    std::string url;
    std::string name;
    int currentVersion;
};

} // end namespace util
} // end namespace mbgl
