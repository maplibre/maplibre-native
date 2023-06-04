package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.maps.MaplibreMap;

public class CameraAnimateTest extends CameraTest {
  @Override
  void executeCameraMovement(CameraUpdate cameraUpdate, MaplibreMap.CancelableCallback callback) {
    maplibreMap.animateCamera(cameraUpdate, callback);
  }
}