package org.maplibre.android.maps;

import android.graphics.Color;
import android.view.View;

import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;

import org.maplibre.android.annotations.BaseMarkerOptions;
import org.maplibre.android.annotations.Marker;
import org.maplibre.android.annotations.MarkerOptions;
import org.maplibre.android.annotations.Polygon;
import org.maplibre.android.annotations.PolygonOptions;
import org.maplibre.android.annotations.Polyline;
import org.maplibre.android.annotations.PolylineOptions;
import org.maplibre.android.exceptions.InvalidMarkerPositionException;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.testapp.R;
import org.maplibre.android.testapp.activity.EspressoTest;

import org.hamcrest.Matcher;
import org.junit.Test;

import java.util.ArrayList;
import java.util.List;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static junit.framework.TestCase.assertFalse;
import static junit.framework.TestCase.assertNotNull;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * This test is responsible for testing the public API.
 * <p>
 * Methods executed on MapboxMap are called from a ViewAction to ensure correct synchronisation
 * with the application UI-thread.
 * </p>
 * @deprecated remove this file when removing deprecated annotations
 */
@Deprecated
public class MapLibreMapTest extends EspressoTest {

  //
  // InfoWindow
  //

  @Test
  public void testConcurrentInfoWindowEnabled() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      maplibreMap.setAllowConcurrentMultipleOpenInfoWindows(true);
      assertTrue("ConcurrentWindows should be true", maplibreMap.isAllowConcurrentMultipleOpenInfoWindows());
    }));
  }

  @Test
  public void testConcurrentInfoWindowDisabled() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      maplibreMap.setAllowConcurrentMultipleOpenInfoWindows(false);
      assertFalse("ConcurrentWindows should be false", maplibreMap.isAllowConcurrentMultipleOpenInfoWindows());
    }));
  }

  @Test
  public void testInfoWindowAdapter() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MapLibreMap.InfoWindowAdapter infoWindowAdapter = marker -> null;
      maplibreMap.setInfoWindowAdapter(infoWindowAdapter);
      assertEquals("InfoWindowAdpter should be the same", infoWindowAdapter, maplibreMap.getInfoWindowAdapter());
    }));
  }

  //
  // Annotations
  //

  @Test
  public void testAddMarker() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker marker = maplibreMap.addMarker(markerOptions);
      assertTrue("Marker should be contained", maplibreMap.getMarkers().contains(marker));
    }));
  }

  @Test(expected = InvalidMarkerPositionException.class)
  public void testAddMarkerInvalidPosition() {
    new MarkerOptions().getMarker();
  }

  @Test
  public void testAddMarkers() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<BaseMarkerOptions> markerList = new ArrayList<>();
      MarkerOptions markerOptions1 = new MarkerOptions().position(new LatLng()).title("a");
      MarkerOptions markerOptions2 = new MarkerOptions().position(new LatLng()).title("b");
      markerList.add(markerOptions1);
      markerList.add(markerOptions2);
      List<Marker> markers = maplibreMap.addMarkers(markerList);
      assertEquals("Markers size should be 2", 2, maplibreMap.getMarkers().size());
      assertTrue(maplibreMap.getMarkers().contains(markers.get(0)));
      assertTrue(maplibreMap.getMarkers().contains(markers.get(1)));
    }));
  }

  @Test
  public void testAddMarkersEmpty() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<BaseMarkerOptions> markerList = new ArrayList<>();
      maplibreMap.addMarkers(markerList);
      assertEquals("Markers size should be 0", 0, maplibreMap.getMarkers().size());
    }));
  }

  @Test
  public void testAddMarkersSingleMarker() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<BaseMarkerOptions> markerList = new ArrayList<>();
      MarkerOptions markerOptions = new MarkerOptions().title("a").position(new LatLng());
      markerList.add(markerOptions);
      List<Marker> markers = maplibreMap.addMarkers(markerList);
      assertEquals("Markers size should be 1", 1, maplibreMap.getMarkers().size());
      assertTrue(maplibreMap.getMarkers().contains(markers.get(0)));
    }));
  }

  @Test
  public void testAddPolygon() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      PolygonOptions polygonOptions = new PolygonOptions().add(new LatLng());
      Polygon polygon = maplibreMap.addPolygon(polygonOptions);
      assertTrue("Polygon should be contained", maplibreMap.getPolygons().contains(polygon));
    }));
  }

  @Test
  public void testAddEmptyPolygon() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      PolygonOptions polygonOptions = new PolygonOptions();
      Polygon polygon = maplibreMap.addPolygon(polygonOptions);
      assertTrue("Polygon should be ignored", !maplibreMap.getPolygons().contains(polygon));
    }));
  }

  @Test
  public void testAddPolygons() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<PolygonOptions> polygonList = new ArrayList<>();
      PolygonOptions polygonOptions1 = new PolygonOptions().fillColor(Color.BLACK).add(new LatLng());
      PolygonOptions polygonOptions2 = new PolygonOptions().fillColor(Color.WHITE).add(new LatLng());
      PolygonOptions polygonOptions3 = new PolygonOptions();
      polygonList.add(polygonOptions1);
      polygonList.add(polygonOptions2);
      polygonList.add(polygonOptions3);
      maplibreMap.addPolygons(polygonList);
      assertEquals("Polygons size should be 2", 2, maplibreMap.getPolygons().size());
      assertTrue(maplibreMap.getPolygons().contains(polygonOptions1.getPolygon()));
      assertTrue(maplibreMap.getPolygons().contains(polygonOptions2.getPolygon()));
      assertTrue("Polygon should be ignored", !maplibreMap.getPolygons().contains(polygonOptions3.getPolygon()));
    }));
  }

  @Test
  public void addPolygonsEmpty() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      maplibreMap.addPolygons(new ArrayList<PolygonOptions>());
      assertEquals("Polygons size should be 0", 0, maplibreMap.getPolygons().size());
    }));
  }

  @Test
  public void addPolygonsSingle() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<PolygonOptions> polygonList = new ArrayList<>();
      PolygonOptions polygonOptions = new PolygonOptions().fillColor(Color.BLACK).add(new LatLng());
      polygonList.add(polygonOptions);
      maplibreMap.addPolygons(polygonList);
      assertEquals("Polygons size should be 1", 1, maplibreMap.getPolygons().size());
      assertTrue(maplibreMap.getPolygons().contains(polygonOptions.getPolygon()));
    }));
  }

  @Test
  public void testAddPolyline() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      PolylineOptions polylineOptions = new PolylineOptions().add(new LatLng());
      Polyline polyline = maplibreMap.addPolyline(polylineOptions);
      assertTrue("Polyline should be contained", maplibreMap.getPolylines().contains(polyline));
    }));
  }

  @Test
  public void testAddEmptyPolyline() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      PolylineOptions polylineOptions = new PolylineOptions();
      Polyline polyline = maplibreMap.addPolyline(polylineOptions);
      assertTrue("Polyline should be ignored", !maplibreMap.getPolylines().contains(polyline));
    }));
  }

  @Test
  public void testAddPolylines() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<PolylineOptions> polylineList = new ArrayList<>();
      PolylineOptions polygonOptions1 = new PolylineOptions().color(Color.BLACK).add(new LatLng());
      PolylineOptions polygonOptions2 = new PolylineOptions().color(Color.WHITE).add(new LatLng());
      PolylineOptions polygonOptions3 = new PolylineOptions();
      polylineList.add(polygonOptions1);
      polylineList.add(polygonOptions2);
      polylineList.add(polygonOptions3);
      maplibreMap.addPolylines(polylineList);
      assertEquals("Polygons size should be 2", 2, maplibreMap.getPolylines().size());
      assertTrue(maplibreMap.getPolylines().contains(polygonOptions1.getPolyline()));
      assertTrue(maplibreMap.getPolylines().contains(polygonOptions2.getPolyline()));
      assertTrue(
        "Polyline should be ignored", !maplibreMap.getPolylines().contains(polygonOptions3.getPolyline())
      );
    }));
  }

  @Test
  public void testAddPolylinesEmpty() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      maplibreMap.addPolylines(new ArrayList<PolylineOptions>());
      assertEquals("Polygons size should be 0", 0, maplibreMap.getPolylines().size());
    }));
  }

  @Test
  public void testAddPolylinesSingle() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<PolylineOptions> polylineList = new ArrayList<>();
      PolylineOptions polygonOptions = new PolylineOptions().color(Color.BLACK).add(new LatLng());
      polylineList.add(polygonOptions);
      maplibreMap.addPolylines(polylineList);
      assertEquals("Polygons size should be 1", 1, maplibreMap.getPolylines().size());
      assertTrue(maplibreMap.getPolylines().contains(polygonOptions.getPolyline()));
    }));
  }

  @Test
  public void testRemoveMarker() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker marker = maplibreMap.addMarker(markerOptions);
      maplibreMap.removeMarker(marker);
      assertTrue("Markers should be empty", maplibreMap.getMarkers().isEmpty());
    }));
  }

  @Test
  public void testRemovePolygon() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      PolygonOptions polygonOptions = new PolygonOptions();
      Polygon polygon = maplibreMap.addPolygon(polygonOptions);
      maplibreMap.removePolygon(polygon);
      assertTrue("Polygons should be empty", maplibreMap.getPolylines().isEmpty());
    }));
  }

  @Test
  public void testRemovePolyline() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      PolylineOptions polylineOptions = new PolylineOptions();
      Polyline polyline = maplibreMap.addPolyline(polylineOptions);
      maplibreMap.removePolyline(polyline);
      assertTrue("Polylines should be empty", maplibreMap.getPolylines().isEmpty());
    }));
  }

  @Test
  public void testRemoveAnnotation() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker marker = maplibreMap.addMarker(markerOptions);
      maplibreMap.removeAnnotation(marker);
      assertTrue("Annotations should be empty", maplibreMap.getAnnotations().isEmpty());
    }));
  }

  @Test
  public void testRemoveAnnotationById() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      maplibreMap.addMarker(markerOptions);
      // id will always be 0 in unit tests
      maplibreMap.removeAnnotation(0);
      assertTrue("Annotations should be empty", maplibreMap.getAnnotations().isEmpty());
    }));
  }

  @Test
  public void testRemoveAnnotations() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<BaseMarkerOptions> markerList = new ArrayList<>();
      MarkerOptions markerOptions1 = new MarkerOptions().title("a").position(new LatLng());
      MarkerOptions markerOptions2 = new MarkerOptions().title("b").position(new LatLng());
      markerList.add(markerOptions1);
      markerList.add(markerOptions2);
      maplibreMap.addMarkers(markerList);
      maplibreMap.removeAnnotations();
      assertTrue("Annotations should be empty", maplibreMap.getAnnotations().isEmpty());
    }));
  }

  @Test
  public void testClear() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<BaseMarkerOptions> markerList = new ArrayList<>();
      MarkerOptions markerOptions1 = new MarkerOptions().title("a").position(new LatLng());
      MarkerOptions markerOptions2 = new MarkerOptions().title("b").position(new LatLng());
      markerList.add(markerOptions1);
      markerList.add(markerOptions2);
      maplibreMap.addMarkers(markerList);
      maplibreMap.clear();
      assertTrue("Annotations should be empty", maplibreMap.getAnnotations().isEmpty());
    }));
  }

  @Test
  public void testRemoveAnnotationsByList() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      List<BaseMarkerOptions> markerList = new ArrayList<>();
      MarkerOptions markerOptions1 = new MarkerOptions().title("a").position(new LatLng());
      MarkerOptions markerOptions2 = new MarkerOptions().title("b").position(new LatLng());
      markerList.add(markerOptions1);
      markerList.add(markerOptions2);
      List<Marker> markers = maplibreMap.addMarkers(markerList);
      Marker marker = maplibreMap.addMarker(new MarkerOptions().position(new LatLng()).title("c"));
      maplibreMap.removeAnnotations(markers);
      assertTrue("Annotations should not be empty", maplibreMap.getAnnotations().size() == 1);
      assertTrue("Marker should be contained", maplibreMap.getAnnotations().contains(marker));
    }));
  }

  @Test
  public void testGetAnnotationById() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker initialMarker = maplibreMap.addMarker(markerOptions);
      Marker retrievedMarker = (Marker) maplibreMap.getAnnotation(0);
      assertEquals("Markers should match", initialMarker, retrievedMarker);
    }));
  }

  @Test
  public void testGetAnnotations() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(
      new MapLibreMapAction((uiController, view) ->
        assertNotNull("Annotations should be non null", maplibreMap.getAnnotations()))
    );
  }

  @Test
  public void testGetMarkers() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(
      new MapLibreMapAction((uiController, view) ->
        assertNotNull("Markers should be non null", maplibreMap.getMarkers()))
    );
  }

  @Test
  public void testGetPolygons() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) ->
      assertNotNull("Polygons should be non null", maplibreMap.getPolygons()))
    );
  }

  @Test
  public void testGetPolylines() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) ->
      assertNotNull("Polylines should be non null", maplibreMap.getPolylines()))
    );
  }

  @Test
  public void testGetSelectedMarkers() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) ->
      assertNotNull("Selected markers should be non null", maplibreMap.getSelectedMarkers()))
    );
  }

  @Test
  public void testSelectMarker() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker marker = maplibreMap.addMarker(markerOptions);
      maplibreMap.selectMarker(marker);
      assertTrue("Marker should be contained", maplibreMap.getSelectedMarkers().contains(marker));
    }));
  }

  @Test
  public void testDeselectMarker() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker marker = maplibreMap.addMarker(markerOptions);
      maplibreMap.selectMarker(marker);
      maplibreMap.deselectMarker(marker);
      assertTrue("Selected markers should be empty", maplibreMap.getSelectedMarkers().isEmpty());
    }));
  }

  @Test
  public void testDeselectMarkers() {
    validateTestSetup();
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      MarkerOptions markerOptions = new MarkerOptions().position(new LatLng());
      Marker marker1 = maplibreMap.addMarker(markerOptions);
      Marker marker2 = maplibreMap.addMarker(markerOptions);
      maplibreMap.selectMarker(marker1);
      maplibreMap.selectMarker(marker2);
      maplibreMap.deselectMarkers();
      assertTrue("Selected markers should be empty", maplibreMap.getSelectedMarkers().isEmpty());
    }));
  }

  public class MapLibreMapAction implements ViewAction {

    private InvokeViewAction invokeViewAction;

    MapLibreMapAction(InvokeViewAction invokeViewAction) {
      this.invokeViewAction = invokeViewAction;
    }

    @Override
    public Matcher<View> getConstraints() {
      return isDisplayed();
    }

    @Override
    public String getDescription() {
      return getClass().getSimpleName();
    }

    @Override
    public void perform(UiController uiController, View view) {
      invokeViewAction.onViewAction(uiController, view);
    }
  }

  interface InvokeViewAction {
    void onViewAction(UiController uiController, View view);
  }
}