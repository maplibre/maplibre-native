//
//  plugin.hpp
//  App
//
//  Created by Malcolm Toon on 11/26/25.
//

#ifndef plugin_hpp
#define plugin_hpp

#include <stdio.h>
#include <string>

namespace mbgl { namespace plugin {

class Plugin {
public:
    // This is a custom name for the plugin.  These need to be unique across all
    // the same type of plugins.  
    std::string pluginId;
};

} }
#endif /* plugin_hpp */
