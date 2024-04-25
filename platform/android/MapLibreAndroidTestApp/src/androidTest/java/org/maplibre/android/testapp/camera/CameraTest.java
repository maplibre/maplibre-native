package org.maplibre.android.testapp.camera;

import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.camera.CameraUpdate;
import org.maplibre.android.camera.CameraUpdateFactory;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.geometry.LatLngBounds;
import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.testapp.activity.BaseTest;
import org.maplibre.android.testapp.activity.espresso.DeviceIndependentTestActivity;
import org.maplibre.android.testapp.utils.TestConstants;

import org.junit.Assert;
import org.junit.Test;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import static org.maplibre.android.testapp.action.MapLibreMapAction.invoke;
import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

public abstract class CameraTest extends BaseTest {

  private CountDownLatch latch;

  @Override
  protected Class getActivityClass() {
    return DeviceIndependentTestActivity.class;
  }

  @Override
  public void beforeTest() {
    super.beforeTest();
    latch = new CountDownLatch(1);
  }

  @Test
  public void testToCameraPositionTarget() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      float zoom = 1.0f;
      LatLng moveTarget = new LatLng(1, 1);
      CameraPosition initialPosition = new CameraPosition.Builder().target(
        new LatLng()).zoom(zoom).bearing(0).tilt(0).padding(new double[4]).build();
      CameraPosition cameraPosition = maplibreMap.getCameraPosition();
      assertEquals("Default camera position should match default", cameraPosition, initialPosition);

      executeCameraMovement(CameraUpdateFactory.newLatLng(moveTarget), new MapLibreMap.CancelableCallback() {
        @Override
        public void onCancel() {
          verifyCameraPosition(maplibreMap, moveTarget, zoom, 0, 0, new double[4]);
          latch.countDown();
        }

        @Override
        public void onFinish() {
          verifyCameraPosition(maplibreMap, moveTarget, zoom, 0, 0, new double[4]);
          latch.countDown();
        }
      });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToCameraPositionTargetZoom() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      final float moveZoom = 15.5f;
      final LatLng moveTarget = new LatLng(1.0000000001, 1.0000000003);

      executeCameraMovement(CameraUpdateFactory.newLatLngZoom(moveTarget, moveZoom),
        new MapLibreMap.CancelableCallback() {
          @Override
          public void onCancel() {
            verifyCameraPosition(maplibreMap, moveTarget, moveZoom, 0, 0, new double[4]);
            latch.countDown();
          }

          @Override
          public void onFinish() {
            verifyCameraPosition(maplibreMap, moveTarget, moveZoom, 0, 0, new double[4]);
            latch.countDown();
          }
        });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToCameraPosition() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      final LatLng moveTarget = new LatLng(1.0000000001, 1.0000000003);
      final float moveZoom = 15.5f;
      final float moveTilt = 45.5f;
      final float moveBearing = 12.5f;
      final double[] movePadding = new double[] {0, 500, 350, 1};

      executeCameraMovement(CameraUpdateFactory.newCameraPosition(
        new CameraPosition.Builder()
          .target(moveTarget)
          .zoom(moveZoom)
          .tilt(moveTilt)
          .bearing(moveBearing)
          .padding(movePadding)
          .build()),
        new MapLibreMap.CancelableCallback() {
          @Override
          public void onCancel() {
            verifyCameraPosition(maplibreMap, moveTarget, moveZoom, moveBearing, moveTilt, movePadding);
            latch.countDown();
          }

          @Override
          public void onFinish() {
            verifyCameraPosition(maplibreMap, moveTarget, moveZoom, moveBearing, moveTilt, movePadding);
            latch.countDown();
          }
        });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToBounds() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      final LatLng centerBounds = new LatLng(1, 1);
      LatLng cornerOne = new LatLng();
      LatLng cornerTwo = new LatLng(2, 2);
      final LatLngBounds.Builder builder = new LatLngBounds.Builder();
      builder.include(cornerOne);
      builder.include(cornerTwo);

      executeCameraMovement(CameraUpdateFactory.newLatLngBounds(builder.build(), 0),
        new MapLibreMap.CancelableCallback() {
          @Override
          public void onCancel() {
            verifyCameraPosition(maplibreMap, centerBounds, maplibreMap.getCameraPosition().zoom, 0, 0, new double[4]);
            latch.countDown();
          }

          @Override
          public void onFinish() {
            verifyCameraPosition(maplibreMap, centerBounds, maplibreMap.getCameraPosition().zoom, 0, 0, new double[4]);
            latch.countDown();
          }
        });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToZoomIn() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      float zoom = 1.0f;

      executeCameraMovement(CameraUpdateFactory.zoomIn(), new MapLibreMap.CancelableCallback() {
        @Override
        public void onCancel() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom + 1, 0, 0, new double[4]);
          latch.countDown();
        }

        @Override
        public void onFinish() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom + 1, 0, 0, new double[4]);
          latch.countDown();
        }
      });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToZoomOut() throws InterruptedException {
    float zoom = 10.0f;
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) ->
      executeCameraMovement(CameraUpdateFactory.newLatLngZoom(new LatLng(), zoom),
        new MapLibreMap.CancelableCallback() {
          @Override
          public void onCancel() {
            verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom, 0, 0, new double[4]);
            latch.countDown();
          }

          @Override
          public void onFinish() {
            verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom, 0, 0, new double[4]);
            latch.countDown();
          }
        })
    );

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }

    latch = new CountDownLatch(1);

    invoke(maplibreMap, (uiController, maplibreMap) -> {
      executeCameraMovement(CameraUpdateFactory.zoomOut(), new MapLibreMap.CancelableCallback() {
        @Override
        public void onCancel() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom - 1, 0, 0, new double[4]);
          latch.countDown();
        }

        @Override
        public void onFinish() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom - 1, 0, 0, new double[4]);
          latch.countDown();
        }
      });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToZoomBy() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      float zoom = 1.0f;
      final float zoomBy = 2.45f;

      executeCameraMovement(CameraUpdateFactory.zoomBy(zoomBy), new MapLibreMap.CancelableCallback() {
        @Override
        public void onCancel() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom + zoomBy, 0, 0, new double[4]);
          latch.countDown();
        }

        @Override
        public void onFinish() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoom + zoomBy, 0, 0, new double[4]);
          latch.countDown();
        }
      });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  @Test
  public void testToZoomTo() throws InterruptedException {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      final float zoomTo = 2.45f;

      executeCameraMovement(CameraUpdateFactory.zoomTo(zoomTo), new MapLibreMap.CancelableCallback() {
        @Override
        public void onCancel() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoomTo, 0, 0, new double[4]);
          latch.countDown();
        }

        @Override
        public void onFinish() {
          verifyCameraPosition(maplibreMap, maplibreMap.getCameraPosition().target, zoomTo, 0, 0, new double[4]);
          latch.countDown();
        }
      });
    });

    if (!latch.await(10, TimeUnit.SECONDS)) {
      Assert.fail("timeout");
    }
  }

  abstract void executeCameraMovement(CameraUpdate cameraUpdate, MapLibreMap.CancelableCallback callback);

  private void verifyCameraPosition(MapLibreMap maplibreMap, LatLng moveTarget, double moveZoom, double moveBearing,
                                    double moveTilt, double[] padding) {
    CameraPosition cameraPosition = maplibreMap.getCameraPosition();
    assertEquals("Moved camera position latitude should match", cameraPosition.target.getLatitude(),
      moveTarget.getLatitude(), TestConstants.LAT_LNG_DELTA);
    assertEquals("Moved camera position longitude should match", cameraPosition.target.getLongitude(),
      moveTarget.getLongitude(), TestConstants.LAT_LNG_DELTA);
    assertEquals("Moved zoom should match", cameraPosition.zoom, moveZoom, TestConstants.ZOOM_DELTA);
    assertEquals("Moved zoom should match", cameraPosition.tilt, moveTilt, TestConstants.TILT_DELTA);
    assertEquals("Moved bearing should match", cameraPosition.bearing, moveBearing, TestConstants.BEARING_DELTA);
    assertArrayEquals("Moved padding should match", cameraPosition.padding, padding, TestConstants.PADDING_DELTA);
  }
}
