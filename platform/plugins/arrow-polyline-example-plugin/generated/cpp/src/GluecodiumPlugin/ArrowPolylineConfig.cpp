// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#include "GluecodiumPlugin/ArrowPolylineConfig.h"
#include <utility>

namespace GluecodiumPlugin {

ArrowPolylineConfig::ArrowPolylineConfig() {}

ArrowPolylineConfig::ArrowPolylineConfig(double head_length,
                                         double head_angle,
                                         ::std::string line_color,
                                         double line_width)
    : head_length(std::move(head_length)),
      head_angle(std::move(head_angle)),
      line_color(std::move(line_color)),
      line_width(std::move(line_width)) {}

} // namespace GluecodiumPlugin
