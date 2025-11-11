package org.maplibre.android.location;

import androidx.annotation.NonNull;

import org.maplibre.android.geometry.LatLng;

/**
 * Listener invoked whenever the rendered puck position changes.
 */
public interface OnPuckPositionChangeListener {
  /**
   * Called when the puck position is updated as part of the location animation.
   *
   * @param puckPosition current rendered puck position.
   */
  void onPuckPositionChanged(@NonNull LatLng puckPosition);
}
