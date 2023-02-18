# We need CMake 3.9 because the Xcode generator doesn't know about systems include paths before 3.9.
# See https://gitlab.kitware.com/cmake/cmake/issues/16795
cmake_minimum_required(VERSION 3.9)

if (NOT NODE_MODULE_MINIMUM_ABI)
    set(NODE_MODULE_MINIMUM_ABI 46) # Don't build node modules for versions earlier than Node 4
endif()
if (NOT NODE_MODULE_CACHE_DIR)
    set(NODE_MODULE_CACHE_DIR "${CMAKE_BINARY_DIR}")
endif()


function(_node_module_download _TYPE _URL _FILE)
    file(REMOVE_RECURSE "${_FILE}")
    string(RANDOM LENGTH 32 _TMP)
    set(_TMP "${CMAKE_BINARY_DIR}/${_TMP}")
    message(STATUS "[Node.js] Downloading ${_TYPE}...")
    file(DOWNLOAD "${_URL}" "${_TMP}" STATUS _STATUS TLS_VERIFY ON)
    list(GET _STATUS 0 _STATUS_CODE)
    if(NOT _STATUS_CODE EQUAL 0)
        file(REMOVE "${_TMP}")
        list(GET _STATUS 1 _STATUS_MESSAGE)
        message(FATAL_ERROR "[Node.js] Failed to download ${_TYPE}: ${_STATUS_MESSAGE}")
    else()
        get_filename_component(_DIR "${_FILE}" DIRECTORY)
        file(MAKE_DIRECTORY "${_DIR}")
        file(RENAME "${_TMP}" "${_FILE}")
    endif()
endfunction()


function(_node_module_unpack_tar_gz _TYPE _URL _PATH _DEST)
    string(RANDOM LENGTH 32 _TMP)
    set(_TMP "${CMAKE_BINARY_DIR}/${_TMP}")
    _node_module_download("${_TYPE}" "${_URL}" "${_TMP}.tar.gz")
    file(REMOVE_RECURSE "${_DEST}" "${_TMP}")
    file(MAKE_DIRECTORY "${_TMP}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xfz "${_TMP}.tar.gz"
        WORKING_DIRECTORY "${_TMP}"
        RESULT_VARIABLE _STATUS_CODE
        OUTPUT_VARIABLE _STATUS_MESSAGE
        ERROR_VARIABLE _STATUS_MESSAGE)
    if(NOT _STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "[Node.js] Failed to unpack ${_TYPE}: ${_STATUS_MESSAGE}")
    endif()
    get_filename_component(_DIR "${_DEST}" DIRECTORY)
    file(MAKE_DIRECTORY "${_DIR}")
    file(RENAME "${_TMP}/${_PATH}" "${_DEST}")
    file(REMOVE_RECURSE "${_TMP}" "${_TMP}.tar.gz")
endfunction()


function(add_node_module NAME)
    cmake_parse_arguments("" "" "MINIMUM_NODE_ABI;NAN_VERSION;INSTALL_PATH;CACHE_DIR" "EXCLUDE_NODE_ABIS" ${ARGN})
    if(NOT _MINIMUM_NODE_ABI)
        set(_MINIMUM_NODE_ABI "${NODE_MODULE_MINIMUM_ABI}")
    endif()
    if (NOT _CACHE_DIR)
        set(_CACHE_DIR "${NODE_MODULE_CACHE_DIR}")
    endif()
    if (NOT _INSTALL_PATH)
        set(_INSTALL_PATH "lib/{node_abi}/${NAME}.node")
    endif()
    get_filename_component(_CACHE_DIR "${_CACHE_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    if(_UNPARSED_ARGUMENTS)
        message(WARNING "[Node.js] Unused arguments: '${_UNPARSED_ARGUMENTS}'")
    endif()


    # Create master target
    add_library(${NAME} INTERFACE)


    # Obtain a list of current Node versions and retrieves the latest version per ABI
    if(NOT EXISTS "${_CACHE_DIR}/node/index.tab")
        _node_module_download(
            "Node.js version list"
            "https://nodejs.org/dist/index.tab"
            "${_CACHE_DIR}/node/index.tab"
        )
    endif()
    file(STRINGS "${_CACHE_DIR}/node/index.tab" _INDEX_FILE)
    list(REMOVE_AT _INDEX_FILE 0)
    set(_ABIS)
    foreach(_LINE IN LISTS _INDEX_FILE)
        string(REGEX MATCHALL "[^\t]*\t" _COLUMNS "${_LINE}")
        list(GET _COLUMNS 8 _ABI)
        string(STRIP "${_ABI}" _ABI)
        if((_ABI GREATER _MINIMUM_NODE_ABI OR _ABI EQUAL _MINIMUM_NODE_ABI) AND NOT _ABI IN_LIST _EXCLUDE_NODE_ABIS AND NOT DEFINED _NODE_ABI_${_ABI}_VERSION)
            list(APPEND _ABIS ${_ABI})
            list(GET _COLUMNS 0 _VERSION)
            string(STRIP "${_VERSION}" _NODE_ABI_${_ABI}_VERSION)
        endif()
    endforeach()


    # Install Nan
    if(_NAN_VERSION AND NOT EXISTS "${_CACHE_DIR}/nan/${_NAN_VERSION}/nan.h")
        _node_module_unpack_tar_gz(
            "Nan ${_NAN_VERSION}"
            "https://registry.npmjs.org/nan/-/nan-${_NAN_VERSION}.tgz"
            "package"
            "${_CACHE_DIR}/nan/${_NAN_VERSION}"
        )
    endif()


    # Generate a target for every ABI
    set(_TARGETS)
    foreach(_ABI IN LISTS _ABIS)
        set(_NODE_VERSION ${_NODE_ABI_${_ABI}_VERSION})

        # Download the headers if we don't have them yet
        if(NOT EXISTS "${_CACHE_DIR}/node/${_NODE_VERSION}/node.h")
            _node_module_unpack_tar_gz(
                "headers for Node ${_NODE_VERSION}"
                "https://nodejs.org/download/release/${_NODE_VERSION}/node-${_NODE_VERSION}-headers.tar.gz"
                "node-${_NODE_VERSION}/include/node"
                "${_CACHE_DIR}/node/${_NODE_VERSION}"
            )
        endif()


        # Generate the library
        set(_TARGET "${NAME}.abi-${_ABI}")
        add_library(${_TARGET} SHARED "${_CACHE_DIR}/empty.cpp")
        list(APPEND _TARGETS "${_TARGET}")


        # C identifiers can only contain certain characters (e.g. no dashes)
        string(REGEX REPLACE "[^a-z0-9]+" "_" NAME_IDENTIFIER "${NAME}")

        set_target_properties(${_TARGET} PROPERTIES
            OUTPUT_NAME "${_TARGET}"
            SOURCES "" # Removes the fake empty.cpp again
            PREFIX ""
            SUFFIX ".node"
            MACOSX_RPATH ON
            C_VISIBILITY_PRESET hidden
            CXX_VISIBILITY_PRESET hidden
            POSITION_INDEPENDENT_CODE TRUE
        )

        target_compile_definitions(${_TARGET} PRIVATE
            "MODULE_NAME=${NAME_IDENTIFIER}"
            "BUILDING_NODE_EXTENSION"
            "_LARGEFILE_SOURCE"
            "_FILE_OFFSET_BITS=64"
        )

        target_include_directories(${_TARGET} SYSTEM PRIVATE
            "${_CACHE_DIR}/node/${_NODE_VERSION}"
        )

        if(_NAN_VERSION)
            # Nan requires C++11. Use a compile option to allow interfaces to override this with a later version.
            target_compile_options(${_TARGET} PRIVATE -std=c++11)
            target_include_directories(${_TARGET} SYSTEM PRIVATE
                "${_CACHE_DIR}/nan/${_NAN_VERSION}"
            )
        endif()

        target_link_libraries(${_TARGET} PRIVATE ${NAME})

        if(APPLE)
            # Ensures that linked symbols are loaded when the module is loaded instead of causing
            # unresolved symbol errors later during runtime.
            set_target_properties(${_TARGET} PROPERTIES
                LINK_FLAGS "-undefined dynamic_lookup -bind_at_load"
            )
            target_compile_definitions(${_TARGET} PRIVATE
                "_DARWIN_USE_64_BIT_INODE=1"
            )
        else()
            # Ensures that linked symbols are loaded when the module is loaded instead of causing
            # unresolved symbol errors later during runtime.
            set_target_properties(${_TARGET} PROPERTIES
                LINK_FLAGS "-z now"
            )
        endif()

        # Copy the file to the installation directory.
        string(REPLACE "{node_abi}" "node-v${_ABI}" _OUTPUT_PATH "${_INSTALL_PATH}")
        get_filename_component(_OUTPUT_PATH "${_OUTPUT_PATH}" ABSOLUTE "${CMAKE_CURRENT_SOURCE_PATH}")
        add_custom_command(
            TARGET ${_TARGET}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${_TARGET}>" "${_OUTPUT_PATH}"
        )
    endforeach()

    # Add a target that builds all Node ABIs.
    add_custom_target("${NAME}.all")
    add_dependencies("${NAME}.all" ${_TARGETS})

    # Add a variable that allows users to iterate over all of the generated/dependendent targets.
    set("${NAME}::abis" "${_ABIS}" PARENT_SCOPE)
    set("${NAME}::targets" "${_TARGETS}" PARENT_SCOPE)
endfunction()
