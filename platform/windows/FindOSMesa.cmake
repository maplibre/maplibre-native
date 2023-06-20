# Standard FIND_PACKAGE module for OSMesa, sets the following variables:
#   - OSMesa_FOUND
#   - OSMesa_INCLUDE_DIRS (only if OSMesa_FOUND)
#   - OSMesa_LIBRARIES (only if OSMesa_FOUND)

# Try to find the header
find_path(OSMesa_INCLUDE_DIR NAMES GL/osmesa.h)

get_filename_component(OSMesa_DIR ${OSMesa_INCLUDE_DIR}/../ ABSOLUTE)

# Try to find the library
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_ARCH x64)
else()
    set(_ARCH x86)
endif()

find_library(OSMesa_osmesa_LIBRARY     NAMES osmesa        PATHS ${OSMesa_DIR}/lib PATH_SUFFIXES ${_ARCH} NO_DEFAULT_PATH)
find_library(OSMesa_libGLESv2_LIBRARY  NAMES libGLESv2     PATHS ${OSMesa_DIR}/lib PATH_SUFFIXES ${_ARCH} NO_DEFAULT_PATH)

find_file(OSMesa_osmesa_LIBRARY_DLL    NAMES osmesa.dll    PATHS ${OSMesa_DIR} PATH_SUFFIXES ${_ARCH} NO_DEFAULT_PATH)
find_file(OSMesa_libGLESv2_LIBRARY_DLL NAMES libGLESv2.dll PATHS ${OSMesa_DIR} PATH_SUFFIXES ${_ARCH} NO_DEFAULT_PATH)

unset(_ARCH)

# Handle the QUIETLY/REQUIRED arguments, set OSMesa_FOUND if all variables are
# found
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSMesa
                                  REQUIRED_VARS
                                  OSMesa_osmesa_LIBRARY
                                  OSMesa_libGLESv2_LIBRARY
                                  OSMesa_INCLUDE_DIR)

# Hide internal variables
mark_as_advanced(OSMesa_INCLUDE_DIR OSMesa_osmesa_LIBRARY OSMesa_libGLESv2_LIBRARY)

# Set standard variables
if(OSMesa_FOUND)
    set(OSMesa_INCLUDE_DIRS "${OSMesa_INCLUDE_DIR}")
    set(OSMesa_LIBRARIES
        "${OSMesa_osmesa_LIBRARY}"
        "${OSMesa_libGLESv2_LIBRARY}"
    )

    add_library(OSMesa::osmesa SHARED IMPORTED)

    set_target_properties(
        OSMesa::osmesa
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${OSMesa_INCLUDE_DIRS}
            IMPORTED_IMPLIB               ${OSMesa_osmesa_LIBRARY}
            IMPORTED_LOCATION             ${OSMesa_osmesa_LIBRARY_DLL}
    )

    add_library(OSMesa::libGLESv2 SHARED IMPORTED)

    set_target_properties(
        OSMesa::libGLESv2
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${OSMesa_INCLUDE_DIRS}
            IMPORTED_IMPLIB               ${OSMesa_libGLESv2_LIBRARY}
            IMPORTED_LOCATION             ${OSMesa_libGLESv2_LIBRARY_DLL}
    )
endif()
