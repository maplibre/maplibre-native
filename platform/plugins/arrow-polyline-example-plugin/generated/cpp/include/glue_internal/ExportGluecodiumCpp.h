// -------------------------------------------------------------------------------------------------
//

//
// -------------------------------------------------------------------------------------------------

#pragma once

#ifdef _GLUECODIUM_CPP_SHARED
#   if defined(_WIN32) || defined(__CYGWIN)
#       ifdef _GLUECODIUM_CPP_INTERNAL
#           define _GLUECODIUM_CPP_EXPORT __declspec( dllexport )
#       else
#           define _GLUECODIUM_CPP_EXPORT __declspec( dllimport )
#       endif
#   else
#       define _GLUECODIUM_CPP_EXPORT __attribute__( ( visibility( "default" ) ) )
#   endif
#else
#   define _GLUECODIUM_CPP_EXPORT
#endif
