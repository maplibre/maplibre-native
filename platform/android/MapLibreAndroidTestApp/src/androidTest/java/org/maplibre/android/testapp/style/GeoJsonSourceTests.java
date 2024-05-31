package org.maplibre.android.testapp.style;

import org.maplibre.geojson.Feature;
import org.maplibre.geojson.FeatureCollection;
import org.maplibre.geojson.Point;
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
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      GeoJsonSource source = null;
      source = new GeoJsonSource("source", FeatureCollection
              .fromJson(ResourceUtils.readRawResource(rule.getActivity(), R.raw.test_feature_collection)));
      maplibreMap.getStyle().addSource(source);
      maplibreMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));
    });
  }

  @Test
  public void testPointGeometry() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      GeoJsonSource source = new GeoJsonSource("source", Point.fromLngLat(0d, 0d));
      maplibreMap.getStyle().addSource(source);
      maplibreMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));
    });
  }

  @Test
  public void testFeatureProperties() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      GeoJsonSource source = null;
      source = new GeoJsonSource("source",
              ResourceUtils.readRawResource(rule.getActivity(), R.raw.test_feature_properties));
      maplibreMap.getStyle().addSource(source);
      maplibreMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));
    });
  }

  @Test
  public void testUpdateCoalescing() {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      GeoJsonSource source = new GeoJsonSource("source");
      maplibreMap.getStyle().addSource(source);
      maplibreMap.getStyle().addLayer(new CircleLayer("layer", source.getId()));

      source.setGeoJson(Point.fromLngLat(0, 0));
      source.setGeoJson(Point.fromLngLat(-25, -25));
      source.setGeoJson(ResourceUtils.readRawResource(rule.getActivity(), R.raw.test_feature_properties));

      source.setGeoJson(Point.fromLngLat(20, 55));
      TestingAsyncUtils.INSTANCE.waitForLayer(uiController, mapView);
      assertEquals(1, maplibreMap.queryRenderedFeatures(
              maplibreMap.getProjection().toScreenLocation(
                      new LatLng(55, 20)), "layer").size());
    });
  }

  @Test
  public void testClearCollectionDuringConversion() {
    // https://github.com/mapbox/mapbox-gl-native/issues/14565
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      for (int j = 0; j < 1000; j++) {
        List<Feature> features = new ArrayList<>();
        for (int i = 0; i < 100; i++) {
          features.add(Feature.fromGeometry(Point.fromLngLat(0, 0)));
        }
        maplibreMap.getStyle().addSource(new GeoJsonSource("source" + j, FeatureCollection.fromFeatures(features)));
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

  protected void testFeatureFromResource(final @RawRes int resource) {
    validateTestSetup();
    MapLibreMapAction.invoke(maplibreMap, (uiController, maplibreMap) -> {
      GeoJsonSource source = new GeoJsonSource("source");
      maplibreMap.getStyle().addSource(source);
      Layer layer = new CircleLayer("layer", source.getId());
      maplibreMap.getStyle().addLayer(layer);

      source.setGeoJson(Feature.fromJson(ResourceUtils.readRawResource(rule.getActivity(), resource)));

      maplibreMap.getStyle().removeLayer(layer);
      maplibreMap.getStyle().removeSource(source);
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
