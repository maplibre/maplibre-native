package org.maplibre.android.maps;

/**
 * Controls the vertical skirts extruded from every 3D-terrain tile edge, set via
 * {@link MapLibreMap#setTerrainSkirtLength(TerrainSkirtLength)}.
 *
 * <p>Skirts hide the hairline gaps (stitches) between neighbouring tiles at different zoom
 * levels, but show as vertical artifacts where the map has a transparent background, so which
 * one you want is a per-map tradeoff. The enum ordinals must stay in sync with the native
 * {@code mln::TerrainSkirtLength}.</p>
 */
public enum TerrainSkirtLength {

  /**
   * Skirt every tile edge by ~1/5 of the tile's width at the current zoom. Default.
   */
  AUTO,

  /**
   * Build no skirts at all: no vertical artifacts over a transparent background, at the cost
   * of visible stitches between tiles at different zoom levels.
   */
  NONE
}
