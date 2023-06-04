package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.maps.MaplibreMap;

public class CameraEaseTest extends CameraTest {

  @Override
  void executeCameraMovement(CameraUpdate cameraUpdate, MaplibreMap.CancelableCallback callback) {
    maplibreMap.easeCamera(cameraUpdate, callback);
  }
}