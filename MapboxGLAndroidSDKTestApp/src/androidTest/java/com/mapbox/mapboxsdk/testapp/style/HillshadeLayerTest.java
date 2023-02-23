// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package com.mapbox.mapboxsdk.testapp.style;

import android.graphics.Color;

import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import com.mapbox.geojson.Point;
import com.mapbox.mapboxsdk.maps.BaseLayerTest;
import com.mapbox.mapboxsdk.style.layers.HillshadeLayer;
import com.mapbox.mapboxsdk.style.layers.TransitionOptions;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.List;

import timber.log.Timber;

import static com.mapbox.mapboxsdk.style.layers.Property.HILLSHADE_ILLUMINATION_ANCHOR_MAP;
import static com.mapbox.mapboxsdk.style.layers.Property.NONE;
import static com.mapbox.mapboxsdk.style.layers.Property.VISIBLE;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.hillshadeAccentColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.hillshadeExaggeration;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.hillshadeHighlightColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.hillshadeIlluminationAnchor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.hillshadeIlluminationDirection;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.hillshadeShadowColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.visibility;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * Basic smoke tests for HillshadeLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class HillshadeLayerTest extends BaseLayerTest {

  private HillshadeLayer layer;
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
    layer = new HillshadeLayer("my-layer", "composite");
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
  public void testHillshadeIlluminationDirectionAsConstant() {
    Timber.i("hillshade-illumination-direction");
    assertNotNull(layer);
    assertNull(layer.getHillshadeIlluminationDirection().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(hillshadeIlluminationDirection(propertyValue));
    assertEquals(layer.getHillshadeIlluminationDirection().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHillshadeIlluminationAnchorAsConstant() {
    Timber.i("hillshade-illumination-anchor");
    assertNotNull(layer);
    assertNull(layer.getHillshadeIlluminationAnchor().getValue());

    // Set and Get
    String propertyValue = HILLSHADE_ILLUMINATION_ANCHOR_MAP;
    layer.setProperties(hillshadeIlluminationAnchor(propertyValue));
    assertEquals(layer.getHillshadeIlluminationAnchor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHillshadeExaggerationTransition() {
    Timber.i("hillshade-exaggerationTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHillshadeExaggerationTransition(options);
    assertEquals(layer.getHillshadeExaggerationTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHillshadeExaggerationAsConstant() {
    Timber.i("hillshade-exaggeration");
    assertNotNull(layer);
    assertNull(layer.getHillshadeExaggeration().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(hillshadeExaggeration(propertyValue));
    assertEquals(layer.getHillshadeExaggeration().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHillshadeShadowColorTransition() {
    Timber.i("hillshade-shadow-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHillshadeShadowColorTransition(options);
    assertEquals(layer.getHillshadeShadowColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHillshadeShadowColorAsConstant() {
    Timber.i("hillshade-shadow-color");
    assertNotNull(layer);
    assertNull(layer.getHillshadeShadowColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(hillshadeShadowColor(propertyValue));
    assertEquals(layer.getHillshadeShadowColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHillshadeShadowColorAsIntConstant() {
    Timber.i("hillshade-shadow-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(hillshadeShadowColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getHillshadeShadowColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testHillshadeHighlightColorTransition() {
    Timber.i("hillshade-highlight-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHillshadeHighlightColorTransition(options);
    assertEquals(layer.getHillshadeHighlightColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHillshadeHighlightColorAsConstant() {
    Timber.i("hillshade-highlight-color");
    assertNotNull(layer);
    assertNull(layer.getHillshadeHighlightColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(hillshadeHighlightColor(propertyValue));
    assertEquals(layer.getHillshadeHighlightColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHillshadeHighlightColorAsIntConstant() {
    Timber.i("hillshade-highlight-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(hillshadeHighlightColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getHillshadeHighlightColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testHillshadeAccentColorTransition() {
    Timber.i("hillshade-accent-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setHillshadeAccentColorTransition(options);
    assertEquals(layer.getHillshadeAccentColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testHillshadeAccentColorAsConstant() {
    Timber.i("hillshade-accent-color");
    assertNotNull(layer);
    assertNull(layer.getHillshadeAccentColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(hillshadeAccentColor(propertyValue));
    assertEquals(layer.getHillshadeAccentColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testHillshadeAccentColorAsIntConstant() {
    Timber.i("hillshade-accent-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(hillshadeAccentColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getHillshadeAccentColorAsInt(), Color.argb(127, 255, 127, 0));
  }
}
