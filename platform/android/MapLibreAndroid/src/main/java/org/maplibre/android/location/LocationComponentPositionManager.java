package org.maplibre.android.location;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.android.maps.Style;
import org.maplibre.android.style.layers.Layer;

class LocationComponentPositionManager {

  @NonNull
  private final Style style;

  @Nullable
  private String layerAbove;

  @Nullable
  private String layerBelow;

  public Boolean bearingOnTop;

  LocationComponentPositionManager(@NonNull Style style, @Nullable String layerAbove,
                                   @Nullable String layerBelow, Boolean bearingOnTop) {
    this.style = style;
    this.layerAbove = layerAbove;
    this.layerBelow = layerBelow;
    this.bearingOnTop = bearingOnTop;
  }

  /**
   * Returns true whenever layer above/below configuration has changed and requires re-layout.
   */
  boolean update(@Nullable String layerAbove, @Nullable String layerBelow, Boolean bearingOnTop) {
    boolean requiresUpdate =
      !(this.layerAbove == layerAbove || (this.layerAbove != null && this.layerAbove.equals(layerAbove)))
        || !(this.layerBelow == layerBelow || (this.layerBelow != null && this.layerBelow.equals(layerBelow)))
        || (this.bearingOnTop != bearingOnTop);

    this.layerAbove = layerAbove;
    this.layerBelow = layerBelow;
    this.bearingOnTop = bearingOnTop;
    return requiresUpdate;
  }

  void addLayerToMap(@NonNull Layer layer) {
    if (layerAbove != null) {
      style.addLayerAbove(layer, layerAbove);
    } else if (layerBelow != null) {
      style.addLayerBelow(layer, layerBelow);
    } else {
      style.addLayer(layer);
    }
  }
}
