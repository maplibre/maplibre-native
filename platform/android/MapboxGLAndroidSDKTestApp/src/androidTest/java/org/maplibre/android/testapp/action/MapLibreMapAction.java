package org.maplibre.android.testapp.action;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;

import org.maplibre.android.maps.MapLibreMap;

import org.hamcrest.Matcher;

import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.withId;

public class MapLibreMapAction implements ViewAction {

  private OnInvokeActionListener invokeViewAction;
  private MapLibreMap maplibreMap;

  public MapLibreMapAction(OnInvokeActionListener invokeViewAction, MapLibreMap maplibreMap) {
    this.invokeViewAction = invokeViewAction;
    this.maplibreMap = maplibreMap;
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
    invokeViewAction.onInvokeAction(uiController, maplibreMap);
  }

  public static void invoke(MapLibreMap maplibreMap, OnInvokeActionListener invokeViewAction) {
    onView(withId(android.R.id.content)).perform(new MapLibreMapAction(invokeViewAction, maplibreMap));
  }

  public interface OnInvokeActionListener {
    void onInvokeAction(@NonNull UiController uiController, @NonNull MapLibreMap maplibreMap);
  }
}


