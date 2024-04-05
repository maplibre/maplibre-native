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
import org.maplibre.android.style.layers.FillLayer;

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
 * Basic smoke tests for FillLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class FillLayerTest extends BaseLayerTest {

  private FillLayer layer;
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
    layer = new FillLayer("my-layer", "composite");
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
  public void testFillSortKeyAsConstant() {
    Timber.i("fill-sort-key");
    assertNotNull(layer);
    assertNull(layer.getFillSortKey().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(fillSortKey(propertyValue));
    assertEquals(layer.getFillSortKey().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillSortKeyAsExpression() {
    Timber.i("fill-sort-key-expression");
    assertNotNull(layer);
    assertNull(layer.getFillSortKey().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(fillSortKey(expression));
    assertEquals(layer.getFillSortKey().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillAntialiasAsConstant() {
    Timber.i("fill-antialias");
    assertNotNull(layer);
    assertNull(layer.getFillAntialias().getValue());

    // Set and Get
    Boolean propertyValue = true;
    layer.setProperties(fillAntialias(propertyValue));
    assertEquals(layer.getFillAntialias().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillOpacityTransition() {
    Timber.i("fill-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillOpacityTransition(options);
    assertEquals(layer.getFillOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillOpacityAsConstant() {
    Timber.i("fill-opacity");
    assertNotNull(layer);
    assertNull(layer.getFillOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(fillOpacity(propertyValue));
    assertEquals(layer.getFillOpacity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillOpacityAsExpression() {
    Timber.i("fill-opacity-expression");
    assertNotNull(layer);
    assertNull(layer.getFillOpacity().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(fillOpacity(expression));
    assertEquals(layer.getFillOpacity().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillColorTransition() {
    Timber.i("fill-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillColorTransition(options);
    assertEquals(layer.getFillColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillColorAsConstant() {
    Timber.i("fill-color");
    assertNotNull(layer);
    assertNull(layer.getFillColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(fillColor(propertyValue));
    assertEquals(layer.getFillColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillColorAsExpression() {
    Timber.i("fill-color-expression");
    assertNotNull(layer);
    assertNull(layer.getFillColor().getExpression());

    // Set and Get
    Expression expression = toColor(Expression.get("undefined"));
    layer.setProperties(fillColor(expression));
    assertEquals(layer.getFillColor().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillColorAsIntConstant() {
    Timber.i("fill-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(fillColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getFillColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testFillOutlineColorTransition() {
    Timber.i("fill-outline-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillOutlineColorTransition(options);
    assertEquals(layer.getFillOutlineColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillOutlineColorAsConstant() {
    Timber.i("fill-outline-color");
    assertNotNull(layer);
    assertNull(layer.getFillOutlineColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(fillOutlineColor(propertyValue));
    assertEquals(layer.getFillOutlineColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillOutlineColorAsExpression() {
    Timber.i("fill-outline-color-expression");
    assertNotNull(layer);
    assertNull(layer.getFillOutlineColor().getExpression());

    // Set and Get
    Expression expression = toColor(Expression.get("undefined"));
    layer.setProperties(fillOutlineColor(expression));
    assertEquals(layer.getFillOutlineColor().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillOutlineColorAsIntConstant() {
    Timber.i("fill-outline-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(fillOutlineColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getFillOutlineColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testFillTranslateTransition() {
    Timber.i("fill-translateTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillTranslateTransition(options);
    assertEquals(layer.getFillTranslateTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillTranslateAsConstant() {
    Timber.i("fill-translate");
    assertNotNull(layer);
    assertNull(layer.getFillTranslate().getValue());

    // Set and Get
    Float[] propertyValue = new Float[] {0f, 0f};
    layer.setProperties(fillTranslate(propertyValue));
    assertEquals(layer.getFillTranslate().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillTranslateAnchorAsConstant() {
    Timber.i("fill-translate-anchor");
    assertNotNull(layer);
    assertNull(layer.getFillTranslateAnchor().getValue());

    // Set and Get
    String propertyValue = FILL_TRANSLATE_ANCHOR_MAP;
    layer.setProperties(fillTranslateAnchor(propertyValue));
    assertEquals(layer.getFillTranslateAnchor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillPatternTransition() {
    Timber.i("fill-patternTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillPatternTransition(options);
    assertEquals(layer.getFillPatternTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillPatternAsConstant() {
    Timber.i("fill-pattern");
    assertNotNull(layer);
    assertNull(layer.getFillPattern().getValue());

    // Set and Get
    String propertyValue = "pedestrian-polygon";
    layer.setProperties(fillPattern(propertyValue));
    assertEquals(layer.getFillPattern().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillPatternAsExpression() {
    Timber.i("fill-pattern-expression");
    assertNotNull(layer);
    assertNull(layer.getFillPattern().getExpression());

    // Set and Get
    Expression expression = image(string(Expression.get("undefined")));
    layer.setProperties(fillPattern(expression));
    assertEquals(layer.getFillPattern().getExpression(), expression);
  }
}
