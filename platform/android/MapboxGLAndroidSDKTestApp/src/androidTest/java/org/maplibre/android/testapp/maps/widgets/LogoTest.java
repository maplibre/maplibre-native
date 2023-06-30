package org.maplibre.android.testapp.maps.widgets;

import android.view.View;

import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;

import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.testapp.activity.EspressoTest;

import org.hamcrest.Matcher;
import org.junit.Test;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withTagValue;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;

public class LogoTest extends EspressoTest {

  @Test
  public void testDefault() {
    validateTestSetup();
    onView(withTagValue(is("logoView"))).check(matches(isDisplayed()));
  }

  @Test
  public void testDisabled() {
    validateTestSetup();

    onView(withTagValue(is("logoView")))
            .perform(new DisableAction(maplibreMap))
            .check(matches(not(isDisplayed())));
  }

  private class DisableAction implements ViewAction {

    private MapLibreMap maplibreMap;

    DisableAction(MapLibreMap map) {
      maplibreMap = map;
    }

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
      maplibreMap.getUiSettings().setLogoEnabled(false);
    }
  }
}
