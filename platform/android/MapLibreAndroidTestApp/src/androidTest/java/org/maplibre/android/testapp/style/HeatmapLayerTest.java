// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package org.maplibre.android.testapp.style;

import android.graphics.Color;
import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import org.maplibre.geojson.LineString;
import org.maplibre.geojson.MultiLineString;
import org.maplibre.geojson.MultiPoint;
import org.maplibre.geojson.MultiPolygon;
import org.maplibre.geojson.Point;
import org.maplibre.geojson.Polygon;
import org.maplibre.android.maps.BaseLayerTest;
import org.junit.Before;
import timber.log.Timber;

import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.HeatmapLayer;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.maplibre.android.style.expressions.Expression.*;
import static org.junit.Assert.*;
import static org.maplibre.android.style.layers.Property.*;
import static org.maplibre.android.style.layers.PropertyFactory.*;

import org.maplibre.android.style.layers.TransitionOptions;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Basic smoke tests for HeatmapLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class HeatmapLayerTest extends BaseLayerTest {

  private HeatmapLayer layer;
  private final List<Point> pointsList = new ArrayList<Point>() {
    {
      add(Point.fromLngLat(55.30122473231012, 25.26476622289597));
      add(Point.fromLngLat(55.29743486255916, 25.25827212207261));
      add(Point.fromLngLat(55.28978863411328, 25.251356725509737));
      add(Point.fromLngLat(55.300027931336984, 25.246425506635504));
      add(Point.fromLngLat(55.307474692951274, 25.244200378933655));
      add(Point.fromLngLat(55.31212891895635, 25.256408010450187));
      add(Point.fromLngLat(55.30774064871093, 25.26266169122738));
      add(Point.fromLngLat(55.301357710197806, 25.264946609615492));
      add(Point.fromLngLat(55.30122473231012, 25.26476622289597));
    }
  };

  @Before
  @UiThreadTest
  public void beforeTest(){
    super.before();
    layer = new HeatmapLayer("my-layer", "composite");
    layer.setSourceLayer("composite");
    setupLayer(layer);
  }

  @Test
  @UiThreadTest
  public void testSourceId() {
    Timber.i("SourceId");
    assertNotNull(layer);
    assertEquals(layer.getSourceId(), "composite");
  }

  @Test
  @UiThreadTest
  public void testSetVisibility() {
    Timber.i("Visibility");
    assertNotNull(layer);

    // Get initial
    assertEquals(layer.getVisibility().getValue(), VISIBLE);

    // Set
    layer.setProperties(visibility(NONE));
    assertEquals(layer.getVisibility().getValue(), NONE);
  }

  @Test
  @UiThreadTest
  public void testSourceLayer() {
    Timber.i("SourceLayer");
    assertNotNull(layer);

    // Get initial
    assertEquals(layer.getSourceLayer(), "composite");

    // Set
    final String sourceLayer = "test";
    layer.setSourceLayer(sourceLayer);
    assertEquals(layer.getSourceLayer(), sourceLayer);
  }

  @Test
  @UiThreadTest
  public void testFilter() {
    Timber.i("Filter");
    assertNotNull(layer);

    // Get initial
    assertEquals(layer.getFilter(), null);

    // Set
    Expression filter = eq(get("undefined"), literal(1.0));
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());

    // Set constant
    filter = literal(true);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());
  }

  @Test
  @UiThreadTest
  public void testFilterDistance() {
    Timber.i("FilterDistance");
    assertNotNull(layer);

    // Get initial
    assertEquals(layer.getFilter(), null);

    // distance with Point
    Expression filter = lt(distance(Point.fromLngLat(1.0, 1.0)), 50);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());

    // distance with LineString
    filter = lt(distance(LineString.fromLngLats(pointsList)), 50);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());

    // distance with MultiPoint
    filter = lt(distance(MultiPoint.fromLngLats(pointsList)), 50);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());

    // distance with MultiPoint
    filter = lt(distance(MultiLineString.fromLngLats(Collections.singletonList(pointsList))), 50);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());

    // distance with Polygon
    filter = lt(distance(Polygon.fromLngLats(Collections.singletonList(pointsList))), 50);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());

    // distance with MultiPolygon
    filter = lt(distance(MultiPolygon.fromLngLats(Collections
      .singletonList(Collections.singletonList(pointsList)))), 50);
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());
  }

  @Test
  @UiThreadTest
  public void testFilterWithin() {
    Timber.i("FilterWithin");
    assertNotNull(layer);

    // Get initial
    assertEquals(layer.getFilter(), null);

    Expression filter = within(Polygon.fromLngLats(Collections.singletonList(pointsList)));
    layer.setFilter(filter);
    assertEquals(layer.getFilter().toString(), filter.toString());
  }


  @Test
  @UiThreadTest
  public void testHeatmapRadiusTransition() {
    Timber.i("heatmap-radiusTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHeatmapRadiusTransition(options);
    assertEquals(layer.getHeatmapRadiusTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHeatmapRadiusAsConstant() {
    Timber.i("heatmap-radius");
    assertNotNull(layer);
    assertNull(layer.getHeatmapRadius().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(heatmapRadius(propertyValue));
    assertEquals(layer.getHeatmapRadius().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHeatmapRadiusAsExpression() {
    Timber.i("heatmap-radius-expression");
    assertNotNull(layer);
    assertNull(layer.getHeatmapRadius().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(heatmapRadius(expression));
    assertEquals(layer.getHeatmapRadius().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testHeatmapWeightAsConstant() {
    Timber.i("heatmap-weight");
    assertNotNull(layer);
    assertNull(layer.getHeatmapWeight().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(heatmapWeight(propertyValue));
    assertEquals(layer.getHeatmapWeight().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHeatmapWeightAsExpression() {
    Timber.i("heatmap-weight-expression");
    assertNotNull(layer);
    assertNull(layer.getHeatmapWeight().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(heatmapWeight(expression));
    assertEquals(layer.getHeatmapWeight().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testHeatmapIntensityTransition() {
    Timber.i("heatmap-intensityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHeatmapIntensityTransition(options);
    assertEquals(layer.getHeatmapIntensityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHeatmapIntensityAsConstant() {
    Timber.i("heatmap-intensity");
    assertNotNull(layer);
    assertNull(layer.getHeatmapIntensity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(heatmapIntensity(propertyValue));
    assertEquals(layer.getHeatmapIntensity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHeatmapOpacityTransition() {
    Timber.i("heatmap-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHeatmapOpacityTransition(options);
    assertEquals(layer.getHeatmapOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHeatmapOpacityAsConstant() {
    Timber.i("heatmap-opacity");
    assertNotNull(layer);
    assertNull(layer.getHeatmapOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(heatmapOpacity(propertyValue));
    assertEquals(layer.getHeatmapOpacity().getValue(), propertyValue);
  }
}
