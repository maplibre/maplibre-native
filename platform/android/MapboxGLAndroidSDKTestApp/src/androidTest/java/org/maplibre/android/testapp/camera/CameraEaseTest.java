package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.maps.MapboxMap;

public class CameraEaseTest extends CameraTest {

  @Override
  void executeCameraMovement(CameraUpdate cameraUpdate, MapboxMap.CancelableCallback callback) {
    mapboxMap.easeCamera(cameraUpdate, callback);
  }
}