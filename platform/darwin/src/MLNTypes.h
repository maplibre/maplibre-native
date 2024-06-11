#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>

#import "MLNFoundation.h"

#pragma once

#if TARGET_OS_IPHONE
@class UIImage;
#define MLNImage UIImage
#else
@class NSImage;
#define MLNImage NSImage
#endif

#if TARGET_OS_IPHONE
@class UIColor;
#define MLNColor UIColor
#else
@class NSColor;
#define MLNColor NSColor
#endif

NS_ASSUME_NONNULL_BEGIN

typedef NSString *MLNExceptionName NS_TYPED_EXTENSIBLE_ENUM;

/**
 :nodoc: Generic exceptions used across multiple disparate classes. Exceptions
 that are unique to a class or class-cluster should be defined in those headers.
 */
FOUNDATION_EXTERN MLN_EXPORT MLNExceptionName const MLNAbstractClassException;

/** Indicates an error occurred in the Mapbox SDK. */
FOUNDATION_EXTERN MLN_EXPORT NSErrorDomain const MLNErrorDomain;

/** Error constants for the Mapbox SDK. */
typedef NS_ENUM(NSInteger, MLNErrorCode) {
  /** An unknown error occurred. */
  MLNErrorCodeUnknown = -1,
  /** The resource could not be found. */
  MLNErrorCodeNotFound = 1,
  /** The connection received an invalid server response. */
  MLNErrorCodeBadServerResponse = 2,
  /** An attempt to establish a connection failed. */
  MLNErrorCodeConnectionFailed = 3,
  /** A style parse error occurred while attempting to load the map. */
  MLNErrorCodeParseStyleFailed = 4,
  /** An attempt to load the style failed. */
  MLNErrorCodeLoadStyleFailed = 5,
  /** An error occurred while snapshotting the map. */
  MLNErrorCodeSnapshotFailed = 6,
  /** Source is in use and cannot be removed */
  MLNErrorCodeSourceIsInUseCannotRemove = 7,
  /** Source is in use and cannot be removed */
  MLNErrorCodeSourceIdentifierMismatch = 8,
  /** An error occurred while modifying the offline storage database */
  MLNErrorCodeModifyingOfflineStorageFailed = 9,
  /** Source is invalid and cannot be removed from the style (e.g. after a style change) */
  MLNErrorCodeSourceCannotBeRemovedFromStyle = 10,
  /** An error occurred while rendering */
  MLNErrorCodeRenderingError = 11,
};

/** Options for enabling debugging features in an `MLNMapView` instance. */
typedef NS_OPTIONS(NSUInteger, MLNMapDebugMaskOptions) {
  /** Edges of tile boundaries are shown as thick, red lines to help diagnose
      tile clipping issues. */
  MLNMapDebugTileBoundariesMask = 1 << 1,
  /** Each tile shows its tile coordinate (x/y/z) in the upper-left corner. */
  MLNMapDebugTileInfoMask = 1 << 2,
  /** Each tile shows a timestamp indicating when it was loaded. */
  MLNMapDebugTimestampsMask = 1 << 3,
  /** Edges of glyphs and symbols are shown as faint, green lines to help
      diagnose collision and label placement issues. */
  MLNMapDebugCollisionBoxesMask = 1 << 4,
  /** Each drawing operation is replaced by a translucent fill. Overlapping
      drawing operations appear more prominent to help diagnose overdrawing.
      @note This option does nothing in Release builds of the SDK. */
  MLNMapDebugOverdrawVisualizationMask = 1 << 5,
#if !TARGET_OS_IPHONE
  /** The stencil buffer is shown instead of the color buffer.
      @note This option does nothing in Release builds of the SDK. */
  MLNMapDebugStencilBufferMask = 1 << 6,
  /** The depth buffer is shown instead of the color buffer.
      @note This option does nothing in Release builds of the SDK. */
  MLNMapDebugDepthBufferMask = 1 << 7,
#endif
};

/**
 A structure containing information about a transition.
 */
typedef struct __attribute__((objc_boxable)) MLNTransition {
  /**
   The amount of time the animation should take, not including the delay.
   */
  NSTimeInterval duration;

  /**
   The amount of time in seconds to wait before beginning the animation.
   */
  NSTimeInterval delay;
} MLNTransition;

NS_INLINE NSString *MLNStringFromMLNTransition(MLNTransition transition) {
  return [NSString stringWithFormat:@"transition { duration: %f, delay: %f }", transition.duration,
                                    transition.delay];
}

/**
 Creates a new `MLNTransition` from the given duration and delay.

 @param duration The amount of time the animation should take, not including
 the delay.
 @param delay The amount of time in seconds to wait before beginning the
 animation.

 @return Returns a `MLNTransition` struct containing the transition attributes.
 */
NS_INLINE MLNTransition MLNTransitionMake(NSTimeInterval duration, NSTimeInterval delay) {
  MLNTransition transition;
  transition.duration = duration;
  transition.delay = delay;

  return transition;
}

/**
 Constants indicating the visibility of different map ornaments.
 */
typedef NS_ENUM(NSInteger, MLNOrnamentVisibility) {
  /** A constant indicating that the ornament adapts to the current map state. */
  MLNOrnamentVisibilityAdaptive,
  /** A constant indicating that the ornament is always hidden. */
  MLNOrnamentVisibilityHidden,
  /** A constant indicating that the ornament is always visible. */
  MLNOrnamentVisibilityVisible
};

NS_ASSUME_NONNULL_END
