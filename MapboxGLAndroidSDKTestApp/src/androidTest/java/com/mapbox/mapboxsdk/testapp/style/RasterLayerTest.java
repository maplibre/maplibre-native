// This file is generated. Edit scripts/generate-style-code.js, then run `make style-code`.

package com.mapbox.mapboxsdk.testapp.style;

import androidx.test.annotation.UiThreadTest;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;

import com.mapbox.geojson.Point;
import com.mapbox.mapboxsdk.maps.BaseLayerTest;
import com.mapbox.mapboxsdk.style.layers.RasterLayer;
import com.mapbox.mapboxsdk.style.layers.TransitionOptions;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.List;

import timber.log.Timber;

import static com.mapbox.mapboxsdk.style.layers.Property.NONE;
import static com.mapbox.mapboxsdk.style.layers.Property.RASTER_RESAMPLING_LINEAR;
import static com.mapbox.mapboxsdk.style.layers.Property.VISIBLE;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterBrightnessMax;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterBrightnessMin;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterContrast;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterFadeDuration;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterHueRotate;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterOpacity;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterResampling;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.rasterSaturation;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.visibility;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

/**
 * Basic smoke tests for RasterLayer
 */
@RunWith(AndroidJUnit4ClassRunner.class)
public class RasterLayerTest extends BaseLayerTest {

  private RasterLayer layer;
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
    layer = new RasterLayer("my-layer", "composite");
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
  public void testRasterOpacityTransition() {
    Timber.i("raster-opacityTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setRasterOpacityTransition(options);
    assertEquals(layer.getRasterOpacityTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testRasterOpacityAsConstant() {
    Timber.i("raster-opacity");
    assertNotNull(layer);
    assertNull(layer.getRasterOpacity().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterOpacity(propertyValue));
    assertEquals(layer.getRasterOpacity().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterHueRotateTransition() {
    Timber.i("raster-hue-rotateTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setRasterHueRotateTransition(options);
    assertEquals(layer.getRasterHueRotateTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testRasterHueRotateAsConstant() {
    Timber.i("raster-hue-rotate");
    assertNotNull(layer);
    assertNull(layer.getRasterHueRotate().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterHueRotate(propertyValue));
    assertEquals(layer.getRasterHueRotate().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterBrightnessMinTransition() {
    Timber.i("raster-brightness-minTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setRasterBrightnessMinTransition(options);
    assertEquals(layer.getRasterBrightnessMinTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testRasterBrightnessMinAsConstant() {
    Timber.i("raster-brightness-min");
    assertNotNull(layer);
    assertNull(layer.getRasterBrightnessMin().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterBrightnessMin(propertyValue));
    assertEquals(layer.getRasterBrightnessMin().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterBrightnessMaxTransition() {
    Timber.i("raster-brightness-maxTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setRasterBrightnessMaxTransition(options);
    assertEquals(layer.getRasterBrightnessMaxTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testRasterBrightnessMaxAsConstant() {
    Timber.i("raster-brightness-max");
    assertNotNull(layer);
    assertNull(layer.getRasterBrightnessMax().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterBrightnessMax(propertyValue));
    assertEquals(layer.getRasterBrightnessMax().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterSaturationTransition() {
    Timber.i("raster-saturationTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setRasterSaturationTransition(options);
    assertEquals(layer.getRasterSaturationTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testRasterSaturationAsConstant() {
    Timber.i("raster-saturation");
    assertNotNull(layer);
    assertNull(layer.getRasterSaturation().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterSaturation(propertyValue));
    assertEquals(layer.getRasterSaturation().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterContrastTransition() {
    Timber.i("raster-contrastTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setRasterContrastTransition(options);
    assertEquals(layer.getRasterContrastTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testRasterContrastAsConstant() {
    Timber.i("raster-contrast");
    assertNotNull(layer);
    assertNull(layer.getRasterContrast().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterContrast(propertyValue));
    assertEquals(layer.getRasterContrast().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterResamplingAsConstant() {
    Timber.i("raster-resampling");
    assertNotNull(layer);
    assertNull(layer.getRasterResampling().getValue());

    // Set and Get
    String propertyValue = RASTER_RESAMPLING_LINEAR;
    layer.setProperties(rasterResampling(propertyValue));
    assertEquals(layer.getRasterResampling().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testRasterFadeDurationAsConstant() {
    Timber.i("raster-fade-duration");
    assertNotNull(layer);
    assertNull(layer.getRasterFadeDuration().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(rasterFadeDuration(propertyValue));
    assertEquals(layer.getRasterFadeDuration().getValue(), propertyValue);
  }
}
