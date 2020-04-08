// This file is generated. Edit android/platform/scripts/generate-style-code.js, then run `make android-style-code`.

package com.mapbox.mapboxsdk.testapp.style;

import android.graphics.Color;
import androidx.test.annotation.UiThreadTest;
import androidx.test.ext.junit.runners.AndroidJUnit4;

import com.mapbox.mapboxsdk.maps.BaseLayerTest;
import org.junit.Before;
import timber.log.Timber;

import com.mapbox.mapboxsdk.style.expressions.Expression;
import com.mapbox.mapboxsdk.style.layers.LocationIndicatorLayer;

import org.junit.Test;
import org.junit.runner.RunWith;

import static com.mapbox.mapboxsdk.style.expressions.Expression.*;
import static org.junit.Assert.*;
import static com.mapbox.mapboxsdk.style.layers.Property.*;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.*;

import com.mapbox.mapboxsdk.style.layers.TransitionOptions;

/**
 * Basic smoke tests for LocationIndicatorLayer
 */
@RunWith(AndroidJUnit4.class)
public class LocationIndicatorLayerTest extends BaseLayerTest {

  private LocationIndicatorLayer layer;

  @Before
  @UiThreadTest
  public void beforeTest(){
    super.before();
    layer = new LocationIndicatorLayer("my-layer");
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
  public void testTopImageAsConstant() {
    Timber.i("top-image");
    assertNotNull(layer);
    assertNull(layer.getTopImage().getValue());

    // Set and Get
    String propertyValue = "undefined";
    layer.setProperties(topImage(propertyValue));
    assertEquals(layer.getTopImage().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testBearingImageAsConstant() {
    Timber.i("bearing-image");
    assertNotNull(layer);
    assertNull(layer.getBearingImage().getValue());

    // Set and Get
    String propertyValue = "undefined";
    layer.setProperties(bearingImage(propertyValue));
    assertEquals(layer.getBearingImage().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testShadowImageAsConstant() {
    Timber.i("shadow-image");
    assertNotNull(layer);
    assertNull(layer.getShadowImage().getValue());

    // Set and Get
    String propertyValue = "undefined";
    layer.setProperties(shadowImage(propertyValue));
    assertEquals(layer.getShadowImage().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testPerspectiveCompensationAsConstant() {
    Timber.i("perspective-compensation");
    assertNotNull(layer);
    assertNull(layer.getPerspectiveCompensation().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(perspectiveCompensation(propertyValue));
    assertEquals(layer.getPerspectiveCompensation().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testImageTiltDisplacementAsConstant() {
    Timber.i("image-tilt-displacement");
    assertNotNull(layer);
    assertNull(layer.getImageTiltDisplacement().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(imageTiltDisplacement(propertyValue));
    assertEquals(layer.getImageTiltDisplacement().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testBearingAsConstant() {
    Timber.i("bearing");
    assertNotNull(layer);
    assertNull(layer.getBearing().getValue());

    // Set and Get
    Double propertyValue = 0.3;
    layer.setProperties(bearing(propertyValue));
    assertEquals(layer.getBearing().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testLocationTransition() {
    Timber.i("locationTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setLocationTransition(options);
    assertEquals(layer.getLocationTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testLocationAsConstant() {
    Timber.i("location");
    assertNotNull(layer);
    assertNull(layer.getLocation().getValue());

    // Set and Get
    Double[] propertyValue = new Double[] {0.0, 0.0, 0.0};
    layer.setProperties(location(propertyValue));
    assertEquals(layer.getLocation().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusTransition() {
    Timber.i("accuracy-radiusTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setAccuracyRadiusTransition(options);
    assertEquals(layer.getAccuracyRadiusTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusAsConstant() {
    Timber.i("accuracy-radius");
    assertNotNull(layer);
    assertNull(layer.getAccuracyRadius().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(accuracyRadius(propertyValue));
    assertEquals(layer.getAccuracyRadius().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testTopImageSizeTransition() {
    Timber.i("top-image-sizeTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setTopImageSizeTransition(options);
    assertEquals(layer.getTopImageSizeTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testTopImageSizeAsConstant() {
    Timber.i("top-image-size");
    assertNotNull(layer);
    assertNull(layer.getTopImageSize().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(topImageSize(propertyValue));
    assertEquals(layer.getTopImageSize().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testBearingImageSizeTransition() {
    Timber.i("bearing-image-sizeTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setBearingImageSizeTransition(options);
    assertEquals(layer.getBearingImageSizeTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testBearingImageSizeAsConstant() {
    Timber.i("bearing-image-size");
    assertNotNull(layer);
    assertNull(layer.getBearingImageSize().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(bearingImageSize(propertyValue));
    assertEquals(layer.getBearingImageSize().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testShadowImageSizeTransition() {
    Timber.i("shadow-image-sizeTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setShadowImageSizeTransition(options);
    assertEquals(layer.getShadowImageSizeTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testShadowImageSizeAsConstant() {
    Timber.i("shadow-image-size");
    assertNotNull(layer);
    assertNull(layer.getShadowImageSize().getValue());

    // Set and Get
    Float propertyValue = 0.3f;
    layer.setProperties(shadowImageSize(propertyValue));
    assertEquals(layer.getShadowImageSize().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusColorTransition() {
    Timber.i("accuracy-radius-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setAccuracyRadiusColorTransition(options);
    assertEquals(layer.getAccuracyRadiusColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusColorAsConstant() {
    Timber.i("accuracy-radius-color");
    assertNotNull(layer);
    assertNull(layer.getAccuracyRadiusColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(accuracyRadiusColor(propertyValue));
    assertEquals(layer.getAccuracyRadiusColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusColorAsIntConstant() {
    Timber.i("accuracy-radius-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(accuracyRadiusColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getAccuracyRadiusColorAsInt(), Color.argb(127, 255, 127, 0));
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusBorderColorTransition() {
    Timber.i("accuracy-radius-border-colorTransitionOptions");
    assertNotNull(layer);

    // Set and Get
    TransitionOptions options = new TransitionOptions(300, 100);
    layer.setAccuracyRadiusBorderColorTransition(options);
    assertEquals(layer.getAccuracyRadiusBorderColorTransition(), options);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusBorderColorAsConstant() {
    Timber.i("accuracy-radius-border-color");
    assertNotNull(layer);
    assertNull(layer.getAccuracyRadiusBorderColor().getValue());

    // Set and Get
    String propertyValue = "rgba(255,128,0,0.7)";
    layer.setProperties(accuracyRadiusBorderColor(propertyValue));
    assertEquals(layer.getAccuracyRadiusBorderColor().getValue(), propertyValue);
  }

  @Test
  @UiThreadTest
  public void testAccuracyRadiusBorderColorAsIntConstant() {
    Timber.i("accuracy-radius-border-color");
    assertNotNull(layer);

    // Set and Get
    layer.setProperties(accuracyRadiusBorderColor(Color.argb(127, 255, 127, 0)));
    assertEquals(layer.getAccuracyRadiusBorderColorAsInt(), Color.argb(127, 255, 127, 0));
  }
}
