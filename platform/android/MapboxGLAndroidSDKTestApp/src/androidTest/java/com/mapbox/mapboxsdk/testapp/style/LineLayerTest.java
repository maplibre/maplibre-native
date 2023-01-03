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
import com.mapbox.mapboxsdk.style.layers.LineLayer;
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
import static com.mapbox.mapboxsdk.style.expressions.Expression.image;
import static com.mapbox.mapboxsdk.style.expressions.Expression.literal;
import static com.mapbox.mapboxsdk.style.expressions.Expression.lt;
import static com.mapbox.mapboxsdk.style.expressions.Expression.number;
import static com.mapbox.mapboxsdk.style.expressions.Expression.string;
import static com.mapbox.mapboxsdk.style.expressions.Expression.toColor;
import static com.mapbox.mapboxsdk.style.expressions.Expression.within;
import static com.mapbox.mapboxsdk.style.layers.Property.LINE_CAP_BUTT;
import static com.mapbox.mapboxsdk.style.layers.Property.LINE_JOIN_BEVEL;
import static com.mapbox.mapboxsdk.style.layers.Property.LINE_TRANSLATE_ANCHOR_MAP;
import static com.mapbox.mapboxsdk.style.layers.Property.NONE;
import static com.mapbox.mapboxsdk.style.layers.Property.VISIBLE;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineBlur;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineCap;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineDasharray;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineGapWidth;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineJoin;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineMiterLimit;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineOffset;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineOpacity;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.linePattern;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineRoundLimit;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineSortKey;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineTranslate;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineTranslateAnchor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.lineWidth;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.visibility;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * Basic smoke tests for LineLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class LineLayerTest extends BaseLayerTest {

  private LineLayer layer;
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
    layer = new LineLayer("my-layer", "composite");
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
  public void testLineCapAsConstant() {
    Timber.i("line-cap");
    assertNotNull(layer);
    assertNull(layer.getLineCap().getValue());

    // Set and Get
    String propertyValue = LINE_CAP_BUTT;
    layer.setProperties(lineCap(propertyValue));
    assertEquals(layer.getLineCap().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineJoinAsConstant() {
    Timber.i("line-join");
    assertNotNull(layer);
    assertNull(layer.getLineJoin().getValue());

    // Set and Get
    String propertyValue = LINE_JOIN_BEVEL;
    layer.setProperties(lineJoin(propertyValue));
    assertEquals(layer.getLineJoin().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineJoinAsExpression() {
    Timber.i("line-join-expression");
    assertNotNull(layer);
    assertNull(layer.getLineJoin().getExpression());

    // Set and Get
    Expression expression = string(Expression.get("undefined"));
    layer.setProperties(lineJoin(expression));
    assertEquals(layer.getLineJoin().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineMiterLimitAsConstant() {
    Timber.i("line-miter-limit");
    assertNotNull(layer);
    assertNull(layer.getLineMiterLimit().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineMiterLimit(propertyValue));
    assertEquals(layer.getLineMiterLimit().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineRoundLimitAsConstant() {
    Timber.i("line-round-limit");
    assertNotNull(layer);
    assertNull(layer.getLineRoundLimit().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineRoundLimit(propertyValue));
    assertEquals(layer.getLineRoundLimit().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineSortKeyAsConstant() {
    Timber.i("line-sort-key");
    assertNotNull(layer);
    assertNull(layer.getLineSortKey().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineSortKey(propertyValue));
    assertEquals(layer.getLineSortKey().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineSortKeyAsExpression() {
    Timber.i("line-sort-key-expression");
    assertNotNull(layer);
    assertNull(layer.getLineSortKey().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(lineSortKey(expression));
    assertEquals(layer.getLineSortKey().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineOpacityTransition() {
    Timber.i("line-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineOpacityTransition(options);
    assertEquals(layer.getLineOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineOpacityAsConstant() {
    Timber.i("line-opacity");
    assertNotNull(layer);
    assertNull(layer.getLineOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineOpacity(propertyValue));
    assertEquals(layer.getLineOpacity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineOpacityAsExpression() {
    Timber.i("line-opacity-expression");
    assertNotNull(layer);
    assertNull(layer.getLineOpacity().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(lineOpacity(expression));
    assertEquals(layer.getLineOpacity().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineColorTransition() {
    Timber.i("line-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineColorTransition(options);
    assertEquals(layer.getLineColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineColorAsConstant() {
    Timber.i("line-color");
    assertNotNull(layer);
    assertNull(layer.getLineColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(lineColor(propertyValue));
    assertEquals(layer.getLineColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineColorAsExpression() {
    Timber.i("line-color-expression");
    assertNotNull(layer);
    assertNull(layer.getLineColor().getExpression());

    // Set and Get
    Expression expression = toColor(Expression.get("undefined"));
    layer.setProperties(lineColor(expression));
    assertEquals(layer.getLineColor().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineColorAsIntConstant() {
    Timber.i("line-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(lineColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getLineColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testLineTranslateTransition() {
    Timber.i("line-translateTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineTranslateTransition(options);
    assertEquals(layer.getLineTranslateTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineTranslateAsConstant() {
    Timber.i("line-translate");
    assertNotNull(layer);
    assertNull(layer.getLineTranslate().getValue());

    // Set and Get
    Float[] propertyValue = new Float[] {0f, 0f};
    layer.setProperties(lineTranslate(propertyValue));
    assertEquals(layer.getLineTranslate().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineTranslateAnchorAsConstant() {
    Timber.i("line-translate-anchor");
    assertNotNull(layer);
    assertNull(layer.getLineTranslateAnchor().getValue());

    // Set and Get
    String propertyValue = LINE_TRANSLATE_ANCHOR_MAP;
    layer.setProperties(lineTranslateAnchor(propertyValue));
    assertEquals(layer.getLineTranslateAnchor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineWidthTransition() {
    Timber.i("line-widthTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineWidthTransition(options);
    assertEquals(layer.getLineWidthTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineWidthAsConstant() {
    Timber.i("line-width");
    assertNotNull(layer);
    assertNull(layer.getLineWidth().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineWidth(propertyValue));
    assertEquals(layer.getLineWidth().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineWidthAsExpression() {
    Timber.i("line-width-expression");
    assertNotNull(layer);
    assertNull(layer.getLineWidth().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(lineWidth(expression));
    assertEquals(layer.getLineWidth().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineGapWidthTransition() {
    Timber.i("line-gap-widthTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineGapWidthTransition(options);
    assertEquals(layer.getLineGapWidthTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineGapWidthAsConstant() {
    Timber.i("line-gap-width");
    assertNotNull(layer);
    assertNull(layer.getLineGapWidth().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineGapWidth(propertyValue));
    assertEquals(layer.getLineGapWidth().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineGapWidthAsExpression() {
    Timber.i("line-gap-width-expression");
    assertNotNull(layer);
    assertNull(layer.getLineGapWidth().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(lineGapWidth(expression));
    assertEquals(layer.getLineGapWidth().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineOffsetTransition() {
    Timber.i("line-offsetTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineOffsetTransition(options);
    assertEquals(layer.getLineOffsetTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineOffsetAsConstant() {
    Timber.i("line-offset");
    assertNotNull(layer);
    assertNull(layer.getLineOffset().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineOffset(propertyValue));
    assertEquals(layer.getLineOffset().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineBlurTransition() {
    Timber.i("line-blurTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineBlurTransition(options);
    assertEquals(layer.getLineBlurTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineBlurAsConstant() {
    Timber.i("line-blur");
    assertNotNull(layer);
    assertNull(layer.getLineBlur().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(lineBlur(propertyValue));
    assertEquals(layer.getLineBlur().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLineBlurAsExpression() {
    Timber.i("line-blur-expression");
    assertNotNull(layer);
    assertNull(layer.getLineBlur().getExpression());

    // Set and Get
    Expression expression = number(Expression.get("undefined"));
    layer.setProperties(lineBlur(expression));
    assertEquals(layer.getLineBlur().getExpression(), expression);
  }

  @Test
  @UiThreadTest
  public void testLineDasharrayTransition() {
    Timber.i("line-dasharrayTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLineDasharrayTransition(options);
    assertEquals(layer.getLineDasharrayTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLineDasharrayAsConstant() {
    Timber.i("line-dasharray");
    assertNotNull(layer);
    assertNull(layer.getLineDasharray().getValue());

    // Set and Get
    Float[] propertyValue = new Float[] {};
    layer.setProperties(lineDasharray(propertyValue));
    assertEquals(layer.getLineDasharray().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLinePatternTransition() {
    Timber.i("line-patternTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLinePatternTransition(options);
    assertEquals(layer.getLinePatternTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLinePatternAsConstant() {
    Timber.i("line-pattern");
    assertNotNull(layer);
    assertNull(layer.getLinePattern().getValue());

    // Set and Get
    String propertyValue = "pedestrian-polygon";
    layer.setProperties(linePattern(propertyValue));
    assertEquals(layer.getLinePattern().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLinePatternAsExpression() {
    Timber.i("line-pattern-expression");
    assertNotNull(layer);
    assertNull(layer.getLinePattern().getExpression());

    // Set and Get
    Expression expression = image(string(Expression.get("undefined")));
    layer.setProperties(linePattern(expression));
    assertEquals(layer.getLinePattern().getExpression(), expression);
  }
}
