package com.mapbox.mapboxsdk.testapp.maps.widgets;

import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.testapp.action.WaitAction;
import com.mapbox.mapboxsdk.testapp.activity.EspressoTest;
import com.mapbox.mapboxsdk.testapp.utils.TestConstants;

import org.junit.Ignore;
import org.junit.Test;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withTagValue;
import static com.mapbox.mapboxsdk.testapp.action.MapboxMapAction.invoke;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.junit.Assert.assertEquals;

public class CompassViewTest extends EspressoTest {

  @Test
  public void testDefault() {
    validateTestSetup();
    onView(withTagValue(is("compassView"))).check(matches(not(isDisplayed())));
  }

  @Test
  @Ignore("No explanation given")
  public void testVisible() {
    validateTestSetup();
    invoke(mapboxMap, (uiController, mapboxMap) -> {
      mapboxMap.moveCamera(CameraUpdateFactory.newCameraPosition(
        new CameraPosition.Builder()
          .bearing(45)
          .zoom(1)
          .target(new LatLng())
            .build()
      ));
      uiController.loopMainThreadForAtLeast(500);
    });
    onView(withTagValue(is("compassView"))).check(matches(isDisplayed()));
  }

  @Test
  @Ignore("No explanation given")
  public void testClick() {
    validateTestSetup();
    invoke(mapboxMap, (uiController, mapboxMap) -> mapboxMap.moveCamera(CameraUpdateFactory.newCameraPosition(
      new CameraPosition.Builder()
        .bearing(45)
        .zoom(1)
        .target(new LatLng())
        .build()
    )));
    onView(withTagValue(is("compassView"))).perform(click());
    WaitAction.invoke(500);
    onView(withTagValue(is("compassView"))).check(matches(not(isDisplayed())));
    invoke(mapboxMap, (uiController, mapboxMap) -> {
      CameraPosition cameraPosition = mapboxMap.getCameraPosition();
      assertEquals("Camera bearing should face north, ", 0, cameraPosition.getBearing(), TestConstants.BEARING_DELTA);
    });
  }
}

