
#ifndef MLNStyleFilter_Private_h
#define MLNStyleFilter_Private_h

#include <mbgl/plugin/plugin_style_filter.hpp>
#import "MLNStyleFilter.h"

@interface MLNStyleFilter (Private)

- (void)setFilter:(std::shared_ptr<mbgl::style::PluginStyleFilter>)filter;

@end

#endif /* MLNStyleFilter_Private_h */
