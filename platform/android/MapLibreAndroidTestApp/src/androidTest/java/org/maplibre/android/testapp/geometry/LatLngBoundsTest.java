package org.maplibre.android.testapp.geometry;

import static org.junit.Assert.assertEquals;

import org.junit.Ignore;
import org.maplibre.android.camera.CameraUpdateFactory;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.geometry.LatLngBounds;
import org.maplibre.android.testapp.action.MapLibreMapAction;
import org.maplibre.android.testapp.activity.BaseTest;
import org.maplibre.android.testapp.activity.feature.QueryRenderedFeaturesBoxHighlightActivity;
import org.maplibre.android.testapp.utils.TestConstants;

import org.junit.Test;

/**
 * Instrumentation test to validate integration of LatLngBounds
 */
@Ignore("https://github.com/maplibre/maplibre-native/issues/2319")
public class LatLngBoundsTest extends BaseTest {

  private static final double MAP_BEARING = 50;

  @Override
  protected Class getActivityClass() {
    return QueryRenderedFeaturesBoxHighlightActivity.class;
  }

  @Test
  public void testLatLngBounds() {
    // regression test for #9322
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      LatLngBounds bounds = new LatLngBounds.Builder()
        .include(new LatLng(48.8589506, 2.2773457))
        .include(new LatLng(47.2383171, -1.6309316))
        .build();
      maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0));
    });
  }

  @Test
  public void testLatLngBoundsBearing() {
    // regression test for #12549
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      LatLngBounds bounds = new LatLngBounds.Builder()
        .include(new LatLng(48.8589506, 2.2773457))
        .include(new LatLng(47.2383171, -1.6309316))
        .build();
      maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0));
      maplibreMap.moveCamera(CameraUpdateFactory.bearingTo(MAP_BEARING));
      assertEquals(
        "Initial bearing should match for latlngbounds",
        maplibreMap.getCameraPosition().bearing,
        MAP_BEARING,
        TestConstants.BEARING_DELTA
      );

      maplibreMap.moveCamera(CameraUpdateFactory.newLatLngBounds(bounds, 0));
      assertEquals("Bearing should match after resetting latlngbounds",
        maplibreMap.getCameraPosition().bearing,
        MAP_BEARING,
        TestConstants.BEARING_DELTA);
    });
  }

}