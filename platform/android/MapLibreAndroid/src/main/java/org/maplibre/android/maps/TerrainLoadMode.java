package org.maplibre.android.maps;

/**
 * Controls progressive loading of 3D-terrain content (draped tiles and drape re-renders) and
 * how much terrain mesh a frame may have. It trades initial-load sharpness for smoother
 * interaction on weaker GPUs, so it is a per-map, hardware-driven choice set via
 * {@link MapLibreMap#setTerrainLoadMode(TerrainLoadMode)}.
 *
 * <p>This is a rendering/performance option, not a style property. Besides spreading the
 * loading work across frames, it sets how deep the terrain mesh goes past a coarse DEM and how
 * many mesh tiles a frame may have, so it changes the final image: close-ups are sharper in
 * {@link #QUALITY}, and tilted views keep more detail in the distance. The enum ordinals must
 * stay in sync with the native {@code mln::TerrainLoadMode}.</p>
 */
public enum TerrainLoadMode {

  /**
   * No budget: all revealed tiles and drapes build immediately. Sharpest initial view; may
   * stall a frame on large bursts (e.g. zooming in over new coverage). Meshes the terrain to z16
   * over a coarser DEM, with up to 64 mesh tiles. Default.
   */
  QUALITY,

  /**
   * Cap 32 new-tile builds and 16 drape re-renders per frame - a middle ground that keeps a
   * near-instant initial view while smoothing the worst interaction stalls. Meshes the terrain
   * to z15 over a coarser DEM, with up to 48 mesh tiles.
   */
  BALANCED,

  /**
   * Cap 8 new-tile builds and 4 drape re-renders per frame - smoothest interaction on weak
   * GPUs, with the most visible progressive fill-in. Meshes the terrain to z14 over a coarser
   * DEM, with up to 24 mesh tiles.
   */
  PERFORMANCE
}
