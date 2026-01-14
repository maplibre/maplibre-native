// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "glue_internal/ExportGluecodiumCpp.h"
#include <string>

namespace GluecodiumPlugin {
/**
 * Configuration for arrow polyline appearance

 */
struct _GLUECODIUM_CPP_EXPORT ArrowPolylineConfig {
    /**
     * Length of the arrow head in pixels

     */
    double head_length = 50.0;
    /**
     * Angle of the arrow head in degrees

     */
    double head_angle = 30.0;
    /**
     * Color of the arrow line as hex string (e.g., "#FF0000")

     */
    ::std::string line_color = "#FF0000";
    /**
     * Width of the arrow line in pixels

     */
    double line_width = 3.0;

    ArrowPolylineConfig();
    ArrowPolylineConfig(double head_length, double head_angle, ::std::string line_color, double line_width);
};

} // namespace GluecodiumPlugin
