package org.maplibre.android.location;

import org.maplibre.android.location.modes.RenderMode;

/**
 * Listener that gets invoked when layer render mode changes.
 */
public interface OnRenderModeChangedListener {

  /**
   * Invoked on every {@link RenderMode} change.
   *
   * @param currentMode current active {@link RenderMode}.
   */
  void onRenderModeChanged(@RenderMode.Mode int currentMode);
}
