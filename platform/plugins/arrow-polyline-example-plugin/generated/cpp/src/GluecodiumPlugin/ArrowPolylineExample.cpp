// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#include "GluecodiumPlugin/ArrowPolylineExample.h"

namespace GluecodiumPlugin {
ArrowPolylineExample::ArrowPolylineExample() {
    ::glue_internal::get_type_repository().add_type(this, "GluecodiumPlugin_ArrowPolylineExample");
}
ArrowPolylineExample::~ArrowPolylineExample() {
    ::glue_internal::get_type_repository().remove_type(this);
}


}

