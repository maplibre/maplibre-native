
package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.maps.MapboxMap;

public class CameraMoveTest extends CameraTest {
  @Override
  void executeCameraMovement(CameraUpdate cameraUpdate, MapboxMap.CancelableCallback callback) {
    mapboxMap.moveCamera(cameraUpdate, callback);
  }
}