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
import org.maplibre.android.style.layers.FillExtrusionLayer;

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
 * Basic smoke tests for FillExtrusionLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class FillExtrusionLayerTest extends BaseLayerTest {

  private FillExtrusionLayer layer;
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
    layer = new FillExtrusionLayer("my-layer", "composite");
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
  public void testFillExtrusionOpacityTransition() {
    Timber.i("fill-extrusion-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillExtrusionOpacityTransition(options);
    assertEquals(layer.getFillExtrusionOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionOpacityAsConstant() {
    Timber.i("fill-extrusion-opacity");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(fillExtrusionOpacity(propertyValue));
    assertEquals(layer.getFillExtrusionOpacity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionColorTransition() {
    Timber.i("fill-extrusion-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillExtrusionColorTransition(options);
    assertEquals(layer.getFillExtrusionColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionColorAsConstant() {
    Timber.i("fill-extrusion-color");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(fillExtrusionColor(propertyValue));
    assertEquals(layer.getFillExtrusionColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionColorAsExpression() {
    Timber.i("fill-extrusion-color-expression");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionColor().getExpression());

    // Set and Get
    Expression expression = toColor(Expression.get("undefined"));
    layer.setProperties(fillExtrusionColor(expression));
    assertEquals(layer.getFillExtrusionColor().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionColorAsIntConstant() {
    Timber.i("fill-extrusion-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(fillExtrusionColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getFillExtrusionColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionTranslateTransition() {
    Timber.i("fill-extrusion-translateTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillExtrusionTranslateTransition(options);
    assertEquals(layer.getFillExtrusionTranslateTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionTranslateAsConstant() {
    Timber.i("fill-extrusion-translate");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionTranslate().getValue());

    // Set and Get
    Float[] propertyValue = new Float[] {0f, 0f};
    layer.setProperties(fillExtrusionTranslate(propertyValue));
    assertEquals(layer.getFillExtrusionTranslate().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionTranslateAnchorAsConstant() {
    Timber.i("fill-extrusion-translate-anchor");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionTranslateAnchor().getValue());

    // Set and Get
    String propertyValue = FILL_EXTRUSION_TRANSLATE_ANCHOR_MAP;
    layer.setProperties(fillExtrusionTranslateAnchor(propertyValue));
    assertEquals(layer.getFillExtrusionTranslateAnchor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionPatternTransition() {
    Timber.i("fill-extrusion-patternTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillExtrusionPatternTransition(options);
    assertEquals(layer.getFillExtrusionPatternTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionPatternAsConstant() {
    Timber.i("fill-extrusion-pattern");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionPattern().getValue());

    // Set and Get
    String propertyValue = "pedestrian-polygon";
    layer.setProperties(fillExtrusionPattern(propertyValue));
    assertEquals(layer.getFillExtrusionPattern().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionPatternAsExpression() {
    Timber.i("fill-extrusion-pattern-expression");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionPattern().getExpression());

    // Set and Get
    Expression expression = image(string(Expression.get("undefined")));
    layer.setProperties(fillExtrusionPattern(expression));
    assertEquals(layer.getFillExtrusionPattern().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionHeightTransition() {
    Timber.i("fill-extrusion-heightTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillExtrusionHeightTransition(options);
    assertEquals(layer.getFillExtrusionHeightTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionHeightAsConstant() {
    Timber.i("fill-extrusion-height");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionHeight().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(fillExtrusionHeight(propertyValue));
    assertEquals(layer.getFillExtrusionHeight().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionHeightAsExpression() {
    Timber.i("fill-extrusion-height-expression");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionHeight().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(fillExtrusionHeight(expression));
    assertEquals(layer.getFillExtrusionHeight().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionBaseTransition() {
    Timber.i("fill-extrusion-baseTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setFillExtrusionBaseTransition(options);
    assertEquals(layer.getFillExtrusionBaseTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionBaseAsConstant() {
    Timber.i("fill-extrusion-base");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionBase().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(fillExtrusionBase(propertyValue));
    assertEquals(layer.getFillExtrusionBase().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionBaseAsExpression() {
    Timber.i("fill-extrusion-base-expression");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionBase().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(fillExtrusionBase(expression));
    assertEquals(layer.getFillExtrusionBase().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testFillExtrusionVerticalGradientAsConstant() {
    Timber.i("fill-extrusion-vertical-gradient");
    assertNotNull(layer);
    assertNull(layer.getFillExtrusionVerticalGradient().getValue());

    // Set and Get
    Boolean propertyValue = true;
    layer.setProperties(fillExtrusionVerticalGradient(propertyValue));
    assertEquals(layer.getFillExtrusionVerticalGradient().getValue(), propertyValue);
  }
}
