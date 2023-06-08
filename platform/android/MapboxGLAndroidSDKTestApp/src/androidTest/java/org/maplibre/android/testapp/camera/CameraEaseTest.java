package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.maps.MapLibreMap;

public class CameraEaseTest extends CameraTest {

  @Override
  void executeCameraMovement(CameraUpdate cameraUpdate, MapLibreMap.CancelableCallback callback) {
    maplibreMap.easeCamera(cameraUpdate, callback);
  }
}