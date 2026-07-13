#pragma once

#ifndef NDEBUG

#include <thread>
#define MBGL_STORE_THREAD(tid) const std::thread::id tid = std::this_thread::get_id();
#define MBGL_VERIFY_THREAD(tid) assert(tid == std::this_thread::get_id());

#else

#define MBGL_STORE_THREAD(tid)
#define MBGL_VERIFY_THREAD(tid)

#endif

// Compiler defines for making symbols visible, otherwise they
// will be defined as hidden by default.

// clang-format off
#ifdef WIN32
    #ifdef MBGL_BUILDING_LIB
        #ifdef __GNUC__
            #define MBGL_EXPORT __attribute__((dllexport))
        #else
            #define MBGL_EXPORT __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define MBGL_EXPORT __attribute__((dllimport))
        #else
            #define MBGL_EXPORT __declspec(dllimport)
        #endif
    #endif
#else
    #define MBGL_EXPORT __attribute__((visibility ("default"))) // NOLINT
#endif
// clang-format on
