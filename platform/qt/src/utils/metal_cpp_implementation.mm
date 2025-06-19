// Copyright (C) 2024 MapLibre contributors
// SPDX-License-Identifier: BSD-2-Clause
//
// This translation unit instantiates the Objective-C selector and class
// singletons required by the header-only metal-cpp wrappers.  It must be
// compiled exactly once inside the target that uses Metal.
//
// The pattern follows the metal-cpp README:
//   https://github.com/metal-cpp/metal-cpp#usage

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_OSX

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>

#endif // TARGET_OS_OSX
#endif // __APPLE__
