package org.maplibre.android.maps;

/**
 * Controls progressive loading of 3D-terrain content (draped tiles and drape re-renders).
 * It trades initial-load sharpness for smoother interaction on weaker GPUs, so it is a
 * per-map, hardware-driven choice set via {@link MapLibreMap#setTerrainLoadMode(TerrainLoadMode)}.
 *
 * <p>This is a rendering/performance option, not a style property, so it does not affect the
 * final rendered image - only how the loading work is spread across frames. The enum ordinals
 * must stay in sync with the native {@code mln::TerrainLoadMode}.</p>
 */
public enum TerrainLoadMode {

  /**
   * No budget: all revealed tiles and drapes build immediately. Sharpest initial view; may
   * stall a frame on large bursts (e.g. zooming in over new coverage). Default.
   */
  QUALITY,

  /**
   * Cap 32 new-tile builds and 16 drape re-renders per frame - a middle ground that keeps a
   * near-instant initial view while smoothing the worst interaction stalls.
   */
  BALANCED,

  /**
   * Cap 8 new-tile builds and 4 drape re-renders per frame - smoothest interaction on weak
   * GPUs, with the most visible progressive fill-in.
   */
  PERFORMANCE
}
