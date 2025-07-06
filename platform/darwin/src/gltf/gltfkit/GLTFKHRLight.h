//
//  Copyright (c) 2018 Warren Moore. All rights reserved.
//
//  Permission to use, copy, modify, and distribute this software for any
//  purpose with or without fee is hereby granted, provided that the above
//  copyright notice and this permission notice appear in all copies.
//
//  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
//  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
//  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
//  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
//  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
//  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
//  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#import "GLTFObject.h"

#import <simd/simd.h>

typedef NS_ENUM(NSInteger, GLTFKHRLightType) {
  GLTFKHRLightTypeAmbient,
  GLTFKHRLightTypeDirectional,
  GLTFKHRLightTypePoint,
  GLTFKHRLightTypeSpot,
};

@interface GLTFKHRLight : GLTFObject

@property (nonatomic, assign) GLTFKHRLightType type;

/// Color of light in a linear RGB color space
@property (nonatomic, assign) simd_float4 color;

/// Brightness of light. Point and spot lights use luminous intensity in candela (lm/sr),
/// while directional lights use illuminance in lux (lm/m^2).
@property (nonatomic, assign) float intensity;

/// Distance threshold at which the light's intensity may be considered to have reached zero,
/// expressed in meters. Default is 0, signifying effectively infinite range.
@property (nonatomic, assign) float range;

/// Angle, in radians, from the center of a spotlight to where falloff begins.
/// Must be greater than or equal to 0, less than or equal to `outerConeAngle`,
/// and less than pi / 2. Default value is 0.
@property (nonatomic, assign) float innerConeAngle;

/// Angle, in radians, from the center of a spotlight to where falloff ends.
/// Must be greater than or equal to 0, greater than or equal to `innerConeAngle`,
/// and less than pi / 2. Default value is pi / 4.
@property (nonatomic, assign) float outerConeAngle;

@end
