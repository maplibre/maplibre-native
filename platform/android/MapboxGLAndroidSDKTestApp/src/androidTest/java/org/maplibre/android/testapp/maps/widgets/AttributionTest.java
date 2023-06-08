package org.maplibre.android.testapp.maps.widgets;

import android.app.Instrumentation;
import android.content.Intent;
import android.net.Uri;
import android.text.Html;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.URLSpan;
import android.view.View;

import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;
import androidx.test.espresso.intent.Intents;

import org.maplibre.android.maps.MapLibreMap;
import org.maplibre.android.style.sources.Source;
import org.maplibre.android.testapp.R;
import org.maplibre.android.testapp.activity.EspressoTest;

import org.hamcrest.Matcher;
import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.intent.Intents.intended;
import static androidx.test.espresso.intent.Intents.intending;
import static androidx.test.espresso.intent.matcher.IntentMatchers.hasAction;
import static androidx.test.espresso.intent.matcher.IntentMatchers.hasData;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withTagValue;
import static androidx.test.espresso.matcher.ViewMatchers.withText;
import static org.hamcrest.CoreMatchers.allOf;
import static org.hamcrest.CoreMatchers.anything;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.core.IsNot.not;

public class AttributionTest extends EspressoTest {

  private URLSpan[] urlSpans;

  @Before
  public void beforeTest() {
    super.beforeTest();
    Intents.init();
  }

  @Test
  public void testDisabled() {
    validateTestSetup();

    // Default
    onView(withTagValue(is("attrView"))).check(matches(isDisplayed()));

    // Disabled
    onView(withTagValue(is("attrView")))
      .perform(new DisableAction(maplibreMap))
      .check(matches(not(isDisplayed())));
  }

  @Test
  @Ignore
  public void testMapboxStreetsMapboxAttributionLink() {
    validateTestSetup();
    if (urlSpans == null) {
      buildUrlSpans();
    }

    // click on View to open dialog
    onView(withTagValue(is("attrView"))).perform(click());
    onView(withText(R.string.maplibre_attributionsDialogTitle)).check(matches(isDisplayed()));

    // test for trigger url intent
    Matcher<Intent> expectedIntent = allOf(hasAction(Intent.ACTION_VIEW), hasData(Uri.parse(urlSpans[0].getURL())));
    intending(expectedIntent).respondWith(new Instrumentation.ActivityResult(0, null));

    // click item and test for url
    onData(anything()).inAdapterView(withId(R.id.select_dialog_listview)).atPosition(0).perform(click());
    intended(expectedIntent);
  }

  @Test
  @Ignore
  public void testMapboxStreetsOpenStreetMapAttributionLink() {
    validateTestSetup();
    if (urlSpans == null) {
      buildUrlSpans();
    }

    // click on View to open dialog
    onView(withTagValue(is("attrView"))).perform(click());
    onView(withText(R.string.maplibre_attributionsDialogTitle)).check(matches(isDisplayed()));

    // test for trigger url intent
    Matcher<Intent> expectedIntent = allOf(hasAction(Intent.ACTION_VIEW), hasData(Uri.parse(urlSpans[1].getURL())));
    intending(expectedIntent).respondWith(new Instrumentation.ActivityResult(0, null));

    // click item and test for url
    onData(anything()).inAdapterView(withId(R.id.select_dialog_listview)).atPosition(1).perform(click());
    intended(expectedIntent);
  }

  @Test
  @Ignore
  public void testImproveMapLink() {
    validateTestSetup();
    if (urlSpans == null) {
      buildUrlSpans();
    }

    // click on View to open dialog
    onView(withTagValue(is("attrView"))).perform(click());
    onView(withText(R.string.maplibre_attributionsDialogTitle)).check(matches(isDisplayed()));

    // test for trigger url intent
    Matcher<Intent> expectedIntent = hasAction(Intent.ACTION_VIEW);
    intending(expectedIntent).respondWith(new Instrumentation.ActivityResult(0, null));

    // click item and test for url
    onData(anything()).inAdapterView(withId(R.id.select_dialog_listview)).atPosition(2).perform(click());
    intended(expectedIntent);
  }

  @After
  public void afterTest() {
    super.afterTest();
    Intents.release();
  }

  private void buildUrlSpans() {
    onView(withId(R.id.mapView)).perform(new MapLibreMapAction((uiController, view) -> {
      for (Source source : maplibreMap.getStyle().getSources()) {
        String attributionSource = source.getAttribution();
        if (!TextUtils.isEmpty(attributionSource)) {
          SpannableStringBuilder htmlBuilder = (SpannableStringBuilder) Html.fromHtml(attributionSource);
          urlSpans = htmlBuilder.getSpans(0, htmlBuilder.length(), URLSpan.class);
        }
      }
    }));
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
      maplibreMap.getUiSettings().setAttributionEnabled(false);
    }
  }

  private class MapLibreMapAction implements ViewAction {

    private InvokeViewAction invokeViewAction;

    MapLibreMapAction(InvokeViewAction invokeViewAction) {
      this.invokeViewAction = invokeViewAction;
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
      invokeViewAction.onViewAction(uiController, view);
    }
  }

  interface InvokeViewAction {
    void onViewAction(UiController uiController, View view);
  }
}