package org.maplibre.android.testapp.style;

import android.graphics.Color;
import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner;
import android.view.View;

import org.maplibre.android.style.light.Light;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.FillExtrusionLayer;
import org.maplibre.android.style.layers.TransitionOptions;
import org.maplibre.android.style.light.Position;
import org.maplibre.android.testapp.R;
import org.maplibre.android.testapp.activity.BaseTest;
import org.maplibre.android.testapp.activity.style.FillExtrusionStyleTestActivity;

import timber.log.Timber;

import org.hamcrest.Matcher;
import org.junit.Test;
import org.junit.runner.RunWith;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static org.maplibre.android.style.expressions.Expression.eq;
import static org.maplibre.android.style.layers.Property.ANCHOR_MAP;
import static org.maplibre.android.style.layers.PropertyFactory.fillExtrusionBase;
import static org.maplibre.android.style.layers.PropertyFactory.fillExtrusionColor;
import static org.maplibre.android.style.layers.PropertyFactory.fillExtrusionHeight;
import static org.maplibre.android.style.layers.PropertyFactory.fillExtrusionOpacity;

import static org.maplibre.android.testapp.action.MapLibreMapAction.invoke;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

@RunWith(AndroidJUnit4ClassRunner.class)
public class LightTest extends BaseTest {

  private Light light;

  @Test
  public void testAnchor() {
    validateTestSetup();
    setupLight();
    Timber.i("anchor");
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      light.setAnchor(ANCHOR_MAP);
      assertEquals("Anchor should match", ANCHOR_MAP, light.getAnchor());
    });
  }

  @Test
  public void testPositionTransition() {
    validateTestSetup();
    setupLight();
    Timber.i("positionTransitionOptions");
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      TransitionOptions options = new TransitionOptions(300, 100);
      light.setPositionTransition(options);
      assertEquals("Transition options should match", options, light.getPositionTransition());
    });
  }

  @Test
  public void testPosition() {
    validateTestSetup();
    setupLight();
    Timber.i("position");
    invoke(maplibreMap,(uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      Position position = new Position(1, 2, 3);
      light.setPosition(position);
      assertEquals("Position should match", position, light.getPosition());
    });
  }

  @Test
  public void testColorTransition() {
    validateTestSetup();
    setupLight();
    Timber.i("colorTransitionOptions");
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      TransitionOptions options = new TransitionOptions(300, 100);
      light.setColorTransition(options);
      assertEquals("Transition options should match", options, light.getColorTransition());
    });
  }

  @Test
  public void testColor() {
    validateTestSetup();
    setupLight();
    Timber.i("color");
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      light.setColor("rgba(255,128,0,0.7)");
      assertEquals("Color should match", "rgba(255,128,0,0.7)", light.getColor());
    });
  }

  @Test
  public void testIntensityTransition() {
    validateTestSetup();
    setupLight();
    Timber.i("intensityTransitionOptions");
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      TransitionOptions options = new TransitionOptions(300, 100);
      light.setIntensityTransition(options);
      assertEquals("Transition options should match", options, light.getIntensityTransition());
    });
  }

  @Test
  public void testIntensity() {
    validateTestSetup();
    setupLight();
    Timber.i("intensity");
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      assertNotNull(light);
      // Set and Get
      light.setIntensity(0.3f);
      assertEquals("Intensity should match", 0.3f, light.getIntensity(), 0f);
    });
  }

  private void setupLight() {
    onView(withId(R.id.mapView)).perform(new ViewAction() {
      @Override
      public Matcher<View> getConstraints() {
        return isDisplayed();
      }

      @Override
      public String getDescription() {
        return getClass().getSimpleName();
      }

      @Override
      public void perform(UiController uiController, View view) {
        light = maplibreMap.getStyle().getLight();
        FillExtrusionLayer fillExtrusionLayer = new FillExtrusionLayer("3d-buildings", "composite");
        fillExtrusionLayer.setSourceLayer("building");
        fillExtrusionLayer.setFilter(eq(Expression.get("extrude"), "true"));
        fillExtrusionLayer.setMinZoom(15);
        fillExtrusionLayer.setProperties(
          fillExtrusionColor(Color.LTGRAY),
          fillExtrusionHeight(Expression.get("height")),
          fillExtrusionBase(Expression.get("min_height")),
          fillExtrusionOpacity(0.6f)
        );
        maplibreMap.getStyle().addLayer(fillExtrusionLayer);
      }
    });
  }

  @Override
  protected Class getActivityClass() {
    return FillExtrusionStyleTestActivity.class;
  }
}