// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package com.mapbox.mapboxsdk.testapp.style;

import android.graphics.Color;

import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import com.mapbox.geojson.LineString;
import com.mapbox.geojson.MultiLineString;
import com.mapbox.geojson.MultiPoint;
import com.mapbox.geojson.MultiPolygon;
import com.mapbox.geojson.Point;
import com.mapbox.geojson.Polygon;
import com.mapbox.mapboxsdk.maps.BaseLayerTest;
import com.mapbox.mapboxsdk.style.expressions.Expression;
import com.mapbox.mapboxsdk.style.layers.CircleLayer;
import com.mapbox.mapboxsdk.style.layers.TransitionOptions;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import timber.log.Timber;

import static com.mapbox.mapboxsdk.style.expressions.Expression.distance;
import static com.mapbox.mapboxsdk.style.expressions.Expression.eq;
import static com.mapbox.mapboxsdk.style.expressions.Expression.get;
import static com.mapbox.mapboxsdk.style.expressions.Expression.literal;
import static com.mapbox.mapboxsdk.style.expressions.Expression.lt;
import static com.mapbox.mapboxsdk.style.expressions.Expression.number;
import static com.mapbox.mapboxsdk.style.expressions.Expression.toColor;
import static com.mapbox.mapboxsdk.style.expressions.Expression.within;
import static com.mapbox.mapboxsdk.style.layers.Property.CIRCLE_PITCH_ALIGNMENT_MAP;
import static com.mapbox.mapboxsdk.style.layers.Property.CIRCLE_PITCH_SCALE_MAP;
import static com.mapbox.mapboxsdk.style.layers.Property.CIRCLE_TRANSLATE_ANCHOR_MAP;
import static com.mapbox.mapboxsdk.style.layers.Property.NONE;
import static com.mapbox.mapboxsdk.style.layers.Property.VISIBLE;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleBlur;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleOpacity;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circlePitchAlignment;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circlePitchScale;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleRadius;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleSortKey;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleStrokeColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleStrokeOpacity;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleStrokeWidth;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleTranslate;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.circleTranslateAnchor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.visibility;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * Basic smoke tests for CircleLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class CircleLayerTest extends BaseLayerTest {

  private CircleLayer layer;
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
    layer = new CircleLayer("my-layer", "composite");
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
  public void testCircleSortKeyAsConstant() {
    Timber.i("circle-sort-key");
    assertNotNull(layer);
    assertNull(layer.getCircleSortKey().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(circleSortKey(propertyValue));
    assertEquals(layer.getCircleSortKey().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleSortKeyAsExpression() {
    Timber.i("circle-sort-key-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleSortKey().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(circleSortKey(expression));
    assertEquals(layer.getCircleSortKey().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleRadiusTransition() {
    Timber.i("circle-radiusTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleRadiusTransition(options);
    assertEquals(layer.getCircleRadiusTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleRadiusAsConstant() {
    Timber.i("circle-radius");
    assertNotNull(layer);
    assertNull(layer.getCircleRadius().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(circleRadius(propertyValue));
    assertEquals(layer.getCircleRadius().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleRadiusAsExpression() {
    Timber.i("circle-radius-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleRadius().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(circleRadius(expression));
    assertEquals(layer.getCircleRadius().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleColorTransition() {
    Timber.i("circle-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleColorTransition(options);
    assertEquals(layer.getCircleColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleColorAsConstant() {
    Timber.i("circle-color");
    assertNotNull(layer);
    assertNull(layer.getCircleColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(circleColor(propertyValue));
    assertEquals(layer.getCircleColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleColorAsExpression() {
    Timber.i("circle-color-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleColor().getExpression());

    // Set and Get
    Expression expression = toColor(Expression.get("undefined"));
    layer.setProperties(circleColor(expression));
    assertEquals(layer.getCircleColor().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleColorAsIntConstant() {
    Timber.i("circle-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(circleColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getCircleColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testCircleBlurTransition() {
    Timber.i("circle-blurTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleBlurTransition(options);
    assertEquals(layer.getCircleBlurTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleBlurAsConstant() {
    Timber.i("circle-blur");
    assertNotNull(layer);
    assertNull(layer.getCircleBlur().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(circleBlur(propertyValue));
    assertEquals(layer.getCircleBlur().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleBlurAsExpression() {
    Timber.i("circle-blur-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleBlur().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(circleBlur(expression));
    assertEquals(layer.getCircleBlur().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleOpacityTransition() {
    Timber.i("circle-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleOpacityTransition(options);
    assertEquals(layer.getCircleOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleOpacityAsConstant() {
    Timber.i("circle-opacity");
    assertNotNull(layer);
    assertNull(layer.getCircleOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(circleOpacity(propertyValue));
    assertEquals(layer.getCircleOpacity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleOpacityAsExpression() {
    Timber.i("circle-opacity-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleOpacity().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(circleOpacity(expression));
    assertEquals(layer.getCircleOpacity().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleTranslateTransition() {
    Timber.i("circle-translateTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleTranslateTransition(options);
    assertEquals(layer.getCircleTranslateTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleTranslateAsConstant() {
    Timber.i("circle-translate");
    assertNotNull(layer);
    assertNull(layer.getCircleTranslate().getValue());

    // Set and Get
    Float[] propertyValue = new Float[] {0f, 0f};
    layer.setProperties(circleTranslate(propertyValue));
    assertEquals(layer.getCircleTranslate().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleTranslateAnchorAsConstant() {
    Timber.i("circle-translate-anchor");
    assertNotNull(layer);
    assertNull(layer.getCircleTranslateAnchor().getValue());

    // Set and Get
    String propertyValue = CIRCLE_TRANSLATE_ANCHOR_MAP;
    layer.setProperties(circleTranslateAnchor(propertyValue));
    assertEquals(layer.getCircleTranslateAnchor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCirclePitchScaleAsConstant() {
    Timber.i("circle-pitch-scale");
    assertNotNull(layer);
    assertNull(layer.getCirclePitchScale().getValue());

    // Set and Get
    String propertyValue = CIRCLE_PITCH_SCALE_MAP;
    layer.setProperties(circlePitchScale(propertyValue));
    assertEquals(layer.getCirclePitchScale().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCirclePitchAlignmentAsConstant() {
    Timber.i("circle-pitch-alignment");
    assertNotNull(layer);
    assertNull(layer.getCirclePitchAlignment().getValue());

    // Set and Get
    String propertyValue = CIRCLE_PITCH_ALIGNMENT_MAP;
    layer.setProperties(circlePitchAlignment(propertyValue));
    assertEquals(layer.getCirclePitchAlignment().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeWidthTransition() {
    Timber.i("circle-stroke-widthTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleStrokeWidthTransition(options);
    assertEquals(layer.getCircleStrokeWidthTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeWidthAsConstant() {
    Timber.i("circle-stroke-width");
    assertNotNull(layer);
    assertNull(layer.getCircleStrokeWidth().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(circleStrokeWidth(propertyValue));
    assertEquals(layer.getCircleStrokeWidth().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeWidthAsExpression() {
    Timber.i("circle-stroke-width-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleStrokeWidth().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(circleStrokeWidth(expression));
    assertEquals(layer.getCircleStrokeWidth().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeColorTransition() {
    Timber.i("circle-stroke-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleStrokeColorTransition(options);
    assertEquals(layer.getCircleStrokeColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeColorAsConstant() {
    Timber.i("circle-stroke-color");
    assertNotNull(layer);
    assertNull(layer.getCircleStrokeColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(circleStrokeColor(propertyValue));
    assertEquals(layer.getCircleStrokeColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeColorAsExpression() {
    Timber.i("circle-stroke-color-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleStrokeColor().getExpression());

    // Set and Get
    Expression expression = toColor(Expression.get("undefined"));
    layer.setProperties(circleStrokeColor(expression));
    assertEquals(layer.getCircleStrokeColor().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeColorAsIntConstant() {
    Timber.i("circle-stroke-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(circleStrokeColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getCircleStrokeColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeOpacityTransition() {
    Timber.i("circle-stroke-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setCircleStrokeOpacityTransition(options);
    assertEquals(layer.getCircleStrokeOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeOpacityAsConstant() {
    Timber.i("circle-stroke-opacity");
    assertNotNull(layer);
    assertNull(layer.getCircleStrokeOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(circleStrokeOpacity(propertyValue));
    assertEquals(layer.getCircleStrokeOpacity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testCircleStrokeOpacityAsExpression() {
    Timber.i("circle-stroke-opacity-expression");
    assertNotNull(layer);
    assertNull(layer.getCircleStrokeOpacity().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(circleStrokeOpacity(expression));
    assertEquals(layer.getCircleStrokeOpacity().getExpression(), expression);
  }
}
