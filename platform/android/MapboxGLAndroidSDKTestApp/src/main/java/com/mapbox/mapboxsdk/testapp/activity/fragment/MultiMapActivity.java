package com.mapbox.mapboxsdk.testapp.activity.fragment;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.FragmentManager;

import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.maps.SupportMapFragment;
import com.mapbox.mapboxsdk.testapp.R;

/**
 * Test Activity showcasing using multiple static map fragments in one layout.
 */
public class MultiMapActivity extends AppCompatActivity {

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_multi_map);

    FragmentManager fragmentManager = getSupportFragmentManager();
    initFragmentStyle(fragmentManager, R.id.map1, Style.getPredefindStyle("Streets"));
    initFragmentStyle(fragmentManager, R.id.map2, Style.getPredefindStyle("Light"));
    initFragmentStyle(fragmentManager, R.id.map3, Style.getPredefindStyle("Satellite Streets"));
    initFragmentStyle(fragmentManager, R.id.map4, Style.getPredefindStyle("Dark"));
  }

  private void initFragmentStyle(FragmentManager fragmentManager, int fragmentId, String styleId) {
    ((SupportMapFragment) fragmentManager.findFragmentById(fragmentId))
      .getMapAsync(mapboxMap -> mapboxMap.setStyle(styleId));
  }
}
