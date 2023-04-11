package org.maplibre.android.testapp.style;

import com.mapbox.geojson.Feature;
import com.mapbox.geojson.FeatureCollection;
import com.mapbox.geojson.Point;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.style.layers.CircleLayer;
import org.maplibre.android.style.layers.Layer;
import org.maplibre.android.style.sources.GeoJsonSource;
import org.maplibre.android.testapp.R;
import org.maplibre.android.testapp.action.MapLibreMapAction;
import org.maplibre.android.testapp.activity.EspressoTest;
import org.maplibre.android.testapp.utils.ResourceUtils;
import org.maplibre.android.testapp.utils.TestingAsyncUtils;

import org.hamcrest.Matcher;
import org.junit.Test;
import org.junit.runner.RunWith;

import android.view.View;

import java.util.ArrayList;
import java.util.List;

import androidx.annotation.RawRes;
import androidx.test.espresso.ViewAction;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static org.junit.Assert.assertEquals;

/**
 * Tests for {@link GeoJsonSource}
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class GeoJsonSourceTests extends EspressoTest {

  @Test
  public void testFeatureCollection() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, mapboxMap) -> {
      GeoJsonSource source = null;
      source = new GeoJsonSource("source", FeatureCollection
              .fromJson(ResourceUtils.readRawResource(rule.getActivity(), R.raw.test_feature_collection)));
      mapboxMap.getStyle().addSource(source);
      mapboxMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));
    });
  }

  @Test
  public void testPointGeometry() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, mapboxMap) -> {
      GeoJsonSource source = new GeoJsonSource("source", Point.fromLngLat(0d, 0d));
      mapboxMap.getStyle().addSource(source);
      mapboxMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));
    });
  }

  @Test
  public void testFeatureProperties() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, mapboxMap) -> {
      GeoJsonSource source = null;
      source = new GeoJsonSource("source",
              ResourceUtils.readRawResource(rule.getActivity(), R.raw.test_feature_properties));
      mapboxMap.getStyle().addSource(source);
      mapboxMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));
    });
  }

  @Test
  public void testUpdateCoalescing() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, mapboxMap) -> {
      GeoJsonSource source = new GeoJsonSource("source");
      mapboxMap.getStyle().addSource(source);
      mapboxMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));

      source.setGeoJson(Point.fromLngLat(0, 0));
      source.setGeoJson(Point.fromLngLat(-25, -25));
      source.setGeoJson(ResourceUtils.readRawResource(rule.getActivity(), R.raw.test_feature_properties));

      source.setGeoJson(Point.fromLngLat(20, 55));
      TestingAsyncUtils.INSTANCE.waitForLayer(uiController, mapView);
      assertEquals(1, mapboxMap.queryRenderedFeatures(
              mapboxMap.getProjection().toScreenLocation(
                      new LatLng(55, 20)), "layer").size());
    });
  }

  @Test
  public void testClearCollectionDuringConversion() {
    // https://github.com/mapbox/mapbox-gl-native/issues/14565
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, mapboxMap) -> {
      for (int j = 0; j < 1000; j++) {
        List<Feature> features = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
          features.add(Feature.fromGeometry(Point.fromLngLat(0, 0)));
        }
        mapboxMap.getStyle().addSource(new GeoJsonSource("source" + j, FeatureCollection.fromFeatures(features)));
        features.clear();
      }
    });
  }

  @Test
  public void testPointFeature() {
    testFeatureFromResource(R.raw.test_point_feature);
  }

  @Test
  public void testLineStringFeature() {
    testFeatureFromResource(R.raw.test_line_string_feature);
  }

  @Test
  public void testPolygonFeature() {
    testFeatureFromResource(R.raw.test_polygon_feature);
  }

  @Test
  public void testPolygonWithHoleFeature() {
    testFeatureFromResource(R.raw.test_polygon_with_hole_feature);
  }

  @Test
  public void testMultiPointFeature() {
    testFeatureFromResource(R.raw.test_multi_point_feature);
  }

  @Test
  public void testMultiLineStringFeature() {
    testFeatureFromResource(R.raw.test_multi_line_string_feature);
  }

  @Test
  public void testMultiPolygonFeature() {
    testFeatureFromResource(R.raw.test_multi_polygon_feature);
  }

  @Test
  public void testFeatureConcurrency() {
    validateTestSetup();
    MapboxMapAction.invoke(mapboxMap, (uiController, mapboxMap) -> {
      GeoJsonSource source = new GeoJsonSource("source");
      source.setSafeSetGeoJson(true);

      // There has a concurrency between worker threads and main thread,
      // turn GeoJsonSource.safeSetGeoJson could reproduce this issue.
      // Because the crash happened on the worker thread and can't capture it,
      // So I just write this test to demonstrate how it happen.
      // Uncomment next line to reproduce the crash.
      //source.setSafeSetGeoJson(false);

      mapboxMap.getStyle().addSource(source);
      mapboxMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));

      // Create a Feature and set to the source
      Feature feature = Feature.fromGeometry(Point.fromLngLat(20, 55));
      source.setGeoJson(feature);

      // make the feature heavy
      int count = 0;
      for (int i = 0; i < 1000; i++) {
        feature.removeProperty(String.valueOf(i));
        feature.addStringProperty(String.valueOf(i), String.valueOf(count));
        count++;
      }

      // display the map
      TestingAsyncUtils.INSTANCE.waitForLayer(uiController, mapView);
      assertEquals(1, mapboxMap.queryRenderedFeatures(
              mapboxMap.getProjection().toScreenLocation(
                      new LatLng(55, 20)), "layer").size());

      // Update the feature
      feature.addStringProperty("a key", "a value");
      source.setGeoJson(feature);

      // Continue manipulate the feature will lead to concurrency if safeSetGeoJson is off.
      for (int round = 0; round < 1000; ++round) {
        for (int i = 0; i < 1000; i++) {
          feature.removeProperty(String.valueOf(i));
          feature.addStringProperty(String.valueOf(i), String.valueOf(count));
          count++;
        }
      }

      // Should run here if safeSetGeoJson = true
    });
  }

  protected void testFeatureFromResource(final @RawRes int resource) {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, mapboxMap) -> {
      GeoJsonSource source = new GeoJsonSource("source");
      mapboxMap.getStyle().addSource(source);
      Layer layer = new CircleLayer("layer", source.getId());
      mapboxMap.getStyle().addLayer(layer);

      source.setGeoJson(Feature.fromJson(ResourceUtils.readRawResource(rule.getActivity(), resource)));

      mapboxMap.getStyle().removeLayer(layer);
      mapboxMap.getStyle().removeSource(source);
    });
  }

  public abstract class BaseViewAction implements ViewAction {

    @Override
    public Matcher<View> getConstraints() {
      return isDisplayed();
    }

    @Override
    public String getDescription() {
      return getClass().getSimpleName();
    }

  }
}
