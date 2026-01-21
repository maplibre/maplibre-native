#import "MLNStyleFilter.h"
#include <mbgl/plugin/plugin_style_filter.hpp>

@interface MLNStyleFilter () {
    std::shared_ptr<mbgl::style::PluginStyleFilter> _coreFilter;
}

@end

@implementation MLNStyleFilter

-(NSData *)filterData:(NSData *)data {
    // Base class does nothing but return the same data passed in
    return data;
}

// Private
-(void)setFilter:(std::shared_ptr<mbgl::style::PluginStyleFilter>)filter {
    _coreFilter = filter;
}


@end
