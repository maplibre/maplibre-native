package org.maplibre.android.location.engine;

import androidx.annotation.NonNull;

/**
 * Invoked for asynchronous notifications when new data
 * from engine becomes available.
 *
 * @param <T> Successful updated data type
 */
public interface LocationEngineCallback<T> {
  /**
   * Invoked when new data available.
   *
   * @param result updated data.
   */
  void onSuccess(T result);

  /**
   * Invoked when engine exception occurs.
   *
   * @param exception {@link Exception}
   */
  void onFailure(@NonNull Exception exception);
}