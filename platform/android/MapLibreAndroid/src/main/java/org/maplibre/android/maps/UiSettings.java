package org.maplibre.android.maps;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;

import androidx.annotation.ColorInt;
import androidx.annotation.FloatRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.Px;
import androidx.annotation.UiThread;
import androidx.annotation.VisibleForTesting;
import androidx.core.content.ContextCompat;
import androidx.core.content.res.ResourcesCompat;

import org.maplibre.android.R;
import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.constants.MapLibreConstants;
import org.maplibre.android.maps.widgets.CompassView;
import org.maplibre.android.utils.BitmapUtils;
import org.maplibre.android.utils.ColorUtils;

/**
 * Settings for the user interface of a MapLibreMap. To obtain this interface, call getUiSettings().
 */
public final class UiSettings {

  @NonNull
  private final FocalPointChangeListener focalPointChangeListener;
  @NonNull
  private final MapView mapView;
  @NonNull
  private final Projection projection;
  @VisibleForTesting
  @Nullable
  CompassView compassView;
  private final int[] compassMargins = new int[4];

  @VisibleForTesting
  @Nullable
  ImageView attributionsView;
  private final int[] attributionsMargins = new int[4];
  private AttributionDialogManager attributionDialogManager;

  @VisibleForTesting
  @Nullable
  ImageView logoView;
  private final int[] logoMargins = new int[4];

  private final float pixelRatio;

  private boolean rotateGesturesEnabled = true;

  private boolean tiltGesturesEnabled = true;

  private boolean zoomGesturesEnabled = true;

  private boolean scrollGesturesEnabled = true;

  private boolean horizontalScrollGesturesEnabled = true;

  private boolean doubleTapGesturesEnabled = true;

  private boolean quickZoomGesturesEnabled = true;

  private boolean scaleVelocityAnimationEnabled = true;
  private boolean rotateVelocityAnimationEnabled = true;
  private boolean flingVelocityAnimationEnabled = true;

  private boolean increaseRotateThresholdWhenScaling = true;
  private boolean disableRotateWhenScaling = true;
  private boolean increaseScaleThresholdWhenRotating = true;

  private float zoomRate = 1.0f;

  private boolean deselectMarkersOnTap = true;

  private long flingAnimationBaseTime = MapLibreConstants.ANIMATION_DURATION_FLING_BASE;
  private long flingThreshold = MapLibreConstants.VELOCITY_THRESHOLD_IGNORE_FLING;

  @Nullable
  private PointF userProvidedFocalPoint;

  @VisibleForTesting
  boolean isCompassInitialized = false;
  @VisibleForTesting
  boolean isAttributionInitialized = false;
  @VisibleForTesting
  boolean isLogoInitialized = false;
  private double clockwiseBearing;

  UiSettings(@NonNull Projection projection, @NonNull FocalPointChangeListener listener,
             float pixelRatio, MapView mapView) {
    this.projection = projection;
    this.focalPointChangeListener = listener;
    this.pixelRatio = pixelRatio;
    this.mapView = mapView;
  }

  void initialise(@NonNull Context context, @NonNull MapLibreMapOptions options) {
    Resources resources = context.getResources();
    initialiseGestures(options);
    if (options.getCompassEnabled()) {
      initialiseCompass(options, resources);
    }
    if (options.getLogoEnabled()) {
      initialiseLogo(options, resources);
    }
    if (options.getAttributionEnabled()) {
      initialiseAttribution(context, options);
    }
  }

  void onSaveInstanceState(@NonNull Bundle outState) {
    saveGestures(outState);
    saveCompass(outState);
    saveLogo(outState);
    saveAttribution(outState);
    saveDeselectMarkersOnTap(outState);
    saveFocalPoint(outState);
  }

  void onRestoreInstanceState(@NonNull Bundle savedInstanceState) {
    restoreGestures(savedInstanceState);
    restoreCompass(savedInstanceState);
    restoreLogo(savedInstanceState);
    restoreAttribution(savedInstanceState);
    restoreDeselectMarkersOnTap(savedInstanceState);
    restoreFocalPoint(savedInstanceState);
  }

  private void initialiseGestures(MapLibreMapOptions options) {
    setZoomGesturesEnabled(options.getZoomGesturesEnabled());
    setScrollGesturesEnabled(options.getScrollGesturesEnabled());
    setHorizontalScrollGesturesEnabled(options.getHorizontalScrollGesturesEnabled());
    setRotateGesturesEnabled(options.getRotateGesturesEnabled());
    setTiltGesturesEnabled(options.getTiltGesturesEnabled());
    setDoubleTapGesturesEnabled(options.getDoubleTapGesturesEnabled());
    setQuickZoomGesturesEnabled(options.getQuickZoomGesturesEnabled());
  }

  private void saveGestures(Bundle outState) {
    outState.putBoolean(MapLibreConstants.STATE_HORIZONAL_SCROLL_ENABLED, isHorizontalScrollGesturesEnabled());
    outState.putBoolean(MapLibreConstants.STATE_ZOOM_ENABLED, isZoomGesturesEnabled());
    outState.putBoolean(MapLibreConstants.STATE_SCROLL_ENABLED, isScrollGesturesEnabled());
    outState.putBoolean(MapLibreConstants.STATE_ROTATE_ENABLED, isRotateGesturesEnabled());
    outState.putBoolean(MapLibreConstants.STATE_TILT_ENABLED, isTiltGesturesEnabled());
    outState.putBoolean(MapLibreConstants.STATE_DOUBLE_TAP_ENABLED, isDoubleTapGesturesEnabled());
    outState.putBoolean(MapLibreConstants.STATE_SCALE_ANIMATION_ENABLED, isScaleVelocityAnimationEnabled());
    outState.putBoolean(MapLibreConstants.STATE_ROTATE_ANIMATION_ENABLED, isRotateVelocityAnimationEnabled());
    outState.putBoolean(MapLibreConstants.STATE_FLING_ANIMATION_ENABLED, isFlingVelocityAnimationEnabled());
    outState.putBoolean(MapLibreConstants.STATE_INCREASE_ROTATE_THRESHOLD, isIncreaseRotateThresholdWhenScaling());
    outState.putBoolean(MapLibreConstants.STATE_DISABLE_ROTATE_WHEN_SCALING, isDisableRotateWhenScaling());
    outState.putBoolean(MapLibreConstants.STATE_INCREASE_SCALE_THRESHOLD, isIncreaseScaleThresholdWhenRotating());
    outState.putBoolean(MapLibreConstants.STATE_QUICK_ZOOM_ENABLED, isQuickZoomGesturesEnabled());
    outState.putFloat(MapLibreConstants.STATE_ZOOM_RATE, getZoomRate());
  }

  private void restoreGestures(Bundle savedInstanceState) {
    setHorizontalScrollGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_HORIZONAL_SCROLL_ENABLED));
    setZoomGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_ZOOM_ENABLED));
    setScrollGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_SCROLL_ENABLED));
    setRotateGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_ROTATE_ENABLED));
    setTiltGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_TILT_ENABLED));
    setDoubleTapGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_DOUBLE_TAP_ENABLED));
    setScaleVelocityAnimationEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_SCALE_ANIMATION_ENABLED));
    setRotateVelocityAnimationEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_ROTATE_ANIMATION_ENABLED));
    setFlingVelocityAnimationEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_FLING_ANIMATION_ENABLED));
    setIncreaseRotateThresholdWhenScaling(
      savedInstanceState.getBoolean(MapLibreConstants.STATE_INCREASE_ROTATE_THRESHOLD));
    setDisableRotateWhenScaling(savedInstanceState.getBoolean(MapLibreConstants.STATE_DISABLE_ROTATE_WHEN_SCALING));
    setIncreaseScaleThresholdWhenRotating(
      savedInstanceState.getBoolean(MapLibreConstants.STATE_INCREASE_SCALE_THRESHOLD));
    setQuickZoomGesturesEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_QUICK_ZOOM_ENABLED));
    setZoomRate(savedInstanceState.getFloat(MapLibreConstants.STATE_ZOOM_RATE, 1.0f));
  }

  private void initialiseCompass(MapLibreMapOptions options, @NonNull Resources resources) {
    isCompassInitialized = true;
    compassView = mapView.initialiseCompassView();
    setCompassEnabled(options.getCompassEnabled());
    setCompassGravity(options.getCompassGravity());
    int[] compassMargins = options.getCompassMargins();
    if (compassMargins != null) {
      setCompassMargins(compassMargins[0], compassMargins[1], compassMargins[2], compassMargins[3]);
    } else {
      int tenDp = (int) resources.getDimension(R.dimen.maplibre_four_dp);
      setCompassMargins(tenDp, tenDp, tenDp, tenDp);
    }
    setCompassFadeFacingNorth(options.getCompassFadeFacingNorth());
    if (options.getCompassImage() == null) {
      options.compassImage(ResourcesCompat.getDrawable(resources, R.drawable.maplibre_compass_icon, null));
    }
    setCompassImage(options.getCompassImage());
  }

  private void saveCompass(Bundle outState) {
    outState.putBoolean(MapLibreConstants.STATE_COMPASS_ENABLED, isCompassEnabled());
    outState.putInt(MapLibreConstants.STATE_COMPASS_GRAVITY, getCompassGravity());
    outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_LEFT, getCompassMarginLeft());
    outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_TOP, getCompassMarginTop());
    outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_BOTTOM, getCompassMarginBottom());
    outState.putInt(MapLibreConstants.STATE_COMPASS_MARGIN_RIGHT, getCompassMarginRight());
    outState.putBoolean(MapLibreConstants.STATE_COMPASS_FADE_WHEN_FACING_NORTH, isCompassFadeWhenFacingNorth());
    outState.putByteArray(MapLibreConstants.STATE_COMPASS_IMAGE_BITMAP,
      BitmapUtils.getByteArrayFromDrawable(getCompassImage()));
  }

  private void restoreCompass(Bundle savedInstanceState) {
    boolean compassEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_COMPASS_ENABLED);
    if (compassEnabled && !isCompassInitialized) {
      compassView = mapView.initialiseCompassView();
      isCompassInitialized = true;
    }
    setCompassEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_COMPASS_ENABLED));
    setCompassGravity(savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_GRAVITY));
    setCompassMargins(savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_LEFT),
      savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_TOP),
      savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_RIGHT),
      savedInstanceState.getInt(MapLibreConstants.STATE_COMPASS_MARGIN_BOTTOM));
    setCompassFadeFacingNorth(savedInstanceState.getBoolean(MapLibreConstants.STATE_COMPASS_FADE_WHEN_FACING_NORTH));
    setCompassImage(BitmapUtils.getDrawableFromByteArray(
      mapView.getContext(), savedInstanceState.getByteArray(MapLibreConstants.STATE_COMPASS_IMAGE_BITMAP)));
  }

  private void initialiseLogo(MapLibreMapOptions options, @NonNull Resources resources) {
    isLogoInitialized = true;
    logoView = mapView.initialiseLogoView();
    setLogoEnabled(options.getLogoEnabled());
    setLogoGravity(options.getLogoGravity());
    setLogoMargins(resources, options.getLogoMargins());
  }

  private void setLogoMargins(@NonNull Resources resources, @Nullable int[] logoMargins) {
    if (logoMargins != null) {
      setLogoMargins(logoMargins[0], logoMargins[1], logoMargins[2], logoMargins[3]);
    } else {
      // user did not specify margins when programmatically creating a map
      int fourDp = (int) resources.getDimension(R.dimen.maplibre_four_dp);
      setLogoMargins(fourDp, fourDp, fourDp, fourDp);
    }
  }

  private void saveLogo(Bundle outState) {
    outState.putInt(MapLibreConstants.STATE_LOGO_GRAVITY, getLogoGravity());
    outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_LEFT, getLogoMarginLeft());
    outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_TOP, getLogoMarginTop());
    outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_RIGHT, getLogoMarginRight());
    outState.putInt(MapLibreConstants.STATE_LOGO_MARGIN_BOTTOM, getLogoMarginBottom());
    outState.putBoolean(MapLibreConstants.STATE_LOGO_ENABLED, isLogoEnabled());
  }

  private void restoreLogo(Bundle savedInstanceState) {
    boolean logoEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_LOGO_ENABLED);
    if (logoEnabled && !isLogoInitialized) {
      logoView = mapView.initialiseLogoView();
      isLogoInitialized = true;
    }
    setLogoEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_LOGO_ENABLED));
    setLogoGravity(savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_GRAVITY));
    setLogoMargins(savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_LEFT),
      savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_TOP),
      savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_RIGHT),
      savedInstanceState.getInt(MapLibreConstants.STATE_LOGO_MARGIN_BOTTOM));
  }

  private void initialiseAttribution(@NonNull Context context, MapLibreMapOptions options) {
    isAttributionInitialized = true;
    attributionsView = mapView.initialiseAttributionView();
    setAttributionEnabled(options.getAttributionEnabled());
    setAttributionGravity(options.getAttributionGravity());
    setAttributionMargins(context, options.getAttributionMargins());
    int attributionTintColor = options.getAttributionTintColor();
    setAttributionTintColor(attributionTintColor != -1
      ? attributionTintColor : ColorUtils.getPrimaryColor(context));
  }

  private void setAttributionMargins(@NonNull Context context, @Nullable int[] attributionMargins) {
    if (attributionMargins != null) {
      setAttributionMargins(attributionMargins[0], attributionMargins[1],
        attributionMargins[2], attributionMargins[3]);
    } else {
      // user did not specify margins when programmatically creating a map
      Resources resources = context.getResources();
      int margin = (int) resources.getDimension(R.dimen.maplibre_four_dp);
      int leftMargin = (int) resources.getDimension(R.dimen.maplibre_ninety_two_dp);
      setAttributionMargins(leftMargin, margin, margin, margin);
    }
  }

  private void saveAttribution(Bundle outState) {
    outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_GRAVITY, getAttributionGravity());
    outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_LEFT, getAttributionMarginLeft());
    outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_TOP, getAttributionMarginTop());
    outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_RIGHT, getAttributionMarginRight());
    outState.putInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_BOTTOM, getAttributionMarginBottom());
    outState.putBoolean(MapLibreConstants.STATE_ATTRIBUTION_ENABLED, isAttributionEnabled());
  }

  private void restoreAttribution(Bundle savedInstanceState) {
    boolean attributionEnabled = savedInstanceState.getBoolean(MapLibreConstants.STATE_ATTRIBUTION_ENABLED);
    if (attributionEnabled && !isAttributionInitialized) {
      attributionsView = mapView.initialiseAttributionView();
      isAttributionInitialized = true;
    }
    setAttributionEnabled(savedInstanceState.getBoolean(MapLibreConstants.STATE_ATTRIBUTION_ENABLED));
    setAttributionGravity(savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_GRAVITY));
    setAttributionMargins(savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_LEFT),
      savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_TOP),
      savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_RIGHT),
      savedInstanceState.getInt(MapLibreConstants.STATE_ATTRIBUTION_MARGIN_BOTTOM));
  }

  public long getFlingAnimationBaseTime() {
    return flingAnimationBaseTime;
  }

  public long getFlingThreshold() {
    return flingThreshold;
  }

  public void setFlingAnimationBaseTime(long t) {
    flingAnimationBaseTime = t;
  }

  public void setFlingThreshold(long t) {
    flingThreshold = t;
  }

  /**
   * <p>
   * Enables or disables the compass. The compass is an icon on the map that indicates the
   * direction of north on the map. When a user clicks
   * the compass, the camera orients itself to its default orientation and fades away shortly
   * after. If disabled, the compass will never be displayed.
   * </p>
   * By default, the compass is enabled.
   *
   * @param compassEnabled True to enable the compass; false to disable the compass.
   */
  public void setCompassEnabled(boolean compassEnabled) {
    if (compassEnabled && !isCompassInitialized) {
      initialiseCompass(mapView.maplibreMapOptions, mapView.getContext().getResources());
    }
    if (compassView != null) {
      compassView.setEnabled(compassEnabled);
      compassView.update(clockwiseBearing);
    }
  }

  /**
   * Returns whether the compass is enabled.
   *
   * @return True if the compass is enabled; false if the compass is disabled.
   */
  public boolean isCompassEnabled() {
    if (compassView != null) {
      return compassView.isEnabled();
    } else {
      return false;
    }
  }

  /**
   * <p>
   * Sets the gravity of the compass view. Use this to change the corner of the map view that the
   * compass is displayed in.
   * </p>
   * By default, the compass is in the top right corner.
   *
   * @param gravity Android SDK Gravity.
   */
  @UiThread
  public void setCompassGravity(int gravity) {
    if (compassView != null) {
      setWidgetGravity(compassView, gravity);
    }
  }

  /**
   * Enables or disables fading of the compass when facing north.
   * <p>
   * By default this feature is enabled
   * </p>
   *
   * @param compassFadeFacingNorth True to enable the fading animation; false to disable it
   */
  public void setCompassFadeFacingNorth(boolean compassFadeFacingNorth) {
    if (compassView != null) {
      compassView.fadeCompassViewFacingNorth(compassFadeFacingNorth);
    }
  }

  /**
   * Specifies the CompassView image.
   * <p>
   * By default this value is R.drawable.maplibre_compass_icon.
   * </p>
   *
   * @param compass the drawable to show as image compass
   */
  public void setCompassImage(@NonNull Drawable compass) {
    if (compassView != null) {
      compassView.setCompassImage(compass);
    }
  }

  /**
   * Returns whether the compass performs a fading animation out when facing north.
   *
   * @return True if the compass will fade, false if it remains visible
   */
  public boolean isCompassFadeWhenFacingNorth() {
    if (compassView != null) {
      return compassView.isFadeCompassViewFacingNorth();
    } else {
      return false;
    }
  }

  /**
   * Returns the gravity value of the CompassView
   *
   * @return The gravity
   */
  public int getCompassGravity() {
    if (compassView != null) {
      return ((FrameLayout.LayoutParams) compassView.getLayoutParams()).gravity;
    } else {
      return -1;
    }
  }

  /**
   * Sets the margins of the compass view in pixels. Use this to change the distance of the compass from the
   * map view edge.
   *
   * @param left   The left margin in pixels.
   * @param top    The top margin in pixels.
   * @param right  The right margin in pixels.
   * @param bottom The bottom margin in pixels.
   */
  @UiThread
  public void setCompassMargins(@Px int left, @Px int top, @Px int right, @Px int bottom) {
    if (compassView != null) {
      setWidgetMargins(compassView, compassMargins, left, top, right, bottom);
    }
  }

  /**
   * Returns the left side margin of CompassView in pixels.
   *
   * @return The left margin in pixels
   */
  @Px
  public int getCompassMarginLeft() {
    return compassMargins[0];
  }

  /**
   * Returns the top side margin of CompassView in pixels.
   *
   * @return The top margin in pixels
   */
  @Px
  public int getCompassMarginTop() {
    return compassMargins[1];
  }

  /**
   * Returns the right side margin of CompassView in pixels.
   *
   * @return The right margin in pixels
   */
  @Px
  public int getCompassMarginRight() {
    return compassMargins[2];
  }

  /**
   * Returns the bottom side margin of CompassView in pixels.
   *
   * @return The bottom margin in pixels
   */
  @Px
  public int getCompassMarginBottom() {
    return compassMargins[3];
  }

  /**
   * Get the current configured CompassView image.
   *
   * @return the drawable used as compass image
   */
  @Nullable
  public Drawable getCompassImage() {
    if (compassView != null) {
      return compassView.getCompassImage();
    } else {
      return null;
    }
  }

  void update(@NonNull CameraPosition cameraPosition) {
    clockwiseBearing = -cameraPosition.bearing;
    if (compassView != null) {
      compassView.update(clockwiseBearing);
    }
  }

  /**
   * <p>
   * Enables or disables the MapLibre logo.
   * </p>
   * By default, the logo is enabled.
   *
   * @param enabled True to enable the logo; false to disable the logo.
   */
  public void setLogoEnabled(boolean enabled) {
    if (enabled && !isLogoInitialized) {
      initialiseLogo(mapView.maplibreMapOptions, mapView.getContext().getResources());
    }
    if (logoView != null) {
      logoView.setVisibility(enabled ? View.VISIBLE : View.GONE);
    }
  }

  /**
   * Returns whether the logo is enabled.
   *
   * @return True if the logo is enabled; false if the logo is disabled.
   */
  public boolean isLogoEnabled() {
    if (logoView != null) {
      return logoView.getVisibility() == View.VISIBLE;
    } else {
      return false;
    }
  }

  /**
   * <p>
   * Sets the gravity of the logo view. Use this to change the corner of the map view that the
   * MapLibre logo is displayed in.
   * </p>
   * By default, the logo is in the bottom left corner.
   *
   * @param gravity Android SDK Gravity.
   */
  public void setLogoGravity(int gravity) {
    if (logoView != null) {
      setWidgetGravity(logoView, gravity);
    }
  }

  /**
   * Returns the gravity value of the logo
   *
   * @return The gravity
   */
  public int getLogoGravity() {
    if (logoView != null) {
      return ((FrameLayout.LayoutParams) logoView.getLayoutParams()).gravity;
    } else {
      return -1;
    }
  }

  /**
   * Sets the margins of the logo view in pixels. Use this to change the distance of the MapLibre logo from the
   * map view edge.
   *
   * @param left   The left margin in pixels.
   * @param top    The top margin in pixels.
   * @param right  The right margin in pixels.
   * @param bottom The bottom margin in pixels.
   */
  public void setLogoMargins(@Px int left, @Px int top, @Px int right, @Px int bottom) {
    if (logoView != null) {
      setWidgetMargins(logoView, logoMargins, left, top, right, bottom);
    }
  }

  /**
   * Returns the left side margin of the logo in pixels.
   *
   * @return The left margin in pixels
   */
  @Px
  public int getLogoMarginLeft() {
    return logoMargins[0];
  }

  /**
   * Returns the top side margin of the logo in pixels.
   *
   * @return The top margin in pixels
   */
  @Px
  public int getLogoMarginTop() {
    return logoMargins[1];
  }

  /**
   * Returns the right side margin of the logo in pixels.
   *
   * @return The right margin in pixels
   */
  @Px
  public int getLogoMarginRight() {
    return logoMargins[2];
  }

  /**
   * Returns the bottom side margin of the logo in pixels.
   *
   * @return The bottom margin in pixels
   */
  @Px
  public int getLogoMarginBottom() {
    return logoMargins[3];
  }

  /**
   * <p>
   * Enables or disables the attribution.
   * </p>
   * By default, the attribution is enabled.
   *
   * @param enabled True to enable the attribution; false to disable the attribution.
   */
  public void setAttributionEnabled(boolean enabled) {
    if (enabled && !isAttributionInitialized) {
      initialiseAttribution(mapView.getContext(), mapView.maplibreMapOptions);
    }
    if (attributionsView != null) {
      attributionsView.setVisibility(enabled ? View.VISIBLE : View.GONE);
    }
  }

  /**
   * Returns whether the attribution is enabled.
   *
   * @return True if the attribution is enabled; false if the attribution is disabled.
   */
  public boolean isAttributionEnabled() {
    if (attributionsView != null) {
      return attributionsView.getVisibility() == View.VISIBLE;
    } else {
      return false;
    }
  }


  /**
   * Set a custom attribution dialog manager.
   * <p>
   * Set to null to reset to default behaviour.
   * </p>
   *
   * @param attributionDialogManager the manager class used for showing attribution
   */
  public void setAttributionDialogManager(@NonNull AttributionDialogManager attributionDialogManager) {
    this.attributionDialogManager = attributionDialogManager;
  }

  /**
   * Get the custom attribution dialog manager.
   *
   * @return the active manager class used for showing attribution
   */
  @Nullable
  public AttributionDialogManager getAttributionDialogManager() {
    return attributionDialogManager;
  }

  /**
   * <p>
   * Sets the gravity of the attribution.
   * </p>
   * By default, the attribution is in the bottom left corner next to the MapLibre logo.
   *
   * @param gravity Android SDK Gravity.
   */
  public void setAttributionGravity(int gravity) {
    if (attributionsView != null) {
      setWidgetGravity(attributionsView, gravity);
    }
  }

  /**
   * Returns the gravity value of the logo
   *
   * @return The gravity
   */
  public int getAttributionGravity() {
    if (attributionsView != null) {
      return ((FrameLayout.LayoutParams) attributionsView.getLayoutParams()).gravity;
    } else {
      return -1;
    }
  }

  /**
   * Sets the margins of the attribution view in pixels.
   *
   * @param left   The left margin in pixels.
   * @param top    The top margin in pixels.
   * @param right  The right margin in pixels.
   * @param bottom The bottom margin in pixels.
   */
  public void setAttributionMargins(@Px int left, @Px int top, @Px int right, @Px int bottom) {
    if (attributionsView != null) {
      setWidgetMargins(attributionsView, attributionsMargins, left, top, right, bottom);
    }
  }

  /**
   * <p>
   * Sets the tint of the attribution view. Use this to change the color of the attribution.
   * </p>
   *
   * @param tintColor Color to tint the attribution.
   */
  public void setAttributionTintColor(@ColorInt int tintColor) {
    // Check that the tint color being passed in isn't transparent.
    if (attributionsView == null) {
      return;
    }
    if (Color.alpha(tintColor) == 0) {
      ColorUtils.setTintList(attributionsView,
        ContextCompat.getColor(attributionsView.getContext(), R.color.maplibre_blue));
    } else {
      ColorUtils.setTintList(attributionsView, tintColor);
    }
  }

  /**
   * Returns the left side margin of the attribution view in pixels.
   *
   * @return The left margin in pixels
   */
  @Px
  public int getAttributionMarginLeft() {
    return attributionsMargins[0];
  }

  /**
   * Returns the top side margin of the attribution view in pixels.
   *
   * @return The top margin in pixels
   */
  @Px
  public int getAttributionMarginTop() {
    return attributionsMargins[1];
  }

  /**
   * Returns the right side margin of the attribution view in pixels.
   *
   * @return The right margin in pixels
   */
  @Px
  public int getAttributionMarginRight() {
    return attributionsMargins[2];
  }

  /**
   * Returns the bottom side margin of the logo in pixels.
   *
   * @return The bottom margin in pixels
   */
  @Px
  public int getAttributionMarginBottom() {
    return attributionsMargins[3];
  }

  /**
   * <p>
   * Changes whether the user may rotate the map.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param rotateGesturesEnabled If true, rotating is enabled.
   */
  public void setRotateGesturesEnabled(boolean rotateGesturesEnabled) {
    this.rotateGesturesEnabled = rotateGesturesEnabled;
  }

  /**
   * Returns whether the user may rotate the map.
   *
   * @return If true, rotating is enabled.
   */
  public boolean isRotateGesturesEnabled() {
    return rotateGesturesEnabled;
  }

  /**
   * <p>
   * Changes whether the user may tilt the map.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param tiltGesturesEnabled If true, tilting is enabled.
   */
  public void setTiltGesturesEnabled(boolean tiltGesturesEnabled) {
    this.tiltGesturesEnabled = tiltGesturesEnabled;

  }

  /**
   * Returns whether the user may tilt the map.
   *
   * @return If true, tilting is enabled.
   */
  public boolean isTiltGesturesEnabled() {
    return tiltGesturesEnabled;
  }

  /**
   * <p>
   * Changes whether the user may zoom the map.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param zoomGesturesEnabled If true, zooming is enabled.
   */
  public void setZoomGesturesEnabled(boolean zoomGesturesEnabled) {
    this.zoomGesturesEnabled = zoomGesturesEnabled;
  }

  /**
   * Returns whether the user may zoom the map.
   *
   * @return If true, zooming is enabled.
   */
  public boolean isZoomGesturesEnabled() {
    return zoomGesturesEnabled;
  }

  /**
   * <p>
   * Changes whether the user may zoom the map with a double tap.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param doubleTapGesturesEnabled If true, zooming with a double tap is enabled.
   */
  public void setDoubleTapGesturesEnabled(boolean doubleTapGesturesEnabled) {
    this.doubleTapGesturesEnabled = doubleTapGesturesEnabled;
  }

  /**
   * Returns whether the user may zoom the map with a double tap.
   *
   * @return If true, zooming with a double tap is enabled.
   */
  public boolean isDoubleTapGesturesEnabled() {
    return doubleTapGesturesEnabled;
  }

  /**
   * Returns whether the user may zoom the map by tapping twice, holding and moving the pointer up and down.
   *
   * @return If true, zooming by tapping twice and holding is enabled.
   */
  public boolean isQuickZoomGesturesEnabled() {
    return quickZoomGesturesEnabled;
  }

  /**
   * Changes whether the user may zoom the map by tapping twice, holding and moving the pointer up and down.
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param quickZoomGesturesEnabled If true, zooming by tapping twice and holding is enabled.
   */
  public void setQuickZoomGesturesEnabled(boolean quickZoomGesturesEnabled) {
    this.quickZoomGesturesEnabled = quickZoomGesturesEnabled;
  }

  /**
   * Returns zoom gesture rate, including pinch to zoom and quick scale.
   *
   * @return The zoom rate.
   */
  public float getZoomRate() {
    return zoomRate;
  }

  /**
   * Sets zoom gesture rate, including pinch to zoom and quick scale.
   * <p>
   * Default value is 1.0f.
   * </p>
   *
   * @param zoomRate The zoom rate.
   */
  public void setZoomRate(@FloatRange(from = 0f) float zoomRate) {
    this.zoomRate = zoomRate;
  }

  private void restoreDeselectMarkersOnTap(Bundle savedInstanceState) {
    setDeselectMarkersOnTap(savedInstanceState.getBoolean(MapLibreConstants.STATE_DESELECT_MARKER_ON_TAP));
  }

  private void saveDeselectMarkersOnTap(Bundle outState) {
    outState.putBoolean(MapLibreConstants.STATE_DESELECT_MARKER_ON_TAP, isDeselectMarkersOnTap());
  }

  /**
   * Gets whether the markers are automatically deselected (and therefore, their infowindows
   * closed) when a map tap is detected.
   *
   * @return If true, markers are deselected on a map tap.
   */
  public boolean isDeselectMarkersOnTap() {
    return deselectMarkersOnTap;
  }

  /**
   * Sets whether the markers are automatically deselected (and therefore, their infowindows
   * closed) when a map tap is detected.
   *
   * @param deselectMarkersOnTap determines if markers should be deslected on tap
   */
  public void setDeselectMarkersOnTap(boolean deselectMarkersOnTap) {
    this.deselectMarkersOnTap = deselectMarkersOnTap;
  }

  /**
   * <p>
   * Changes whether the user may scroll around the map.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param scrollGesturesEnabled If true, scrolling is enabled.
   */
  public void setScrollGesturesEnabled(boolean scrollGesturesEnabled) {
    this.scrollGesturesEnabled = scrollGesturesEnabled;
  }

  /**
   * Returns whether the user may scroll around the map.
   *
   * @return If true, scrolling is enabled.
   */
  public boolean isScrollGesturesEnabled() {
    return scrollGesturesEnabled;
  }

  /**
   * <p>
   * Changes whether the user may scroll horizontally around the map.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param horizontalScrollGesturesEnabled If true, scrolling horizontally is enabled.
   */
  public void setHorizontalScrollGesturesEnabled(boolean horizontalScrollGesturesEnabled) {
    this.horizontalScrollGesturesEnabled = horizontalScrollGesturesEnabled;
  }

  /**
   * Returns whether the user may scroll horizontally around the map.
   *
   * @return If true, scrolling horizontally is enabled.
   */
  public boolean isHorizontalScrollGesturesEnabled() {
    return horizontalScrollGesturesEnabled;
  }

  /**
   * Returns whether scale velocity animation should execute after users finishes a gesture.
   *
   * @return If true, scale velocity animation is enabled.
   */
  public boolean isScaleVelocityAnimationEnabled() {
    return scaleVelocityAnimationEnabled;
  }

  /**
   * Set whether scale velocity animation should execute after users finishes a gesture. True by default.
   *
   * @param scaleVelocityAnimationEnabled If true, scale velocity animation will be enabled.
   */
  public void setScaleVelocityAnimationEnabled(boolean scaleVelocityAnimationEnabled) {
    this.scaleVelocityAnimationEnabled = scaleVelocityAnimationEnabled;
  }

  /**
   * Returns whether rotate velocity animation should execute after users finishes a gesture.
   *
   * @return If true, rotate velocity animation is enabled.
   */
  public boolean isRotateVelocityAnimationEnabled() {
    return rotateVelocityAnimationEnabled;
  }

  /**
   * Set whether rotate velocity animation should execute after users finishes a gesture. True by default.
   *
   * @param rotateVelocityAnimationEnabled If true, rotate velocity animation will be enabled.
   */
  public void setRotateVelocityAnimationEnabled(boolean rotateVelocityAnimationEnabled) {
    this.rotateVelocityAnimationEnabled = rotateVelocityAnimationEnabled;
  }

  /**
   * Returns whether fling velocity animation should execute after users finishes a gesture.
   *
   * @return If true, fling velocity animation is enabled.
   */
  public boolean isFlingVelocityAnimationEnabled() {
    return flingVelocityAnimationEnabled;
  }

  /**
   * Set whether fling velocity animation should execute after users finishes a gesture. True by default.
   *
   * @param flingVelocityAnimationEnabled If true, fling velocity animation will be enabled.
   */
  public void setFlingVelocityAnimationEnabled(boolean flingVelocityAnimationEnabled) {
    this.flingVelocityAnimationEnabled = flingVelocityAnimationEnabled;
  }

  /**
   * Set whether all velocity animations should execute after users finishes a gesture.
   *
   * @param allVelocityAnimationsEnabled If true, all velocity animations will be enabled.
   */
  public void setAllVelocityAnimationsEnabled(boolean allVelocityAnimationsEnabled) {
    setScaleVelocityAnimationEnabled(allVelocityAnimationsEnabled);
    setRotateVelocityAnimationEnabled(allVelocityAnimationsEnabled);
    setFlingVelocityAnimationEnabled(allVelocityAnimationsEnabled);
  }

  /**
   * Returns whether rotation threshold should be increase whenever scale is detected.
   *
   * @return If true, rotation threshold will be increased.
   * @deprecated unused, see {@link #isDisableRotateWhenScaling()} instead
   */
  @Deprecated
  public boolean isIncreaseRotateThresholdWhenScaling() {
    return increaseRotateThresholdWhenScaling;
  }

  /**
   * Set whether rotation threshold should be increase whenever scale is detected.
   *
   * @param increaseRotateThresholdWhenScaling If true, rotation threshold will be increased.
   * @deprecated unused, see {@link #setDisableRotateWhenScaling(boolean)} instead
   */
  @Deprecated
  public void setIncreaseRotateThresholdWhenScaling(boolean increaseRotateThresholdWhenScaling) {
    this.increaseRotateThresholdWhenScaling = increaseRotateThresholdWhenScaling;
  }

  /**
   * Returns whether rotation gesture detector is disabled when scale is detected first.
   *
   * @return If true, rotation gesture detector will be disabled when scale is detected first.
   */
  public boolean isDisableRotateWhenScaling() {
    return disableRotateWhenScaling;
  }

  /**
   * Set whether rotation gesture detector should be disabled when scale is detected first.
   *
   * @param disableRotateWhenScaling If true, rotation gesture detector will be disabled when scale is detected first.
   */
  public void setDisableRotateWhenScaling(boolean disableRotateWhenScaling) {
    this.disableRotateWhenScaling = disableRotateWhenScaling;
  }

  /**
   * Returns whether scale threshold should be increase whenever rotation is detected.
   *
   * @return If true, scale threshold will be increased.
   */
  public boolean isIncreaseScaleThresholdWhenRotating() {
    return increaseScaleThresholdWhenRotating;
  }

  /**
   * set whether scale threshold should be increase whenever rotation is detected.
   *
   * @param increaseScaleThresholdWhenRotating If true, scale threshold will be increased.
   */
  public void setIncreaseScaleThresholdWhenRotating(boolean increaseScaleThresholdWhenRotating) {
    this.increaseScaleThresholdWhenRotating = increaseScaleThresholdWhenRotating;
  }

  /**
   * <p>
   * Sets the preference for whether all gestures should be enabled or disabled.
   * </p>
   * <p>
   * This setting controls only user interactions with the map. If you set the value to false,
   * you may still change the map location programmatically.
   * </p>
   * The default value is true.
   *
   * @param enabled If true, all gestures are available; otherwise, all gestures are disabled.
   * @see #setZoomGesturesEnabled(boolean) )
   * @see #setScrollGesturesEnabled(boolean)
   * @see #setRotateGesturesEnabled(boolean)
   * @see #setTiltGesturesEnabled(boolean)
   * @see #setDoubleTapGesturesEnabled(boolean)
   * @see #setQuickZoomGesturesEnabled(boolean)
   */
  public void setAllGesturesEnabled(boolean enabled) {
    setScrollGesturesEnabled(enabled);
    setRotateGesturesEnabled(enabled);
    setTiltGesturesEnabled(enabled);
    setZoomGesturesEnabled(enabled);
    setDoubleTapGesturesEnabled(enabled);
    setQuickZoomGesturesEnabled(enabled);
  }

  /**
   * <p>
   * Retrieves the current status of whether all gestures are enabled.
   * </p>
   *
   * @return If true, all gestures are enabled.
   */
  public boolean areAllGesturesEnabled() {
    return rotateGesturesEnabled && tiltGesturesEnabled && zoomGesturesEnabled
      && scrollGesturesEnabled && doubleTapGesturesEnabled && quickZoomGesturesEnabled;
  }

  private void saveFocalPoint(Bundle outState) {
    outState.putParcelable(MapLibreConstants.STATE_USER_FOCAL_POINT, getFocalPoint());
  }

  private void restoreFocalPoint(Bundle savedInstanceState) {
    PointF pointF = savedInstanceState.getParcelable(MapLibreConstants.STATE_USER_FOCAL_POINT);
    if (pointF != null) {
      setFocalPoint(pointF);
    }
  }

  /**
   * Sets the focal point used as center for a gesture
   *
   * @param focalPoint the focal point to be used.
   */
  public void setFocalPoint(@Nullable PointF focalPoint) {
    this.userProvidedFocalPoint = focalPoint;
    focalPointChangeListener.onFocalPointChanged(focalPoint);
  }

  /**
   * Returns the gesture focal point
   *
   * @return The focal point
   */
  @Nullable
  public PointF getFocalPoint() {
    return userProvidedFocalPoint;
  }

  /**
   * Returns the measured height of the MapView
   *
   * @return height in pixels
   */
  public float getHeight() {
    return projection.getHeight();
  }

  /**
   * Returns the measured width of the MapView
   *
   * @return widht in pixels
   */
  public float getWidth() {
    return projection.getWidth();
  }

  float getPixelRatio() {
    return pixelRatio;
  }

  /**
   * Invalidates the ViewSettings instances shown on top of the MapView
   */
  public void invalidate() {
    setLogoMargins(getLogoMarginLeft(), getLogoMarginTop(), getLogoMarginRight(), getLogoMarginBottom());
    setCompassEnabled(isCompassEnabled());
    setCompassMargins(getCompassMarginLeft(), getCompassMarginTop(), getCompassMarginRight(), getCompassMarginBottom());
    setAttributionMargins(getAttributionMarginLeft(), getAttributionMarginTop(), getAttributionMarginRight(),
      getAttributionMarginBottom());
  }

  private void setWidgetGravity(@NonNull final View view, int gravity) {
    FrameLayout.LayoutParams layoutParams = (FrameLayout.LayoutParams) view.getLayoutParams();
    layoutParams.gravity = gravity;
    view.setLayoutParams(layoutParams);
  }

  private void setWidgetMargins(@NonNull final View view, int[] initMargins, int left, int top, int right, int bottom) {
    // keep state of initially set margins
    initMargins[0] = left;
    initMargins[1] = top;
    initMargins[2] = right;
    initMargins[3] = bottom;

    // convert initial margins with padding
    FrameLayout.LayoutParams layoutParams = (FrameLayout.LayoutParams) view.getLayoutParams();
    layoutParams.setMargins(left, top, right, bottom);

    // support RTL
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      layoutParams.setMarginStart(left);
      layoutParams.setMarginEnd(right);
    }

    view.setLayoutParams(layoutParams);
  }
}
