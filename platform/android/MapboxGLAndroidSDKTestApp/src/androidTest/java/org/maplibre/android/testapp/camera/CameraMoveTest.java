
package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.maps.MapLibreMap;

public class CameraMoveTest extends CameraTest {
  @Override
  void executeCameraMovement(CameraUpdate cameraUpdate, MapLibreMap.CancelableCallback callback) {
    maplibreMap.moveCamera(cameraUpdate, callback);
  }
}