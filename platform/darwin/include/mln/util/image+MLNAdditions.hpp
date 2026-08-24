#pragma once

#include <mln/util/image.hpp>

#include <CoreGraphics/CGImage.h>

// Creates a CGImage from a PremultipliedImage, taking over the memory ownership.
CGImageRef CGImageCreateWithMLNPremultipliedImage(mln::PremultipliedImage&&);

// Creates a PremultipliedImage by copying the pixels of the CGImage.
// Does not alter the retain count of the supplied CGImage.
mln::PremultipliedImage MLNPremultipliedImageFromCGImage(CGImageRef);
