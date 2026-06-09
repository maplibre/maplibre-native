//
//  StyleFilterPlugin.hpp
//  App
//
//  Created by Malcolm Toon on 11/26/25.
//

#ifndef StyleFilterPlugin_hpp
#define StyleFilterPlugin_hpp

#include "plugin.hpp"

#include <stdio.h>
#include <string>

namespace mbgl { namespace plugin {


    class StylePreprocessor : public Plugin {
    public:
        virtual ~StylePreprocessor();
        virtual std::string processStyle(const std::string & data);
        
    };


}

}

#endif /* StyleFilterPlugin_hpp */
