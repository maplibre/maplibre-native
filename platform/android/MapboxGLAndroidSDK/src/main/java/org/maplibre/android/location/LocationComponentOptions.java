package org.maplibre.android.location;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.RectF;
import android.os.Parcel;
import android.os.Parcelable;
import android.view.animation.Interpolator;

import androidx.annotation.ColorInt;
import androidx.annotation.Dimension;
import androidx.annotation.DrawableRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.StyleRes;

import com.mapbox.android.gestures.AndroidGesturesManager;

import org.maplibre.android.R;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.style.layers.Layer;

import java.util.Arrays;

/**
 * This class exposes options for the Location Component. The options can be set by defining a
 * style in your apps style.xml file and passing in directly into the {@link LocationComponent}
 * class. Alternatively, if properties need to be changed at runtime depending on a specific state,
 * you can build an instance of this class, setting the values you desire, and then passing it into
 * either the {@link LocationComponent} activation method (if it isn't initialized yet) or
 * {@link LocationComponent#applyStyle(LocationComponentOptions)}.
 * <p>
 * When the {@link #createFromAttributes(Context, int)} methods called, any attributes not found
 * inside the style will revert back to using their default set values. Likewise, when building a
 * new {@link LocationComponentOptions} class using the builder, any options neglecting to be set will
 * reset to their default values.
 * <p>
 * If you would like to keep your custom style changes while modifying a single attribute, you can
 * get the currently used options object using {@link LocationComponent#getLocationComponentOptions()}
 * and it's {@code toBuilder} method to modify a single entry while also maintaining the other
 * settings. Once your modifications have been made, you'll need to pass it back into the location
 * component using {@link LocationComponent#applyStyle(LocationComponentOptions)}.
 */
public class LocationComponentOptions implements Parcelable {

  /**
   * Default accuracy alpha
   */
  private static final float ACCURACY_ALPHA_DEFAULT = 0.15f;

  /**
   * Default icon scale factor when the map is zoomed out
   */
  private static final float MIN_ZOOM_ICON_SCALE_DEFAULT = 0.6f;

  /**
   * Default icon scale factor when the map is zoomed in
   */
  private static final float MAX_ZOOM_ICON_SCALE_DEFAULT = 1f;

  /**
   * Default map padding
   */
  private static final int[] PADDING_DEFAULT = {0, 0, 0, 0};

  /**
   * The default value which is used when the stale state is enabled
   */
  private static final long STALE_STATE_DELAY_MS = 30_000L;

  /**
   * Default animation duration multiplier
   */
  private static final float TRACKING_ANIMATION_DURATION_MULTIPLIER_DEFAULT = 1.1f;

  /**
   * Default duration of a single LocationComponent circle pulse.
   */
  private static final long CIRCLE_PULSING_DURATION_DEFAULT_MS = 2300;

  /**
   * Default opacity of the LocationComponent circle when it ends a single pulse.
   */
  private static final float CIRCLE_PULSING_ALPHA_DEFAULT = 1f;

  /**
   * Default maximum radius of the LocationComponent circle when it's pulsing.
   */
  public static final float CIRCLE_PULSING_MAX_RADIUS_DEFAULT = 35f;

  private float accuracyAlpha;
  private int accuracyColor;
  private int backgroundDrawableStale;
  @Nullable
  private String backgroundStaleName;
  private int foregroundDrawableStale;
  @Nullable
  private String foregroundStaleName;
  private int gpsDrawable;
  @Nullable
  private String gpsName;
  private int foregroundDrawable;
  @Nullable
  private String foregroundName;
  private int backgroundDrawable;
  @Nullable
  private String backgroundName;
  private int bearingDrawable;
  @Nullable
  private String bearingName;
  @Nullable
  private Integer bearingTintColor;
  @Nullable
  private Integer foregroundTintColor;
  @Nullable
  private Integer backgroundTintColor;
  @Nullable
  private Integer foregroundStaleTintColor;
  @Nullable
  private Integer backgroundStaleTintColor;
  private float elevation;
  private boolean enableStaleState;
  private long staleStateTimeout;
  @Nullable
  private int[] padding;
  private float maxZoomIconScale;
  private float minZoomIconScale;
  private boolean trackingGesturesManagement;
  private float trackingInitialMoveThreshold;
  private float trackingMultiFingerMoveThreshold;
  @Nullable
  private RectF trackingMultiFingerProtectedMoveArea;
  private String layerAbove;
  private String layerBelow;
  private float trackingAnimationDurationMultiplier;
  private boolean compassAnimationEnabled;
  private boolean accuracyAnimationEnabled;
  private Boolean pulseEnabled;
  private Boolean pulseFadeEnabled;
  private Integer pulseColor;
  private float pulseSingleDuration;
  private float pulseMaxRadius;
  private float pulseAlpha;
  @Nullable
  private Interpolator pulseInterpolator;

  public LocationComponentOptions(
    float accuracyAlpha,
    int accuracyColor,
    int backgroundDrawableStale,
    @Nullable String backgroundStaleName,
    int foregroundDrawableStale,
    @Nullable String foregroundStaleName,
    int gpsDrawable,
    @Nullable String gpsName,
    int foregroundDrawable,
    @Nullable String foregroundName,
    int backgroundDrawable,
    @Nullable String backgroundName,
    int bearingDrawable,
    @Nullable String bearingName,
    @Nullable Integer bearingTintColor,
    @Nullable Integer foregroundTintColor,
    @Nullable Integer backgroundTintColor,
    @Nullable Integer foregroundStaleTintColor,
    @Nullable Integer backgroundStaleTintColor,
    float elevation,
    boolean enableStaleState,
    long staleStateTimeout,
    @Nullable int[] padding,
    float maxZoomIconScale,
    float minZoomIconScale,
    boolean trackingGesturesManagement,
    float trackingInitialMoveThreshold,
    float trackingMultiFingerMoveThreshold,
    RectF trackingMultiFingerProtectedMoveArea,
    String layerAbove,
    String layerBelow,
    float trackingAnimationDurationMultiplier,
    boolean compassAnimationEnabled,
    boolean accuracyAnimationEnabled,
    Boolean pulseEnabled,
    Boolean pulseFadeEnabled,
    Integer pulseColor,
    float pulseSingleDuration,
    float pulseMaxRadius,
    float pulseAlpha,
    @Nullable Interpolator pulseInterpolator) {
    this.accuracyAlpha = accuracyAlpha;
    this.accuracyColor = accuracyColor;
    this.backgroundDrawableStale = backgroundDrawableStale;
    this.backgroundStaleName = backgroundStaleName;
    this.foregroundDrawableStale = foregroundDrawableStale;
    this.foregroundStaleName = foregroundStaleName;
    this.gpsDrawable = gpsDrawable;
    this.gpsName = gpsName;
    this.foregroundDrawable = foregroundDrawable;
    this.foregroundName = foregroundName;
    this.backgroundDrawable = backgroundDrawable;
    this.backgroundName = backgroundName;
    this.bearingDrawable = bearingDrawable;
    this.bearingName = bearingName;
    this.bearingTintColor = bearingTintColor;
    this.foregroundTintColor = foregroundTintColor;
    this.backgroundTintColor = backgroundTintColor;
    this.foregroundStaleTintColor = foregroundStaleTintColor;
    this.backgroundStaleTintColor = backgroundStaleTintColor;
    this.elevation = elevation;
    this.enableStaleState = enableStaleState;
    this.staleStateTimeout = staleStateTimeout;
    if (padding == null) {
      throw new NullPointerException("Null padding");
    }
    this.padding = padding;
    this.maxZoomIconScale = maxZoomIconScale;
    this.minZoomIconScale = minZoomIconScale;
    this.trackingGesturesManagement = trackingGesturesManagement;
    this.trackingInitialMoveThreshold = trackingInitialMoveThreshold;
    this.trackingMultiFingerMoveThreshold = trackingMultiFingerMoveThreshold;
    this.trackingMultiFingerProtectedMoveArea = trackingMultiFingerProtectedMoveArea;
    this.layerAbove = layerAbove;
    this.layerBelow = layerBelow;
    this.trackingAnimationDurationMultiplier = trackingAnimationDurationMultiplier;
    this.compassAnimationEnabled = compassAnimationEnabled;
    this.accuracyAnimationEnabled = accuracyAnimationEnabled;
    this.pulseEnabled = pulseEnabled;
    this.pulseFadeEnabled = pulseFadeEnabled;
    this.pulseColor = pulseColor;
    this.pulseSingleDuration = pulseSingleDuration;
    this.pulseMaxRadius = pulseMaxRadius;
    this.pulseAlpha = pulseAlpha;
    this.pulseInterpolator = pulseInterpolator;
  }

  /**
   * Construct a new Location Component Options class using the attributes found within a style
   * resource. It's important to note that you only need to define the attributes you plan to
   * change and can safely ignore the other attributes which will be set to their default value.
   *
   * @param context  your activity's context used for acquiring resources
   * @param styleRes the style id where your custom attributes are defined
   * @return a new {@link LocationComponentOptions} object with the settings you defined in your style
   * resource
   */
  @NonNull
  public static LocationComponentOptions createFromAttributes(@NonNull Context context,
                                                              @StyleRes int styleRes) {

    TypedArray typedArray = context.obtainStyledAttributes(
      styleRes, R.styleable.maplibre_LocationComponent);

    LocationComponentOptions.Builder builder = new LocationComponentOptions.Builder()
      .enableStaleState(true)
      .staleStateTimeout(STALE_STATE_DELAY_MS)
      .maxZoomIconScale(MAX_ZOOM_ICON_SCALE_DEFAULT)
      .minZoomIconScale(MIN_ZOOM_ICON_SCALE_DEFAULT)
      .padding(PADDING_DEFAULT);

    builder.foregroundDrawable(typedArray.getResourceId(
      R.styleable.maplibre_LocationComponent_maplibre_foregroundDrawable, -1));
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_foregroundTintColor)) {
      builder.foregroundTintColor(typedArray.getColor(
        R.styleable.maplibre_LocationComponent_maplibre_foregroundTintColor, -1));
    }
    builder.backgroundDrawable(typedArray.getResourceId(
      R.styleable.maplibre_LocationComponent_maplibre_backgroundDrawable, -1));
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_backgroundTintColor)) {
      builder.backgroundTintColor(typedArray.getColor(
        R.styleable.maplibre_LocationComponent_maplibre_backgroundTintColor, -1));
    }
    builder.foregroundDrawableStale(typedArray.getResourceId(
      R.styleable.maplibre_LocationComponent_maplibre_foregroundDrawableStale, -1));
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_foregroundStaleTintColor)) {
      builder.foregroundStaleTintColor(typedArray.getColor(
        R.styleable.maplibre_LocationComponent_maplibre_foregroundStaleTintColor, -1));
    }
    builder.backgroundDrawableStale(typedArray.getResourceId(
      R.styleable.maplibre_LocationComponent_maplibre_backgroundDrawableStale, -1));
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_backgroundStaleTintColor)) {
      builder.backgroundStaleTintColor(typedArray.getColor(
        R.styleable.maplibre_LocationComponent_maplibre_backgroundStaleTintColor, -1));
    }
    builder.bearingDrawable(typedArray.getResourceId(
      R.styleable.maplibre_LocationComponent_maplibre_bearingDrawable, -1));
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_bearingTintColor)) {
      builder.bearingTintColor(typedArray.getColor(
        R.styleable.maplibre_LocationComponent_maplibre_bearingTintColor, -1));
    }
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_enableStaleState)) {
      builder.enableStaleState(typedArray.getBoolean(
        R.styleable.maplibre_LocationComponent_maplibre_enableStaleState, true));
    }
    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_staleStateTimeout)) {
      builder.staleStateTimeout(typedArray.getInteger(
        R.styleable.maplibre_LocationComponent_maplibre_staleStateTimeout, (int) STALE_STATE_DELAY_MS));
    }
    builder.gpsDrawable(typedArray.getResourceId(
      R.styleable.maplibre_LocationComponent_maplibre_gpsDrawable, -1));
    float elevation = typedArray.getDimension(
      R.styleable.maplibre_LocationComponent_maplibre_elevation, 0);
    builder.accuracyColor(typedArray.getColor(
      R.styleable.maplibre_LocationComponent_maplibre_accuracyColor, -1));
    builder.accuracyAlpha(typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_accuracyAlpha, ACCURACY_ALPHA_DEFAULT));
    builder.elevation(elevation);

    builder.trackingGesturesManagement(typedArray.getBoolean(
      R.styleable.maplibre_LocationComponent_maplibre_trackingGesturesManagement, false));
    builder.trackingInitialMoveThreshold(typedArray.getDimension(
      R.styleable.maplibre_LocationComponent_maplibre_trackingInitialMoveThreshold,
      context.getResources().getDimension(R.dimen.maplibre_locationComponentTrackingInitialMoveThreshold)));
    builder.trackingMultiFingerMoveThreshold(typedArray.getDimension(
      R.styleable.maplibre_LocationComponent_maplibre_trackingMultiFingerMoveThreshold,
      context.getResources().getDimension(R.dimen.maplibre_locationComponentTrackingMultiFingerMoveThreshold)));

    builder.padding(new int[] {
      typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingLeft, 0),
      typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingTop, 0),
      typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingRight, 0),
      typedArray.getInt(R.styleable.maplibre_LocationComponent_maplibre_iconPaddingBottom, 0),
    });

    builder.layerAbove(
      typedArray.getString(R.styleable.maplibre_LocationComponent_maplibre_layer_above));

    builder.layerBelow(
      typedArray.getString(R.styleable.maplibre_LocationComponent_maplibre_layer_below));

    float minScale = typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_minZoomIconScale, MIN_ZOOM_ICON_SCALE_DEFAULT);
    float maxScale = typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_maxZoomIconScale, MAX_ZOOM_ICON_SCALE_DEFAULT);
    builder.minZoomIconScale(minScale);
    builder.maxZoomIconScale(maxScale);

    float trackingAnimationDurationMultiplier = typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_trackingAnimationDurationMultiplier,
      TRACKING_ANIMATION_DURATION_MULTIPLIER_DEFAULT
    );
    builder.trackingAnimationDurationMultiplier(trackingAnimationDurationMultiplier);

    builder.compassAnimationEnabled = typedArray.getBoolean(
      R.styleable.maplibre_LocationComponent_maplibre_compassAnimationEnabled, true
    );

    builder.accuracyAnimationEnabled = typedArray.getBoolean(
      R.styleable.maplibre_LocationComponent_maplibre_accuracyAnimationEnabled, true
    );

    builder.pulseEnabled = typedArray.getBoolean(
      R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleEnabled, false
    );

    builder.pulseFadeEnabled = typedArray.getBoolean(
      R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleFadeEnabled, true
    );

    if (typedArray.hasValue(R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleColor)) {
      builder.pulseColor(typedArray.getColor(
        R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleColor,
        -1));
    }

    builder.pulseSingleDuration = typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleDuration, CIRCLE_PULSING_DURATION_DEFAULT_MS
    );

    builder.pulseMaxRadius = typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleRadius, CIRCLE_PULSING_MAX_RADIUS_DEFAULT
    );

    builder.pulseAlpha = typedArray.getFloat(
      R.styleable.maplibre_LocationComponent_maplibre_pulsingLocationCircleAlpha, CIRCLE_PULSING_ALPHA_DEFAULT);

    typedArray.recycle();

    return builder.build();
  }

  /**
   * Takes the currently constructed {@link LocationComponentOptions} object and provides it's builder
   * with all the values set matching the values in this instance. This allows you to modify a
   * single attribute and then rebuild the object.
   *
   * @return the builder which contains the values defined in this current instance as defaults.
   */
  @NonNull
  public Builder toBuilder() {
    return new Builder(this);
  }

  /**
   * Build a new instance of the {@link LocationComponentOptions} class with all the attributes set
   * automatically to their defined defaults in this library. This allows you to adjust a few
   * attributes while leaving the rest alone and maintaining their default behavior.
   *
   * @param context your activities context used to acquire the style resource
   * @return the builder which contains the default values defined by the style resource
   */
  @NonNull
  public static Builder builder(@NonNull Context context) {
    return LocationComponentOptions.createFromAttributes(context,
      R.style.maplibre_LocationComponent).toBuilder();
  }

  /**
   * Set the opacity of the accuracy view to a value from 0 to 1, where 0 means the accuracy view is
   * completely transparent and 1 means the view is completely opaque.
   * References style attribute R.styleable#LocationComponent_accuracyAlpha
   *
   * @return the opacity of the accuracy view
   */
  public float accuracyAlpha() {
    return accuracyAlpha;
  }

  /**
   * Solid color to use as the accuracy view color property.
   * References style attribute R.styleable#LocationComponent_accuracyColor
   *
   * @return the color of the accuracy view
   */
  @ColorInt
  public int accuracyColor() {
    return accuracyColor;
  }

  /**
   * Defines the drawable used for the stale background icon.
   * References style attribute R.styleable#LocationComponent_backgroundDrawableStale
   *
   * @return the drawable resource ID
   */
  @DrawableRes
  public int backgroundDrawableStale() {
    return backgroundDrawableStale;
  }

  /**
   * String image name, identical to one used in
   * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
   * component, will use this image in place of the provided or default maplibre_foregroundDrawableStale.
   * <p>
   * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
   * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
   * </p>
   *
   * @return String icon or maki-icon name
   */
  @Nullable
  public String backgroundStaleName() {
    return backgroundStaleName;
  }

  /**
   * Defines the drawable used for the stale foreground icon.
   * References style attribute R.styleable#LocationComponent_foregroundDrawableStale
   *
   * @return the drawable resource ID
   */
  @DrawableRes
  public int foregroundDrawableStale() {
    return foregroundDrawableStale;
  }

  /**
   * String image name, identical to one used in
   * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
   * component, will used this image in place of the provided or default maplibre_foregroundDrawableStale.
   * <p>
   * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
   * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
   * </p>
   *
   * @return String icon or maki-icon name
   */
  @Nullable
  public String foregroundStaleName() {
    return foregroundStaleName;
  }

  /**
   * Defines the drawable used for the navigation state icon.
   * References style attribute R.styleable#LocationComponent_gpsDrawable
   *
   * @return the drawable resource ID
   */
  @DrawableRes
  public int gpsDrawable() {
    return gpsDrawable;
  }

  /**
   * String image name, identical to one used in
   * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
   * component, will used this image in place of the provided or default maplibre_gpsDrawable.
   * <p>
   * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
   * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
   * </p>
   *
   * @return String icon or maki-icon name
   */
  @Nullable
  public String gpsName() {
    return gpsName;
  }

  /**
   * Supply a Drawable that is to be rendered on top of all of the content in the Location LayerComponent layer stack.
   * References style attribute R.styleable#LocationComponent_foregroundDrawable
   *
   * @return the drawable resource used for the foreground layer
   */
  @DrawableRes
  public int foregroundDrawable() {
    return foregroundDrawable;
  }

  /**
   * String image name, identical to one used in
   * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
   * component, will used this image in place of the provided or default maplibre_foregroundDrawable.
   * <p>
   * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
   * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
   * </p>
   *
   * @return String icon or maki-icon name
   */
  @Nullable
  public String foregroundName() {
    return foregroundName;
  }

  /**
   * Defines the drawable used for the background state icon.
   * References style attribute R.styleable#LocationComponent_backgroundDrawable
   *
   * @return the drawable resource ID
   */
  @DrawableRes
  public int backgroundDrawable() {
    return backgroundDrawable;
  }

  /**
   * String image name, identical to one used in
   * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
   * component, will used this image in place of the provided or default maplibre_backgroundDrawable.
   * <p>
   * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
   * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
   * </p>
   *
   * @return String icon or maki-icon name
   */
  @Nullable
  public String backgroundName() {
    return backgroundName;
  }

  /**
   * Defines the drawable used for the bearing icon.
   * References style attribute R.styleable#LocationComponent_bearingDrawable
   *
   * @return the drawable resource ID
   */
  @DrawableRes
  public int bearingDrawable() {
    return bearingDrawable;
  }

  /**
   * String image name, identical to one used in
   * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
   * component, will used this image in place of the provided or default maplibre_bearingDrawable.
   * <p>
   * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
   * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
   * </p>
   *
   * @return String icon or maki-icon name
   */
  @Nullable
  public String bearingName() {
    return bearingName;
  }

  /**
   * Defines the bearing icon color as an integer.
   * References style attribute R.styleable#LocationComponent_bearingTintColor
   *
   * @return the color integer resource
   */
  @ColorInt
  @Nullable
  public Integer bearingTintColor() {
    return bearingTintColor;
  }

  /**
   * Defines the foreground color as an integer.
   * References style attribute R.styleable#LocationComponent_foregroundTintColor
   *
   * @return the color integer resource
   */
  @ColorInt
  @Nullable
  public Integer foregroundTintColor() {
    return foregroundTintColor;
  }

  /**
   * Defines the background color as an integer.
   * References style attribute R.styleable#LocationComponent_backgroundTintColor
   *
   * @return the color integer resource
   */
  @ColorInt
  @Nullable
  public Integer backgroundTintColor() {
    return backgroundTintColor;
  }

  /**
   * Defines the foreground stale color as an integer.
   * References style attribute R.styleable#LocationComponent_foregroundStaleTintColor
   *
   * @return the color integer resource
   */
  @ColorInt
  @Nullable
  public Integer foregroundStaleTintColor() {
    return foregroundStaleTintColor;
  }

  /**
   * Defines the background stale color as an integer.
   * References style attribute R.styleable#LocationComponent_backgroundStaleTintColor
   *
   * @return the color integer resource
   */
  @ColorInt
  @Nullable
  public Integer backgroundStaleTintColor() {
    return backgroundStaleTintColor;
  }

  /**
   * Sets the base elevation of this view, in pixels. To turn off the shadow that appears under
   * the location icon, set the elevation to 0.
   * References style attribute R.styleable#LocationComponent_elevation
   *
   * @return the elevation currently set for the location component icon
   */
  @Dimension
  public float elevation() {
    return elevation;
  }

  /**
   * Enable or disable to stale state mode. This mode indicates to the user that the location being
   * displayed on the map hasn't been updated in a specific amount of time.
   * References style attribute R.styleable#LocationComponent_enableStaleState
   *
   * @return whether the stale state mode is enabled or not
   */
  public boolean enableStaleState() {
    return enableStaleState;
  }

  /**
   * Set the delay before the location icon becomes stale. The timer begins approximately when a new
   * location update comes in and using this defined time, if an update hasn't occured by the end,
   * the location is considered stale.
   * References style attribute R.styleable#LocationComponent_staleStateDelay
   *
   * @return the duration in milliseconds which it should take before the location is
   * considered stale
   */
  public long staleStateTimeout() {
    return staleStateTimeout;
  }

  /**
   * Sets the distance from the edges of the map view’s frame to the edges of the map
   * view’s logical viewport.
   * </p>
   * <p>
   * When the value of this property is equal to {0,0,0,0}, viewport
   * properties such as `centerCoordinate` assume a viewport that matches the map
   * view’s frame. Otherwise, those properties are inset, excluding part of the
   * frame from the viewport. For instance, if the only the top edge is inset, the
   * map center is effectively shifted downward.
   * </p>
   *
   * @return integer array of padding values
   */
  @Nullable
  @SuppressWarnings("mutable")
  public int[] padding() {
    return padding;
  }

  /**
   * The scale factor of the location icon when the map is zoomed in.
   * Scaling is linear.
   *
   * @return icon scale factor
   */
  public float maxZoomIconScale() {
    return maxZoomIconScale;
  }

  /**
   * The scale factor of the location icon when the map is zoomed out.
   * Scaling is linear.
   *
   * @return icon scale factor
   */
  public float minZoomIconScale() {
    return minZoomIconScale;
  }

  /**
   * Returns whether gesture threshold should be adjusted when camera is in one of the tracking modes.
   * This will adjust the focal point and increase thresholds to enable camera manipulation,
   * like zooming in and out, without breaking tracking.
   * <p>
   * <strong>Note</strong>: If set to true, this can overwrite some of the gesture thresholds
   * and the custom {@link com.mapbox.android.gestures.AndroidGesturesManager} that was set with
   * {@link MapLibreMap#setGesturesManager(AndroidGesturesManager, boolean, boolean)}.
   *
   * @return true if gestures are adjusted when in one of the camera tracking modes, false otherwise
   * @see Builder#trackingInitialMoveThreshold(float)
   * @see Builder#trackingMultiFingerMoveThreshold(float)
   * @see Builder#trackingMultiFingerProtectedMoveArea(RectF)
   */
  public boolean trackingGesturesManagement() {
    return trackingGesturesManagement;
  }

  /**
   * Minimum single pointer movement in pixels required to break camera tracking.
   *
   * @return the minimum movement
   */
  public float trackingInitialMoveThreshold() {
    return trackingInitialMoveThreshold;
  }

  /**
   * Minimum multi pointer movement in pixels required to break camera tracking (for example during scale gesture).
   *
   * @return the minimum movement
   */
  public float trackingMultiFingerMoveThreshold() {
    return trackingMultiFingerMoveThreshold;
  }

  /**
   * Protected multi pointer gesture area. When the camera is in a tracking mode, any multi finger gesture with focal
   * point inside the provided screen coordinate rectangle is not going to break the tracking.
   * <p>
   * Best paired with the {@link LocationComponentOptions.Builder#trackingMultiFingerMoveThreshold(float)}
   * set to 0 or a relatively small value to not interfere with gestures outside of the defined rectangle.
   *
   * @return the protected multi finger area while camera is tracking
   */
  @Nullable
  public RectF trackingMultiFingerProtectedMoveArea() {
    return trackingMultiFingerProtectedMoveArea;
  }

  /**
   * Gets the id of the layer that's referenced when placing the component on the map using
   * {@link org.maplibre.android.maps.Style#addLayerAbove(Layer, String)}.
   * <p>
   * The component is going to placed directly above this layer.
   *
   * @return layerAbove the id of the layer the component is going to placed directly above.
   */
  public String layerAbove() {
    return layerAbove;
  }

  /**
   * Gets the id of the layer that's referenced when placing the component on the map using
   * {@link org.maplibre.android.maps.Style#addLayerBelow(Layer, String)}.
   * <p>
   * The component is going to placed directly below this layer.
   *
   * @return layerBelow the id of the layer the component is going to placed directly below.
   */
  public String layerBelow() {
    return layerBelow;
  }

  /**
   * Get the tracking animation duration multiplier.
   *
   * @return tracking animation duration multiplier
   */
  public float trackingAnimationDurationMultiplier() {
    return trackingAnimationDurationMultiplier;
  }

  /**
   * Enable or disable smooth animation of compass values for {@link org.maplibre.android.location.modes.CameraMode}
   * and {@link org.maplibre.android.location.modes.RenderMode}.
   *
   * @return whether smooth compass animation is enabled
   */
  public boolean compassAnimationEnabled() {
    return compassAnimationEnabled;
  }

  /**
   * Enable or disable smooth animation of the accuracy circle around the user's position.
   *
   * @return whether smooth animation of the accuracy circle is enabled
   */
  public boolean accuracyAnimationEnabled() {
    return accuracyAnimationEnabled;
  }

  /**
   * Enable or disable the LocationComponent's pulsing circle.
   *
   * @return whether the LocationComponent's pulsing circle is enabled
   */
  public Boolean pulseEnabled() {
    return pulseEnabled;
  }

  /**
   * Enable or disable fading of the LocationComponent's pulsing circle. If it fades, the circle's
   * opacity decreases as its radius increases.
   *
   * @return whether fading of the LocationComponent's pulsing circle is enabled
   */
  public Boolean pulseFadeEnabled() {
    return pulseFadeEnabled;
  }

  /**
   * Color of the LocationComponent's pulsing circle as it pulses.
   *
   * @return the current set color of the circle
   */
  public Integer pulseColor() {
    return pulseColor;
  }

  /**
   * The number of milliseconds it takes for a single pulse of the LocationComponent's pulsing circle.
   *
   * @return the current set length of time for a single pulse
   */
  public float pulseSingleDuration() {
    return pulseSingleDuration;
  }

  /**
   * The maximum radius that a single pulse should expand the LocationComponent's pulsing circle to.
   *
   * @return the maximum radius that the pulsing circle will expand to.
   */
  public float pulseMaxRadius() {
    return pulseMaxRadius;
  }

  /**
   * The opacity of the LocationComponent's circle as it pulses. The expected range is
   * 0 to 1. An opacity of 1 makes the layer fully visible.
   *
   * @return the current opacity of the LocationComponent's pulsing circle
   */
  public float pulseAlpha() {
    return pulseAlpha;
  }

  /**
   * The interpolator type of animation for the movement of the LocationComponent's circle
   *
   * @return the current set type of animation interpolator for the pulsing circle
   */
  @Nullable
  public Interpolator pulseInterpolator() {
    return pulseInterpolator;
  }

  @NonNull
  @Override
  public String toString() {
    return "LocationComponentOptions{"
      + "accuracyAlpha=" + accuracyAlpha + ", "
      + "accuracyColor=" + accuracyColor + ", "
      + "backgroundDrawableStale=" + backgroundDrawableStale + ", "
      + "backgroundStaleName=" + backgroundStaleName + ", "
      + "foregroundDrawableStale=" + foregroundDrawableStale + ", "
      + "foregroundStaleName=" + foregroundStaleName + ", "
      + "gpsDrawable=" + gpsDrawable + ", "
      + "gpsName=" + gpsName + ", "
      + "foregroundDrawable=" + foregroundDrawable + ", "
      + "foregroundName=" + foregroundName + ", "
      + "backgroundDrawable=" + backgroundDrawable + ", "
      + "backgroundName=" + backgroundName + ", "
      + "bearingDrawable=" + bearingDrawable + ", "
      + "bearingName=" + bearingName + ", "
      + "bearingTintColor=" + bearingTintColor + ", "
      + "foregroundTintColor=" + foregroundTintColor + ", "
      + "backgroundTintColor=" + backgroundTintColor + ", "
      + "foregroundStaleTintColor=" + foregroundStaleTintColor + ", "
      + "backgroundStaleTintColor=" + backgroundStaleTintColor + ", "
      + "elevation=" + elevation + ", "
      + "enableStaleState=" + enableStaleState + ", "
      + "staleStateTimeout=" + staleStateTimeout + ", "
      + "padding=" + Arrays.toString(padding) + ", "
      + "maxZoomIconScale=" + maxZoomIconScale + ", "
      + "minZoomIconScale=" + minZoomIconScale + ", "
      + "trackingGesturesManagement=" + trackingGesturesManagement + ", "
      + "trackingInitialMoveThreshold=" + trackingInitialMoveThreshold + ", "
      + "trackingMultiFingerMoveThreshold=" + trackingMultiFingerMoveThreshold + ", "
      + "trackingMultiFingerProtectedMoveArea=" + trackingMultiFingerProtectedMoveArea + ", "
      + "layerAbove=" + layerAbove
      + "layerBelow=" + layerBelow
      + "trackingAnimationDurationMultiplier=" + trackingAnimationDurationMultiplier
      + "pulseEnabled=" + pulseEnabled
      + "pulseFadeEnabled=" + pulseFadeEnabled
      + "pulseColor=" + pulseColor
      + "pulseSingleDuration=" + pulseSingleDuration
      + "pulseMaxRadius=" + pulseMaxRadius
      + "pulseAlpha=" + pulseAlpha
      + "}";
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) {
      return true;
    }
    if (o == null || getClass() != o.getClass()) {
      return false;
    }

    LocationComponentOptions options = (LocationComponentOptions) o;

    if (Float.compare(options.accuracyAlpha, accuracyAlpha) != 0) {
      return false;
    }
    if (accuracyColor != options.accuracyColor) {
      return false;
    }
    if (backgroundDrawableStale != options.backgroundDrawableStale) {
      return false;
    }
    if (foregroundDrawableStale != options.foregroundDrawableStale) {
      return false;
    }
    if (gpsDrawable != options.gpsDrawable) {
      return false;
    }
    if (foregroundDrawable != options.foregroundDrawable) {
      return false;
    }
    if (backgroundDrawable != options.backgroundDrawable) {
      return false;
    }
    if (bearingDrawable != options.bearingDrawable) {
      return false;
    }
    if (Float.compare(options.elevation, elevation) != 0) {
      return false;
    }
    if (enableStaleState != options.enableStaleState) {
      return false;
    }
    if (staleStateTimeout != options.staleStateTimeout) {
      return false;
    }
    if (Float.compare(options.maxZoomIconScale, maxZoomIconScale) != 0) {
      return false;
    }
    if (Float.compare(options.minZoomIconScale, minZoomIconScale) != 0) {
      return false;
    }
    if (trackingGesturesManagement != options.trackingGesturesManagement) {
      return false;
    }
    if (Float.compare(options.trackingInitialMoveThreshold, trackingInitialMoveThreshold) != 0) {
      return false;
    }
    if (Float.compare(options.trackingMultiFingerMoveThreshold, trackingMultiFingerMoveThreshold) != 0) {
      return false;
    }
    if (Float.compare(options.trackingAnimationDurationMultiplier, trackingAnimationDurationMultiplier) != 0) {
      return false;
    }
    if (trackingMultiFingerProtectedMoveArea != null
      ? !trackingMultiFingerProtectedMoveArea.equals(options.trackingMultiFingerProtectedMoveArea) :
      options.trackingMultiFingerProtectedMoveArea != null) {
      return false;
    }
    if (compassAnimationEnabled != options.compassAnimationEnabled) {
      return false;
    }
    if (accuracyAnimationEnabled != options.accuracyAnimationEnabled) {
      return false;
    }
    if (backgroundStaleName != null ? !backgroundStaleName.equals(options.backgroundStaleName) :
      options.backgroundStaleName != null) {
      return false;
    }
    if (foregroundStaleName != null ? !foregroundStaleName.equals(options.foregroundStaleName) :
      options.foregroundStaleName != null) {
      return false;
    }
    if (gpsName != null ? !gpsName.equals(options.gpsName) : options.gpsName != null) {
      return false;
    }
    if (foregroundName != null ? !foregroundName.equals(options.foregroundName) : options.foregroundName != null) {
      return false;
    }
    if (backgroundName != null ? !backgroundName.equals(options.backgroundName) : options.backgroundName != null) {
      return false;
    }
    if (bearingName != null ? !bearingName.equals(options.bearingName) : options.bearingName != null) {
      return false;
    }
    if (bearingTintColor != null ? !bearingTintColor.equals(options.bearingTintColor) :
      options.bearingTintColor != null) {
      return false;
    }
    if (foregroundTintColor != null ? !foregroundTintColor.equals(options.foregroundTintColor) :
      options.foregroundTintColor != null) {
      return false;
    }
    if (backgroundTintColor != null ? !backgroundTintColor.equals(options.backgroundTintColor) :
      options.backgroundTintColor != null) {
      return false;
    }
    if (foregroundStaleTintColor != null ? !foregroundStaleTintColor.equals(options.foregroundStaleTintColor) :
      options.foregroundStaleTintColor != null) {
      return false;
    }
    if (backgroundStaleTintColor != null ? !backgroundStaleTintColor.equals(options.backgroundStaleTintColor) :
      options.backgroundStaleTintColor != null) {
      return false;
    }
    if (!Arrays.equals(padding, options.padding)) {
      return false;
    }
    if (layerAbove != null ? !layerAbove.equals(options.layerAbove) : options.layerAbove != null) {
      return false;
    }

    if (pulseEnabled != options.pulseEnabled) {
      return false;
    }

    if (pulseFadeEnabled != options.pulseFadeEnabled) {
      return false;
    }

    if (pulseColor != null ? !pulseColor.equals(options.pulseColor) :
      options.pulseColor() != null) {
      return false;
    }
    if (Float.compare(options.pulseSingleDuration, pulseSingleDuration) != 0) {
      return false;
    }

    if (Float.compare(options.pulseMaxRadius, pulseMaxRadius) != 0) {
      return false;
    }

    if (Float.compare(options.pulseAlpha, pulseAlpha) != 0) {
      return false;
    }

    return layerBelow != null ? layerBelow.equals(options.layerBelow) : options.layerBelow == null;
  }

  @Override
  public int hashCode() {
    int result = (accuracyAlpha != +0.0f ? Float.floatToIntBits(accuracyAlpha) : 0);
    result = 31 * result + accuracyColor;
    result = 31 * result + backgroundDrawableStale;
    result = 31 * result + (backgroundStaleName != null ? backgroundStaleName.hashCode() : 0);
    result = 31 * result + foregroundDrawableStale;
    result = 31 * result + (foregroundStaleName != null ? foregroundStaleName.hashCode() : 0);
    result = 31 * result + gpsDrawable;
    result = 31 * result + (gpsName != null ? gpsName.hashCode() : 0);
    result = 31 * result + foregroundDrawable;
    result = 31 * result + (foregroundName != null ? foregroundName.hashCode() : 0);
    result = 31 * result + backgroundDrawable;
    result = 31 * result + (backgroundName != null ? backgroundName.hashCode() : 0);
    result = 31 * result + bearingDrawable;
    result = 31 * result + (bearingName != null ? bearingName.hashCode() : 0);
    result = 31 * result + (bearingTintColor != null ? bearingTintColor.hashCode() : 0);
    result = 31 * result + (foregroundTintColor != null ? foregroundTintColor.hashCode() : 0);
    result = 31 * result + (backgroundTintColor != null ? backgroundTintColor.hashCode() : 0);
    result = 31 * result + (foregroundStaleTintColor != null ? foregroundStaleTintColor.hashCode() : 0);
    result = 31 * result + (backgroundStaleTintColor != null ? backgroundStaleTintColor.hashCode() : 0);
    result = 31 * result + (elevation != +0.0f ? Float.floatToIntBits(elevation) : 0);
    result = 31 * result + (enableStaleState ? 1 : 0);
    result = 31 * result + (int) (staleStateTimeout ^ (staleStateTimeout >>> 32));
    result = 31 * result + Arrays.hashCode(padding);
    result = 31 * result + (maxZoomIconScale != +0.0f ? Float.floatToIntBits(maxZoomIconScale) : 0);
    result = 31 * result + (minZoomIconScale != +0.0f ? Float.floatToIntBits(minZoomIconScale) : 0);
    result = 31 * result + (trackingGesturesManagement ? 1 : 0);
    result = 31 * result + (trackingInitialMoveThreshold != +0.0f
      ? Float.floatToIntBits(trackingInitialMoveThreshold) : 0);
    result = 31 * result + (trackingMultiFingerMoveThreshold != +0.0f
      ? Float.floatToIntBits(trackingMultiFingerMoveThreshold) : 0);
    result = 31 * result + (trackingMultiFingerProtectedMoveArea != null
      ? trackingMultiFingerProtectedMoveArea.hashCode() : 0);
    result = 31 * result + (layerAbove != null ? layerAbove.hashCode() : 0);
    result = 31 * result + (layerBelow != null ? layerBelow.hashCode() : 0);
    result = 31 * result + (trackingAnimationDurationMultiplier != +0.0f
      ? Float.floatToIntBits(trackingAnimationDurationMultiplier) : 0);
    result = 31 * result + (compassAnimationEnabled ? 1 : 0);
    result = 31 * result + (accuracyAnimationEnabled ? 1 : 0);
    result = 31 * result + (pulseEnabled ? 1 : 0);
    result = 31 * result + (pulseFadeEnabled ? 1 : 0);
    result = 31 * result + (pulseColor != null ? pulseColor.hashCode() : 0);
    result = 31 * result + (pulseSingleDuration != +0.0f ? Float.floatToIntBits(pulseSingleDuration) : 0);
    result = 31 * result + (pulseMaxRadius != +0.0f ? Float.floatToIntBits(pulseMaxRadius) : 0);
    result = 31 * result + (pulseAlpha != +0.0f ? Float.floatToIntBits(pulseAlpha) : 0);
    return result;
  }

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    dest.writeFloat(this.accuracyAlpha);
    dest.writeInt(this.accuracyColor);
    dest.writeInt(this.backgroundDrawableStale);
    dest.writeString(this.backgroundStaleName);
    dest.writeInt(this.foregroundDrawableStale);
    dest.writeString(this.foregroundStaleName);
    dest.writeInt(this.gpsDrawable);
    dest.writeString(this.gpsName);
    dest.writeInt(this.foregroundDrawable);
    dest.writeString(this.foregroundName);
    dest.writeInt(this.backgroundDrawable);
    dest.writeString(this.backgroundName);
    dest.writeInt(this.bearingDrawable);
    dest.writeString(this.bearingName);
    dest.writeValue(this.bearingTintColor);
    dest.writeValue(this.foregroundTintColor);
    dest.writeValue(this.backgroundTintColor);
    dest.writeValue(this.foregroundStaleTintColor);
    dest.writeValue(this.backgroundStaleTintColor);
    dest.writeFloat(this.elevation);
    dest.writeByte(this.enableStaleState ? (byte) 1 : (byte) 0);
    dest.writeLong(this.staleStateTimeout);
    dest.writeIntArray(this.padding);
    dest.writeFloat(this.maxZoomIconScale);
    dest.writeFloat(this.minZoomIconScale);
    dest.writeByte(this.trackingGesturesManagement ? (byte) 1 : (byte) 0);
    dest.writeFloat(this.trackingInitialMoveThreshold);
    dest.writeFloat(this.trackingMultiFingerMoveThreshold);
    dest.writeParcelable(this.trackingMultiFingerProtectedMoveArea, flags);
    dest.writeString(this.layerAbove);
    dest.writeString(this.layerBelow);
    dest.writeFloat(this.trackingAnimationDurationMultiplier);
    dest.writeByte(this.compassAnimationEnabled ? (byte) 1 : (byte) 0);
    dest.writeByte(this.accuracyAnimationEnabled ? (byte) 1 : (byte) 0);
    dest.writeValue(this.pulseEnabled);
    dest.writeValue(this.pulseFadeEnabled);
    dest.writeValue(this.pulseColor);
    dest.writeFloat(this.pulseSingleDuration);
    dest.writeFloat(this.pulseMaxRadius);
    dest.writeFloat(this.pulseAlpha);
  }

  protected LocationComponentOptions(Parcel in) {
    this.accuracyAlpha = in.readFloat();
    this.accuracyColor = in.readInt();
    this.backgroundDrawableStale = in.readInt();
    this.backgroundStaleName = in.readString();
    this.foregroundDrawableStale = in.readInt();
    this.foregroundStaleName = in.readString();
    this.gpsDrawable = in.readInt();
    this.gpsName = in.readString();
    this.foregroundDrawable = in.readInt();
    this.foregroundName = in.readString();
    this.backgroundDrawable = in.readInt();
    this.backgroundName = in.readString();
    this.bearingDrawable = in.readInt();
    this.bearingName = in.readString();
    this.bearingTintColor = (Integer) in.readValue(Integer.class.getClassLoader());
    this.foregroundTintColor = (Integer) in.readValue(Integer.class.getClassLoader());
    this.backgroundTintColor = (Integer) in.readValue(Integer.class.getClassLoader());
    this.foregroundStaleTintColor = (Integer) in.readValue(Integer.class.getClassLoader());
    this.backgroundStaleTintColor = (Integer) in.readValue(Integer.class.getClassLoader());
    this.elevation = in.readFloat();
    this.enableStaleState = in.readByte() != 0;
    this.staleStateTimeout = in.readLong();
    this.padding = in.createIntArray();
    this.maxZoomIconScale = in.readFloat();
    this.minZoomIconScale = in.readFloat();
    this.trackingGesturesManagement = in.readByte() != 0;
    this.trackingInitialMoveThreshold = in.readFloat();
    this.trackingMultiFingerMoveThreshold = in.readFloat();
    this.trackingMultiFingerProtectedMoveArea = in.readParcelable(RectF.class.getClassLoader());
    this.layerAbove = in.readString();
    this.layerBelow = in.readString();
    this.trackingAnimationDurationMultiplier = in.readFloat();
    this.compassAnimationEnabled = in.readByte() != 0;
    this.accuracyAnimationEnabled = in.readByte() != 0;
    this.pulseEnabled = (Boolean) in.readValue(Boolean.class.getClassLoader());
    this.pulseFadeEnabled = (Boolean) in.readValue(Boolean.class.getClassLoader());
    this.pulseColor = (Integer) in.readValue(Integer.class.getClassLoader());
    this.pulseSingleDuration = in.readFloat();
    this.pulseMaxRadius = in.readFloat();
    this.pulseAlpha = in.readFloat();
  }

  public static final Parcelable.Creator<LocationComponentOptions> CREATOR =
    new Parcelable.Creator<LocationComponentOptions>() {
      @Override
      public LocationComponentOptions createFromParcel(Parcel source) {
        return new LocationComponentOptions(source);
      }

      @Override
      public LocationComponentOptions[] newArray(int size) {
        return new LocationComponentOptions[size];
      }
    };

  /**
   * Builder class for constructing a new instance of {@link LocationComponentOptions}.
   */
  public static class Builder {

    /**
     * Build a new instance of this {@link LocationComponentOptions} class.
     *
     * @return a new instance of {@link LocationComponentOptions}
     */
    @NonNull
    public LocationComponentOptions build() {
      LocationComponentOptions locationComponentOptions = autoBuild();
      if (locationComponentOptions.accuracyAlpha() < 0 || locationComponentOptions.accuracyAlpha() > 1) {
        throw new IllegalArgumentException(
          "Accuracy alpha value must be between 0.0 and 1.0.");
      }

      if (locationComponentOptions.elevation() < 0f) {
        throw new IllegalArgumentException("Invalid shadow size "
          + locationComponentOptions.elevation() + ". Must be >= 0");
      }

      if (locationComponentOptions.layerAbove() != null && locationComponentOptions.layerBelow() != null) {
        throw new IllegalArgumentException("You cannot set both layerAbove and layerBelow options."
          + " Choose one or the other.");
      }

      if (locationComponentOptions.pulseEnabled() == null) {
        String pulsingSetupError = "";
        if (locationComponentOptions.pulseFadeEnabled() != null) {
          pulsingSetupError += " pulseFadeEnabled";
        }
        if (locationComponentOptions.pulseColor() != null) {
          pulsingSetupError += " pulseColor";
        }
        if (locationComponentOptions.pulseSingleDuration() > 0) {
          pulsingSetupError += " pulseSingleDuration";
        }
        if (locationComponentOptions.pulseMaxRadius() > 0) {
          pulsingSetupError += " pulseMaxRadius";
        }
        if (locationComponentOptions.pulseAlpha() >= 0 && locationComponentOptions.pulseAlpha() <= 1) {
          pulsingSetupError += " pulseAlpha";
        }
        if (locationComponentOptions.pulseInterpolator() != null) {
          pulsingSetupError += " pulseInterpolator";
        }
        if (!pulsingSetupError.isEmpty()) {
          throw new IllegalStateException("You've set up the following pulsing circle options but have not enabled"
            + " the pulsing circle via the LocationComponentOptions builder:" + pulsingSetupError
            + ". Enable the pulsing circle if you're going to set pulsing options.");
        }
      }
      return locationComponentOptions;
    }

    private Float accuracyAlpha;
    private Integer accuracyColor;
    private Integer backgroundDrawableStale;
    @Nullable
    private String backgroundStaleName;
    private Integer foregroundDrawableStale;
    @Nullable
    private String foregroundStaleName;
    private Integer gpsDrawable;
    @Nullable
    private String gpsName;
    private Integer foregroundDrawable;
    @Nullable
    private String foregroundName;
    private Integer backgroundDrawable;
    @Nullable
    private String backgroundName;
    private Integer bearingDrawable;
    @Nullable
    private String bearingName;
    @Nullable
    private Integer bearingTintColor;
    @Nullable
    private Integer foregroundTintColor;
    @Nullable
    private Integer backgroundTintColor;
    @Nullable
    private Integer foregroundStaleTintColor;
    @Nullable
    private Integer backgroundStaleTintColor;
    private Float elevation;
    private Boolean enableStaleState;
    private Long staleStateTimeout;
    @Nullable
    private int[] padding;
    private Float maxZoomIconScale;
    private Float minZoomIconScale;
    private Boolean trackingGesturesManagement;
    private Float trackingInitialMoveThreshold;
    private Float trackingMultiFingerMoveThreshold;
    private RectF trackingMultiFingerProtectedMoveArea;
    private String layerAbove;
    private String layerBelow;
    private Float trackingAnimationDurationMultiplier;
    private Boolean compassAnimationEnabled;
    private Boolean accuracyAnimationEnabled;
    private Boolean pulseEnabled;
    private Boolean pulseFadeEnabled;
    private int pulseColor;
    private float pulseSingleDuration;
    private float pulseMaxRadius;
    private float pulseAlpha;
    @Nullable
    private Interpolator pulseInterpolator;

    Builder() {
    }

    private Builder(LocationComponentOptions source) {
      this.accuracyAlpha = source.accuracyAlpha();
      this.accuracyColor = source.accuracyColor();
      this.backgroundDrawableStale = source.backgroundDrawableStale();
      this.backgroundStaleName = source.backgroundStaleName();
      this.foregroundDrawableStale = source.foregroundDrawableStale();
      this.foregroundStaleName = source.foregroundStaleName();
      this.gpsDrawable = source.gpsDrawable();
      this.gpsName = source.gpsName();
      this.foregroundDrawable = source.foregroundDrawable();
      this.foregroundName = source.foregroundName();
      this.backgroundDrawable = source.backgroundDrawable();
      this.backgroundName = source.backgroundName();
      this.bearingDrawable = source.bearingDrawable();
      this.bearingName = source.bearingName();
      this.bearingTintColor = source.bearingTintColor();
      this.foregroundTintColor = source.foregroundTintColor();
      this.backgroundTintColor = source.backgroundTintColor();
      this.foregroundStaleTintColor = source.foregroundStaleTintColor();
      this.backgroundStaleTintColor = source.backgroundStaleTintColor();
      this.elevation = source.elevation();
      this.enableStaleState = source.enableStaleState();
      this.staleStateTimeout = source.staleStateTimeout();
      this.padding = source.padding();
      this.maxZoomIconScale = source.maxZoomIconScale();
      this.minZoomIconScale = source.minZoomIconScale();
      this.trackingGesturesManagement = source.trackingGesturesManagement();
      this.trackingInitialMoveThreshold = source.trackingInitialMoveThreshold();
      this.trackingMultiFingerMoveThreshold = source.trackingMultiFingerMoveThreshold();
      this.trackingMultiFingerProtectedMoveArea = source.trackingMultiFingerProtectedMoveArea();
      this.layerAbove = source.layerAbove();
      this.layerBelow = source.layerBelow();
      this.trackingAnimationDurationMultiplier = source.trackingAnimationDurationMultiplier();
      this.compassAnimationEnabled = source.compassAnimationEnabled();
      this.accuracyAnimationEnabled = source.accuracyAnimationEnabled();
      this.pulseEnabled = source.pulseEnabled;
      this.pulseFadeEnabled = source.pulseFadeEnabled;
      this.pulseColor = source.pulseColor;
      this.pulseSingleDuration = source.pulseSingleDuration;
      this.pulseMaxRadius = source.pulseMaxRadius;
      this.pulseAlpha = source.pulseAlpha;
      this.pulseInterpolator = source.pulseInterpolator;
    }

    /**
     * Set the opacity of the accuracy view to a value from 0 to 1, where 0 means the accuracy view
     * is completely transparent and 1 means the view is completely opaque.
     * References style attribute R.styleable#LocationComponent_accuracyAlpha
     *
     * @param accuracyAlpha the opacity of the accuracy view
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder accuracyAlpha(float accuracyAlpha) {
      this.accuracyAlpha = accuracyAlpha;
      return this;
    }

    /**
     * Solid color to use as the accuracy view color property.
     * References style attribute R.styleable#LocationComponent_accuracyColor
     *
     * @param accuracyColor the color of the accuracy view
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder accuracyColor(int accuracyColor) {
      this.accuracyColor = accuracyColor;
      return this;
    }

    /**
     * Defines the drawable used for the stale background icon.
     * References style attribute R.styleable#LocationComponent_backgroundDrawableStale
     *
     * @param backgroundDrawableStale the drawable resource ID
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder backgroundDrawableStale(int backgroundDrawableStale) {
      this.backgroundDrawableStale = backgroundDrawableStale;
      return this;
    }

    /**
     * Given a String image name, identical to one used in
     * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
     * component, will used this image in place of the provided or default maplibre_backgroundDrawableStale.
     * <p>
     * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
     * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
     * </p>
     *
     * @param backgroundStaleName String icon or maki-icon name
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder backgroundStaleName(@Nullable String backgroundStaleName) {
      this.backgroundStaleName = backgroundStaleName;
      return this;
    }

    /**
     * Defines the drawable used for the stale foreground icon.
     * References style attribute R.styleable#LocationComponent_foregroundDrawableStale
     *
     * @param foregroundDrawableStale the drawable resource ID
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder foregroundDrawableStale(int foregroundDrawableStale) {
      this.foregroundDrawableStale = foregroundDrawableStale;
      return this;
    }

    /**
     * Given a String image name, identical to one used in
     * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
     * component, will used this image in place of the provided or default maplibre_foregroundDrawableStale.
     * <p>
     * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
     * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
     * </p>
     *
     * @param foregroundStaleName String icon or maki-icon name
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder foregroundStaleName(@Nullable String foregroundStaleName) {
      this.foregroundStaleName = foregroundStaleName;
      return this;
    }

    /**
     * Defines the drawable used for the navigation state icon.
     * References style attribute R.styleable#LocationComponent_gpsDrawable
     *
     * @param gpsDrawable the drawable resource ID
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder gpsDrawable(int gpsDrawable) {
      this.gpsDrawable = gpsDrawable;
      return this;
    }

    /**
     * Given a String image name, identical to one used in
     * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
     * component, will used this image in place of the provided or default maplibre_gpsDrawable.
     * <p>
     * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
     * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
     * </p>
     *
     * @param gpsName String icon or maki-icon name
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder gpsName(@Nullable String gpsName) {
      this.gpsName = gpsName;
      return this;
    }

    /**
     * Supply a Drawable that is to be rendered on top of all of the content in the Location Component layer stack.
     * References style attribute R.styleable#LocationComponent_foregroundDrawable
     *
     * @param foregroundDrawable the drawable resource used for the foreground layer
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder foregroundDrawable(int foregroundDrawable) {
      this.foregroundDrawable = foregroundDrawable;
      return this;
    }

    /**
     * Given a String image name, identical to one used in
     * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
     * component, will used this image in place of the provided or default maplibre_foregroundDrawable.
     * <p>
     * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
     * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
     * </p>
     *
     * @param foregroundName String icon or maki-icon name
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder foregroundName(@Nullable String foregroundName) {
      this.foregroundName = foregroundName;
      return this;
    }

    /**
     * Defines the drawable used for the background state icon.
     * References style attribute R.styleable#LocationComponent_backgroundDrawable
     *
     * @param backgroundDrawable the drawable resource ID
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder backgroundDrawable(int backgroundDrawable) {
      this.backgroundDrawable = backgroundDrawable;
      return this;
    }

    /**
     * Given a String image name, identical to one used in
     * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
     * component, will used this image in place of the provided or default maplibre_backgroundDrawable.
     * <p>
     * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
     * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
     * </p>
     *
     * @param backgroundName String icon or maki-icon name
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder backgroundName(@Nullable String backgroundName) {
      this.backgroundName = backgroundName;
      return this;
    }

    /**
     * Defines the drawable used for the bearing icon.
     * References style attribute R.styleable#LocationComponent_bearingDrawable
     *
     * @param bearingDrawable the drawable resource ID
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder bearingDrawable(int bearingDrawable) {
      this.bearingDrawable = bearingDrawable;
      return this;
    }

    /**
     * Given a String image name, identical to one used in
     * the first parameter of {@link org.maplibre.android.maps.Style.Builder#addImage(String, Bitmap)}, the
     * component, will used this image in place of the provided or default maplibre_bearingDrawable.
     * <p>
     * A maki-icon name (example: "circle-15") may also be provided.  These are images that can be loaded
     * with certain styles.  Note, this will fail if the provided icon name is not provided by the loaded map style.
     * </p>
     *
     * @param bearingName String icon or maki-icon name
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder bearingName(@Nullable String bearingName) {
      this.bearingName = bearingName;
      return this;
    }

    /**
     * Defines the bearing icon color as an integer.
     * References style attribute R.styleable#LocationComponent_bearingTintColor
     *
     * @param bearingTintColor the color integer resource
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder bearingTintColor(@Nullable Integer bearingTintColor) {
      this.bearingTintColor = bearingTintColor;
      return this;
    }

    /**
     * Defines the foreground color as an integer.
     * References style attribute R.styleable#LocationComponent_foregroundTintColor
     *
     * @param foregroundTintColor the color integer resource
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder foregroundTintColor(@Nullable Integer foregroundTintColor) {
      this.foregroundTintColor = foregroundTintColor;
      return this;
    }

    /**
     * Defines the background color as an integer.
     * References style attribute R.styleable#LocationComponent_backgroundTintColor
     *
     * @param backgroundTintColor the color integer resource
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder backgroundTintColor(@Nullable Integer backgroundTintColor) {
      this.backgroundTintColor = backgroundTintColor;
      return this;
    }

    /**
     * Defines the foreground stale color as an integer.
     * References style attribute R.styleable#LocationComponent_foregroundStaleTintColor
     *
     * @param foregroundStaleTintColor the color integer resource
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder foregroundStaleTintColor(@Nullable Integer foregroundStaleTintColor) {
      this.foregroundStaleTintColor = foregroundStaleTintColor;
      return this;
    }

    /**
     * Defines the background stale color as an integer.
     * References style attribute R.styleable#LocationComponent_backgroundStaleTintColor
     *
     * @param backgroundStaleTintColor the color integer resource
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder backgroundStaleTintColor(@Nullable Integer backgroundStaleTintColor) {
      this.backgroundStaleTintColor = backgroundStaleTintColor;
      return this;
    }

    /**
     * Sets the base elevation of this view, in pixels.
     * References style attribute R.styleable#LocationComponent_elevation
     *
     * @param elevation the elevation currently set for the location icon
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder elevation(float elevation) {
      this.elevation = elevation;
      return this;
    }

    /**
     * Enable or disable to stale state mode. This mode indicates to the user that the location
     * being displayed on the map hasn't been updated in a specific amount of time.
     * References style attribute R.styleable#LocationComponent_enableStaleState
     *
     * @param enabled whether the stale state mode is enabled or not
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder enableStaleState(boolean enabled) {
      this.enableStaleState = enabled;
      return this;
    }

    /**
     * Set the timeout before the location icon becomes stale. The timer begins approximately when a
     * new location update comes in and using this defined time, if an update hasn't occurred by the
     * end, the location is considered stale.
     * References style attribute R.styleable#LocationComponent_staleStateTimeout
     *
     * @param timeout the duration in milliseconds which it should take before the location is
     *                considered stale
     * @return this builder for chaining options together
     */
    @NonNull
    public LocationComponentOptions.Builder staleStateTimeout(long timeout) {
      this.staleStateTimeout = timeout;
      return this;
    }

    /**
     * Sets the distance from the edges of the map view’s frame to the edges of the map
     * view’s logical viewport.
     * </p>
     * <p>
     * When the value of this property is equal to {0,0,0,0}, viewport
     * properties such as `centerCoordinate` assume a viewport that matches the map
     * view’s frame. Otherwise, those properties are inset, excluding part of the
     * frame from the viewport. For instance, if the only the top edge is inset, the
     * map center is effectively shifted downward.
     * </p>
     *
     * @param padding The margins for the map in pixels (left, top, right, bottom).
     * @deprecated Use
     * {@link org.maplibre.android.camera.CameraPosition.Builder#padding(double, double, double, double)} or
     * {@link org.maplibre.android.camera.CameraUpdateFactory#paddingTo(double, double, double, double)}
     * instead.
     */
    @NonNull
    @Deprecated
    public LocationComponentOptions.Builder padding(@Nullable int[] padding) {
      if (padding == null) {
        throw new NullPointerException("Null padding");
      }
      this.padding = padding;
      return this;
    }

    /**
     * Sets the scale factor of the location icon when the map is zoomed in.
     * Scaling is linear and the new pixel size of the image will be the original pixel size multiplied by the argument.
     * <p>
     * Set both this and {@link #minZoomIconScale(float)} to 1f to disable location icon scaling.
     * <p>
     * Scaling is based on the maps minimum and maximum zoom levels in time of component's style application.
     *
     * @param maxZoomIconScale icon scale factor
     */
    @NonNull
    public LocationComponentOptions.Builder maxZoomIconScale(float maxZoomIconScale) {
      this.maxZoomIconScale = maxZoomIconScale;
      return this;
    }

    /**
     * Sets the scale factor of the location icon when the map is zoomed out.
     * Scaling is linear and the new pixel size of the image will be the original pixel size multiplied by the argument.
     * <p>
     * Set both this and {@link #maxZoomIconScale(float)} to 1f to disable location icon scaling.
     * <p>
     * Scaling is based on the maps minimum and maximum zoom levels in time of component's style application.
     *
     * @param minZoomIconScale icon scale factor
     */
    @NonNull
    public LocationComponentOptions.Builder minZoomIconScale(float minZoomIconScale) {
      this.minZoomIconScale = minZoomIconScale;
      return this;
    }

    /**
     * Set whether gesture threshold should be adjusted when camera is in one of the tracking modes.
     * This will adjust the focal point and increase thresholds to enable camera manipulation,
     * like zooming in and out, without breaking tracking.
     * <p>
     * <strong>Note</strong>: This can overwrite some of the gesture thresholds
     * and the custom {@link com.mapbox.android.gestures.AndroidGesturesManager} that was set with
     * {@link MapLibreMap#setGesturesManager(AndroidGesturesManager, boolean, boolean)}.
     *
     * @param trackingGesturesManagement true if gestures should be adjusted when in one of the camera tracking modes,
     *                                   false otherwise
     * @see Builder#trackingInitialMoveThreshold(float)
     * @see Builder#trackingMultiFingerMoveThreshold(float)
     * @see Builder#trackingMultiFingerProtectedMoveArea(RectF)
     */
    @NonNull
    public LocationComponentOptions.Builder trackingGesturesManagement(boolean trackingGesturesManagement) {
      this.trackingGesturesManagement = trackingGesturesManagement;
      return this;
    }

    /**
     * Sets minimum single pointer movement (map pan) in pixels required to break camera tracking.
     *
     * @param moveThreshold the minimum movement
     */
    @NonNull
    public LocationComponentOptions.Builder trackingInitialMoveThreshold(float moveThreshold) {
      this.trackingInitialMoveThreshold = moveThreshold;
      return this;
    }

    /**
     * Sets minimum multi pointer movement (map pan) in pixels required to break camera tracking
     * (for example during scale gesture).
     *
     * @param moveThreshold the minimum movement
     */
    @NonNull
    public LocationComponentOptions.Builder trackingMultiFingerMoveThreshold(float moveThreshold) {
      this.trackingMultiFingerMoveThreshold = moveThreshold;
      return this;
    }

    /**
     * Sets protected multi pointer gesture area.
     * When the camera is in a tracking mode,any multi finger gesture with focal
     * point inside the provided screen coordinate rectangle is not going to break the tracking.
     * <p>
     * Best paired with the {@link LocationComponentOptions.Builder#trackingMultiFingerMoveThreshold(float)}
     * set to 0 or a relatively small value to not interfere with gestures outside of the defined rectangle.
     *
     * @param rect the protected multi finger area while camera is tracking
     */
    @NonNull
    public LocationComponentOptions.Builder trackingMultiFingerProtectedMoveArea(@Nullable RectF rect) {
      this.trackingMultiFingerProtectedMoveArea = rect;
      return this;
    }

    /**
     * Sets the id of the layer that's referenced when placing the component on the map using
     * {@link org.maplibre.android.maps.Style#addLayerAbove(Layer, String)}.
     * <p>
     * The component is going to placed directly above this layer.
     *
     * @param layerAbove the id of the layer the component is going to placed directly above.
     */
    @NonNull
    public LocationComponentOptions.Builder layerAbove(String layerAbove) {
      this.layerAbove = layerAbove;
      return this;
    }

    /**
     * Sets the id of the layer that's referenced when placing the component on the map using
     * {@link org.maplibre.android.maps.Style#addLayerBelow(Layer, String)}.
     * <p>
     * The component is going to placed directly below this layer.
     *
     * @param layerBelow the id of the layer the component is going to placed directly below.
     */
    @NonNull
    public LocationComponentOptions.Builder layerBelow(String layerBelow) {
      this.layerBelow = layerBelow;
      return this;
    }

    /**
     * Sets the tracking animation duration multiplier.
     *
     * @param trackingAnimationDurationMultiplier the tracking animation duration multiplier
     */
    @NonNull
    public LocationComponentOptions.Builder trackingAnimationDurationMultiplier(
      float trackingAnimationDurationMultiplier) {
      this.trackingAnimationDurationMultiplier = trackingAnimationDurationMultiplier;
      return this;
    }

    /**
     * Enable or disable smooth animation of compass values for {@link org.maplibre.android.location.modes.CameraMode}
     * and {@link org.maplibre.android.location.modes.RenderMode}.
     *
     * @return whether smooth compass animation is enabled
     */
    public LocationComponentOptions.Builder compassAnimationEnabled(Boolean compassAnimationEnabled) {
      this.compassAnimationEnabled = compassAnimationEnabled;
      return this;
    }

    /**
     * Enable or disable smooth animation of the accuracy circle around the user's position.
     *
     * @return whether smooth animation of the accuracy circle is enabled
     */
    public LocationComponentOptions.Builder accuracyAnimationEnabled(boolean accuracyAnimationEnabled) {
      this.accuracyAnimationEnabled = accuracyAnimationEnabled;
      return this;
    }

    /**
     * Enable or disable the LocationComponent's pulsing circle.
     *
     * @return whether the LocationComponent's pulsing circle is enabled
     */
    public LocationComponentOptions.Builder pulseEnabled(boolean pulseEnabled) {
      this.pulseEnabled = pulseEnabled;
      return this;
    }

    /**
     * Enable or disable fading of the LocationComponent's pulsing circle. If it fades, the circle's
     * opacity decreases as its radius increases.
     *
     * @return whether fading of the LocationComponent's pulsing circle is enabled
     */
    public LocationComponentOptions.Builder pulseFadeEnabled(boolean pulseFadeEnabled) {
      this.pulseFadeEnabled = pulseFadeEnabled;
      return this;
    }

    /**
     * Sets the color of the LocationComponent's pulsing circle.
     *
     * @return the current set color of the circle
     */
    public LocationComponentOptions.Builder pulseColor(@ColorInt int pulseColor) {
      this.pulseColor = pulseColor;
      return this;
    }

    /**
     * Sets the number of milliseconds it takes for a single pulse of the LocationComponent's pulsing circle.
     *
     * @return the current set length of time for a single pulse
     */
    public LocationComponentOptions.Builder pulseSingleDuration(float pulseSingleDuration) {
      this.pulseSingleDuration = pulseSingleDuration;
      return this;
    }

    /**
     * The maximum radius that a single pulse should expand the LocationComponent's pulsing circle to.
     *
     * @return the maximum radius that the pulsing circle will expand to.
     */
    public LocationComponentOptions.Builder pulseMaxRadius(float pulseMaxRadius) {
      this.pulseMaxRadius = pulseMaxRadius;
      return this;
    }

    /**
     * Sets the opacity of the LocationComponent's pulsing circle. The expected range is
     * 0 to 1. An opacity of 1 makes the layer fully visible.
     *
     * @return the current opacity of the LocationComponent's pulsing circle
     */
    public LocationComponentOptions.Builder pulseAlpha(float pulseAlpha) {
      this.pulseAlpha = pulseAlpha;
      return this;
    }

    /**
     * Sets the pulsing circle's interpolator animation.
     *
     * @param pulseInterpolator the type of Android-system interpolator to use when
     *                          creating the pulsing animation
     * @return a String which represents the interpolator animation that the pulsing circle will use.
     */
    public LocationComponentOptions.Builder pulseInterpolator(Interpolator pulseInterpolator) {
      this.pulseInterpolator = pulseInterpolator;
      return this;
    }

    @Nullable
    LocationComponentOptions autoBuild() {
      String missing = "";
      if (this.accuracyAlpha == null) {
        missing += " accuracyAlpha";
      }
      if (this.accuracyColor == null) {
        missing += " accuracyColor";
      }
      if (this.backgroundDrawableStale == null) {
        missing += " backgroundDrawableStale";
      }
      if (this.foregroundDrawableStale == null) {
        missing += " foregroundDrawableStale";
      }
      if (this.gpsDrawable == null) {
        missing += " gpsDrawable";
      }
      if (this.foregroundDrawable == null) {
        missing += " foregroundDrawable";
      }
      if (this.backgroundDrawable == null) {
        missing += " backgroundDrawable";
      }
      if (this.bearingDrawable == null) {
        missing += " bearingDrawable";
      }
      if (this.elevation == null) {
        missing += " elevation";
      }
      if (this.enableStaleState == null) {
        missing += " enableStaleState";
      }
      if (this.staleStateTimeout == null) {
        missing += " staleStateTimeout";
      }
      if (this.padding == null) {
        missing += " padding";
      }
      if (this.maxZoomIconScale == null) {
        missing += " maxZoomIconScale";
      }
      if (this.minZoomIconScale == null) {
        missing += " minZoomIconScale";
      }
      if (this.trackingGesturesManagement == null) {
        missing += " trackingGesturesManagement";
      }
      if (this.trackingInitialMoveThreshold == null) {
        missing += " trackingInitialMoveThreshold";
      }
      if (this.trackingMultiFingerMoveThreshold == null) {
        missing += " trackingMultiFingerMoveThreshold";
      }
      if (this.trackingAnimationDurationMultiplier == null) {
        missing += " trackingAnimationDurationMultiplier";
      }
      if (!missing.isEmpty()) {
        throw new IllegalStateException("Missing required properties:" + missing);
      }
      return new LocationComponentOptions(
        this.accuracyAlpha,
        this.accuracyColor,
        this.backgroundDrawableStale,
        this.backgroundStaleName,
        this.foregroundDrawableStale,
        this.foregroundStaleName,
        this.gpsDrawable,
        this.gpsName,
        this.foregroundDrawable,
        this.foregroundName,
        this.backgroundDrawable,
        this.backgroundName,
        this.bearingDrawable,
        this.bearingName,
        this.bearingTintColor,
        this.foregroundTintColor,
        this.backgroundTintColor,
        this.foregroundStaleTintColor,
        this.backgroundStaleTintColor,
        this.elevation,
        this.enableStaleState,
        this.staleStateTimeout,
        this.padding,
        this.maxZoomIconScale,
        this.minZoomIconScale,
        trackingGesturesManagement,
        this.trackingInitialMoveThreshold,
        this.trackingMultiFingerMoveThreshold,
        this.trackingMultiFingerProtectedMoveArea,
        this.layerAbove,
        this.layerBelow,
        this.trackingAnimationDurationMultiplier,
        this.compassAnimationEnabled,
        this.accuracyAnimationEnabled,
        this.pulseEnabled,
        this.pulseFadeEnabled,
        this.pulseColor,
        this.pulseSingleDuration,
        this.pulseMaxRadius,
        this.pulseAlpha,
        this.pulseInterpolator);
    }
  }
}
