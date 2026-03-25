package org.maplibre.android.testapp.maps.widgets;

import android.graphics.Color;

import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.camera.CameraUpdateFactory;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.testapp.activity.EspressoTest;

import org.junit.Test;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withTagValue;
import static org.maplibre.android.testapp.action.MapLibreMapAction.invoke;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * Tests for ScaleBarView widget.
 */
public class ScaleBarViewTest extends EspressoTest {

  private static final String SCALE_BAR_TAG = "scaleBarView";

  @Test
  public void testDefaultDisabled() {
    validateTestSetup();
    // Scale bar should be disabled by default
    // Note: The scale bar view is lazily initialized and won't exist until enabled
    assertFalse("Scale bar should be disabled by default",
        maplibreMap.getUiSettings().isScaleBarEnabled());
  }

  @Test
  public void testEnabled() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      uiController.loopMainThreadForAtLeast(300);
    });
    assertTrue("Scale bar should be enabled after setting",
        maplibreMap.getUiSettings().isScaleBarEnabled());
  }

  @Test
  public void testVisibleAtZoom15() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.moveCamera(CameraUpdateFactory.newCameraPosition(
          new CameraPosition.Builder()
              .zoom(15)
              .target(new LatLng(0, 0))
              .build()
      ));
      uiController.loopMainThreadForAtLeast(500);
    });
    onView(withTagValue(is(SCALE_BAR_TAG))).check(matches(isDisplayed()));
  }

  @Test
  public void testMetricSystem() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarIsMetric(true);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertTrue("Scale bar should use metric system",
        maplibreMap.getUiSettings().isScaleBarIsMetric());
  }

  @Test
  public void testImperialSystem() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarIsMetric(false);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertFalse("Scale bar should use imperial system",
        maplibreMap.getUiSettings().isScaleBarIsMetric());
  }

  @Test
  public void testPrimaryColor() {
    validateTestSetup();
    final int testColor = Color.RED;
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarPrimaryColor(testColor);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertEquals("Primary color should match",
        testColor, maplibreMap.getUiSettings().getScaleBarPrimaryColor());
  }

  @Test
  public void testSecondaryColor() {
    validateTestSetup();
    final int testColor = Color.BLUE;
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarSecondaryColor(testColor);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertEquals("Secondary color should match",
        testColor, maplibreMap.getUiSettings().getScaleBarSecondaryColor());
  }

  @Test
  public void testTextColor() {
    validateTestSetup();
    final int testColor = Color.GREEN;
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarTextColor(testColor);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertEquals("Text color should match",
        testColor, maplibreMap.getUiSettings().getScaleBarTextColor());
  }

  @Test
  public void testGravity() {
    validateTestSetup();
    final int testGravity = android.view.Gravity.BOTTOM | android.view.Gravity.END;
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarGravity(testGravity);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertEquals("Gravity should match",
        testGravity, maplibreMap.getUiSettings().getScaleBarGravity());
  }

  @Test
  public void testMargins() {
    validateTestSetup();
    final int left = 10;
    final int top = 20;
    final int right = 30;
    final int bottom = 40;
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarMargins(left, top, right, bottom);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertEquals("Left margin should match", left, maplibreMap.getUiSettings().getScaleBarMarginLeft());
    assertEquals("Top margin should match", top, maplibreMap.getUiSettings().getScaleBarMarginTop());
    assertEquals("Right margin should match", right, maplibreMap.getUiSettings().getScaleBarMarginRight());
    assertEquals("Bottom margin should match", bottom, maplibreMap.getUiSettings().getScaleBarMarginBottom());
  }

  @Test
  public void testDisable() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      uiController.loopMainThreadForAtLeast(200);
      maplibreMap.getUiSettings().setScaleBarEnabled(false);
      uiController.loopMainThreadForAtLeast(300);
    });
    assertFalse("Scale bar should be disabled",
        maplibreMap.getUiSettings().isScaleBarEnabled());
    onView(withTagValue(is(SCALE_BAR_TAG))).check(matches(not(isDisplayed())));
  }

  @Test
  public void testToggleMetricImperial() {
    validateTestSetup();
    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarEnabled(true);
      maplibreMap.getUiSettings().setScaleBarIsMetric(true);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertTrue("Should start with metric", maplibreMap.getUiSettings().isScaleBarIsMetric());

    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarIsMetric(false);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertFalse("Should switch to imperial", maplibreMap.getUiSettings().isScaleBarIsMetric());

    invoke(maplibreMap, (uiController, maplibreMap) -> {
      maplibreMap.getUiSettings().setScaleBarIsMetric(true);
      uiController.loopMainThreadForAtLeast(100);
    });
    assertTrue("Should switch back to metric", maplibreMap.getUiSettings().isScaleBarIsMetric());
  }
}
