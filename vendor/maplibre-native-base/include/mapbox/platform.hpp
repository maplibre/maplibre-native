#pragma once

// Determine compiler
#define MB_COMPILER 0
#define MB_COMPILER_GNU 1
#define MB_COMPILER_CLANG 2
#define MB_COMPILER_MSVC 3

#if defined(__clang__)
#    define MB_COMPILER MB_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__) // after clang, possible it also has the same defines (need to research)
#    define MB_COMPILER MB_COMPILER_GNU
#elif defined(_MSC_VER)
#    define MB_COMPILER MB_COMPILER_MSVC
#else
#    error "Unsupported compiler"
#endif

// Determine platform
#define MB_PLATFORM 0
#define MB_PLATFORM_WIN32 1
#define MB_PLATFORM_LINUX 2
#define MB_PLATFORM_MAC 3
#define MB_PLATFORM_IOS_EMBEDDED 4
#define MB_PLATFORM_IOS_SIMULATOR 5
#define MB_PLATFORM_ANDROID 6
#define MB_PLATFORM_QNX_NEUTRINO 7
#define MB_PLATFORM_QNX_4 8

#if (defined(__WIN32__) || defined(_WIN32)) && !defined(__ANDROID__)
#    define MB_PLATFORM MB_PLATFORM_WIN32
#elif defined(__APPLE_CC__)
#    include "TargetConditionals.h"
#    if TARGET_OS_IOS && TARGET_OS_EMBEDDED
#        define MB_PLATFORM MB_PLATFORM_IOS_EMBEDDED
#    elif TARGET_OS_IOS && TARGET_OS_SIMULATOR
#        define MB_PLATFORM MB_PLATFORM_IOS_SIMULATOR
#    else
#        define MB_PLATFORM MB_PLATFORM_MAC
#    endif
#elif defined(__ANDROID__)
#    define MB_PLATFORM MB_PLATFORM_ANDROID
#elif defined(__QNX__)
#    if defined(__QNXNTO__)
#        define MB_PLATFORM MB_PLATFORM_QNX_NEUTRINO
#    else
#        define MB_PLATFORM MB_PLATFORM_QNX_4
#    endif
#else // let all other platform right now is a linux platforms
#    define MB_PLATFORM MB_PLATFORM_LINUX
#endif

#define MB_PLATFORM_IS_WIN32 (MB_PLATFORM == MB_PLATFORM_WIN32)
#define MB_PLATFORM_IS_LINUX (MB_PLATFORM == MB_PLATFORM_LINUX)
#define MB_PLATFORM_IS_MAC (MB_PLATFORM == MB_PLATFORM_MAC)
#define MB_PLATFORM_IS_IOS_EMBEDDED (MB_PLATFORM == MB_PLATFORM_IOS_EMBEDDED)
#define MB_PLATFORM_IS_IOS_SIMULATOR (MB_PLATFORM == MB_PLATFORM_IOS_SIMULATOR)
#define MB_PLATFORM_IS_IOS (MB_PLATFORM_IS_IOS_EMBEDDED || MB_PLATFORM_IS_IOS_SIMULATOR)
#define MB_PLATFORM_IS_ANDROID (MB_PLATFORM == MB_PLATFORM_ANDROID)
#define MB_PLATFORM_IS_QNX_NEUTRINO (MB_PLATFORM == MB_PLATFORM_QNX_NEUTRINO)
#define MB_PLATFORM_IS_QNX_4 (MB_PLATFORM == MB_PLATFORM_QNX_4)
#define MB_PLATFORM_IS_QNX (MB_PLATFORM_IS_QNX_NEUTRINO || MB_PLATFORM_IS_QNX_4)

#define MB_PLATFORM_IS_DESKTOP (MB_PLATFORM_IS_LINUX || MB_PLATFORM_IS_MAC || MB_PLATFORM_IS_WIN32)

#ifdef NDEBUG
#    define MB_IS_DEBUG 0
#else
#    define MB_IS_DEBUG 1
#endif
