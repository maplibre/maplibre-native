#include "settings_json.hpp"
#include <fstream>
#include <mbgl/util/constants.hpp>
#include <mbgl/math/angles.hpp>

namespace mbgl {

Settings_JSON::Settings_JSON() {
    load();
}

void Settings_JSON::load() {
    std::ifstream file("/tmp/mbgl-native.cfg");
    if (file) {
        file >> longitude;
        file >> latitude;
        file >> altitude;
        file >> zoom;
        file >> bearing;
        file >> pitch;
        file >> roll;
        file >> fov;
        file >> debug;
        file >> online;
    }
}

void Settings_JSON::save() {
    std::ofstream file("/tmp/mbgl-native.cfg");
    if (file) {
        file << longitude << std::endl;
        file << latitude << std::endl;
        file << altitude << std::endl;
        file << zoom << std::endl;
        file << bearing << std::endl;
        file << pitch << std::endl;
        file << roll << std::endl;
        file << fov << std::endl;
        file << debug << std::endl;
        file << online << std::endl;
    }
}

void Settings_JSON::clear() {
    longitude = 0;
    latitude = 0;
    altitude = 0;
    zoom = 0;
    bearing = 0;
    pitch = 0;
    roll = 0;
    fov = util::rad2deg(util::DEFAULT_FOV);
    debug = 0;
    online = true;
}

} // namespace mbgl
