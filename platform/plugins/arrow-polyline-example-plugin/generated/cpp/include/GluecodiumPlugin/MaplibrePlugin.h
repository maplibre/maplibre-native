// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#pragma once

#include "glue_internal/ExportGluecodiumCpp.h"
#include "glue_internal/TypeRepository.h"
#include <cstdint>
#include <memory>

namespace GluecodiumPlugin {
/**
 * Base class for gluecodium-based maplibre plugins
 * (In a full integration, this would be in a shared package)

 */
class _GLUECODIUM_CPP_EXPORT MaplibrePlugin {
public:
    MaplibrePlugin();
    virtual ~MaplibrePlugin();


public:
    /**
     *
     * \return @NotNull
     */
    static ::std::shared_ptr< ::GluecodiumPlugin::MaplibrePlugin > create(  );
    /**
     *
     * \return Pointer to the underlying C++ class that implements XPlatformPlugin
     */
    virtual uint64_t get_ptr(  ) const = 0;

};


}
