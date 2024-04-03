package org.maplibre.android.location.engine;

import android.app.PendingIntent;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresPermission;

import static android.Manifest.permission.ACCESS_COARSE_LOCATION;
import static android.Manifest.permission.ACCESS_FINE_LOCATION;

/**
 * Generic location engine interface wrapper for the location providers.
 * Default providers bundled with MapLibre location library:
 * Android location provider and Google Play Services fused location provider
 *
 * @since 1.0.0
 */
public interface LocationEngine {

  /**
   * Returns the most recent location currently available.
   * <p>
   * If a location is not available, which should happen very rarely, null will be returned.
   *
   * @param callback {@link LocationEngineCallback} for the location result {@link LocationEngineResult}.
   * @throws SecurityException if permission is not granted to access location services.
   * @since 1.0.0
   */
  @RequiresPermission(anyOf = {ACCESS_COARSE_LOCATION, ACCESS_FINE_LOCATION})
  void getLastLocation(@NonNull LocationEngineCallback<LocationEngineResult> callback) throws SecurityException;

  /**
   * Requests location updates with a callback on the specified Looper thread.
   *
   * @param request  {@link LocationEngineRequest} for the updates.
   * @param callback {@link LocationEngineCallback} for the location result {@link LocationEngineResult}.
   * @param looper   The Looper object whose message queue will be used to implement the callback mechanism,
   *                 or null to invoke callbacks on the main thread.
   * @throws SecurityException if permission is not granted to access location services.
   * @since 1.0.0
   */
  @RequiresPermission(anyOf = {ACCESS_COARSE_LOCATION, ACCESS_FINE_LOCATION})
  void requestLocationUpdates(@NonNull LocationEngineRequest request,
                              @NonNull LocationEngineCallback<LocationEngineResult> callback,
                              @Nullable Looper looper) throws SecurityException;

  /**
   * Requests location updates with callback on the specified PendingIntent.
   *
   * @param request       {@link LocationEngineRequest} for the updates.
   * @param pendingIntent {@link PendingIntent} for the location result {@link LocationEngineResult}.
   * @throws SecurityException if permission is not granted to access location services.
   * @since 1.1.0
   */
  @RequiresPermission(anyOf = {ACCESS_COARSE_LOCATION, ACCESS_FINE_LOCATION})
  void requestLocationUpdates(@NonNull LocationEngineRequest request,
                              PendingIntent pendingIntent) throws SecurityException;

  /**
   * Removes location updates for the given location engine callback.
   * <p>
   * It is recommended to remove location requests when the activity is in a paused or
   * stopped state, doing so helps battery performance.
   *
   * @param callback {@link LocationEngineCallback} to remove.
   * @since 1.0.0
   */
  void removeLocationUpdates(@NonNull LocationEngineCallback<LocationEngineResult> callback);

  /**
   * Removes location updates for the given pending intent.
   * <p>
   * It is recommended to remove location requests when the activity is in a paused or
   * stopped state, doing so helps battery performance.
   *
   * @param pendingIntent {@link PendingIntent} to remove.
   * @since 1.1.0
   */
  void removeLocationUpdates(PendingIntent pendingIntent);
}