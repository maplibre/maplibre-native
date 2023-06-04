package org.maplibre.android.location.engine;

import android.app.PendingIntent;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

/**
 * Internal location engine implementation interface.
 *
 * @param <T> location listener object type
 */
public interface LocationEngineImpl<T> {
  @NonNull
  T createListener(LocationEngineCallback<LocationEngineResult> callback);

  void getLastLocation(@NonNull LocationEngineCallback<LocationEngineResult> callback) throws SecurityException;

  void requestLocationUpdates(@NonNull LocationEngineRequest request,
                              @NonNull T listener, @Nullable Looper looper) throws SecurityException;

  void requestLocationUpdates(@NonNull LocationEngineRequest request,
                              @NonNull PendingIntent pendingIntent) throws SecurityException;

  void removeLocationUpdates(T listener);

  void removeLocationUpdates(PendingIntent pendingIntent);
}
