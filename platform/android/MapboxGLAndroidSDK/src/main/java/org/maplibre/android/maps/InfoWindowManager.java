package org.maplibre.android.maps;

import android.text.TextUtils;

import androidx.annotation.Nullable;

import org.maplibre.android.annotations.InfoWindow;
import org.maplibre.android.annotations.Marker;

import java.util.ArrayList;
import java.util.List;

/**
 * Responsible for managing InfoWindows shown on the Map.
 * <p>
 * Maintains a {@link List} of opened {@link InfoWindow} and tracks configurations as
 * allowConcurrentMultipleInfoWindows which allows to have multiple {@link InfoWindow} open at the
 * same time. Responsible for managing listeners as
 * {@link MaplibreMap.OnInfoWindowClickListener} and
 * {@link MaplibreMap.OnInfoWindowLongClickListener}.
 * </p>
 */
class InfoWindowManager {

  private final List<InfoWindow> infoWindows = new ArrayList<>();

  @Nullable
  private MaplibreMap.InfoWindowAdapter infoWindowAdapter;
  private boolean allowConcurrentMultipleInfoWindows;

  @Nullable
  private MaplibreMap.OnInfoWindowClickListener onInfoWindowClickListener;
  @Nullable
  private MaplibreMap.OnInfoWindowLongClickListener onInfoWindowLongClickListener;
  @Nullable
  private MaplibreMap.OnInfoWindowCloseListener onInfoWindowCloseListener;

  void update() {
    if (!infoWindows.isEmpty()) {
      for (InfoWindow infoWindow : infoWindows) {
        infoWindow.update();
      }
    }
  }

  void setInfoWindowAdapter(@Nullable MaplibreMap.InfoWindowAdapter infoWindowAdapter) {
    this.infoWindowAdapter = infoWindowAdapter;
  }

  @Nullable
  MaplibreMap.InfoWindowAdapter getInfoWindowAdapter() {
    return infoWindowAdapter;
  }

  void setAllowConcurrentMultipleOpenInfoWindows(boolean allow) {
    allowConcurrentMultipleInfoWindows = allow;
  }

  boolean isAllowConcurrentMultipleOpenInfoWindows() {
    return allowConcurrentMultipleInfoWindows;
  }

  boolean isInfoWindowValidForMarker(@Nullable Marker marker) {
    return marker != null && (!TextUtils.isEmpty(marker.getTitle()) || !TextUtils.isEmpty(marker.getSnippet()));
  }

  void setOnInfoWindowClickListener(@Nullable MaplibreMap.OnInfoWindowClickListener listener) {
    onInfoWindowClickListener = listener;
  }

  @Nullable
  MaplibreMap.OnInfoWindowClickListener getOnInfoWindowClickListener() {
    return onInfoWindowClickListener;
  }

  void setOnInfoWindowLongClickListener(@Nullable MaplibreMap.OnInfoWindowLongClickListener listener) {
    onInfoWindowLongClickListener = listener;
  }

  @Nullable
  MaplibreMap.OnInfoWindowLongClickListener getOnInfoWindowLongClickListener() {
    return onInfoWindowLongClickListener;
  }

  void setOnInfoWindowCloseListener(@Nullable MaplibreMap.OnInfoWindowCloseListener listener) {
    onInfoWindowCloseListener = listener;
  }

  @Nullable
  MaplibreMap.OnInfoWindowCloseListener getOnInfoWindowCloseListener() {
    return onInfoWindowCloseListener;
  }

  public void add(InfoWindow infoWindow) {
    infoWindows.add(infoWindow);
  }
}
