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
import org.maplibre.android.style.layers.BackgroundLayer;

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
 * Basic smoke tests for BackgroundLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class BackgroundLayerTest extends BaseLayerTest {

  private BackgroundLayer layer;
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
    layer = new BackgroundLayer("my-layer");
    setupLayer(layer);
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
  public void testBackgroundColorTransition() {
    Timber.i("background-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setBackgroundColorTransition(options);
    assertEquals(layer.getBackgroundColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testBackgroundColorAsConstant() {
    Timber.i("background-color");
    assertNotNull(layer);
    assertNull(layer.getBackgroundColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(backgroundColor(propertyValue));
    assertEquals(layer.getBackgroundColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testBackgroundColorAsIntConstant() {
    Timber.i("background-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(backgroundColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getBackgroundColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testBackgroundPatternTransition() {
    Timber.i("background-patternTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setBackgroundPatternTransition(options);
    assertEquals(layer.getBackgroundPatternTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testBackgroundPatternAsConstant() {
    Timber.i("background-pattern");
    assertNotNull(layer);
    assertNull(layer.getBackgroundPattern().getValue());

    // Set and Get
    String propertyValue = "pedestrian-polygon";
    layer.setProperties(backgroundPattern(propertyValue));
    assertEquals(layer.getBackgroundPattern().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testBackgroundOpacityTransition() {
    Timber.i("background-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setBackgroundOpacityTransition(options);
    assertEquals(layer.getBackgroundOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testBackgroundOpacityAsConstant() {
    Timber.i("background-opacity");
    assertNotNull(layer);
    assertNull(layer.getBackgroundOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(backgroundOpacity(propertyValue));
    assertEquals(layer.getBackgroundOpacity().getValue(), propertyValue);
  }
}
