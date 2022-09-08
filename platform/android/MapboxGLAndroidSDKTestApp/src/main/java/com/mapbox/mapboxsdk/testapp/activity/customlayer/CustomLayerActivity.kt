package com.mapbox.mapboxsdk.testapp.activity.customlayer;

import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.mapbox.mapboxsdk.camera.CameraUpdateFactory;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.style.layers.CustomLayer;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.mapboxsdk.testapp.model.customlayer.ExampleCustomLayer;

/**
 * Test activity showcasing the Custom Layer API
 * <p>
 * Note: experimental API, do not use.
 * </p>
 */
public class CustomLayerActivity extends AppCompatActivity {

  private MapboxMap mapboxMap;
  private MapView mapView;
  private CustomLayer customLayer;

  private FloatingActionButton fab;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_custom_layer);

    mapView = findViewById(R.id.mapView);
    mapView.onCreate(savedInstanceState);
    mapView.getMapAsync(map -> {
      mapboxMap = map;
      mapboxMap.moveCamera(CameraUpdateFactory.newLatLngZoom(new LatLng(39.91448, -243.60947), 10));
      mapboxMap.setStyle(Style.getPredefinedStyle("Streets"), style -> initFab());
    });
  }

  private void initFab() {
    fab = findViewById(R.id.fab);
    fab.setColorFilter(ContextCompat.getColor(this, R.color.primary));
    fab.setOnClickListener(view -> {
      if (mapboxMap != null) {
        swapCustomLayer();
      }
    });
  }

  private void swapCustomLayer() {
    Style style = mapboxMap.getStyle();
    if (customLayer != null) {
      style.removeLayer(customLayer);
      customLayer = null;
      fab.setImageResource(R.drawable.ic_layers);
    } else {
      customLayer = new CustomLayer("custom",
        ExampleCustomLayer.createContext());
      style.addLayerBelow(customLayer, "building");
      fab.setImageResource(R.drawable.ic_layers_clear);
    }
  }

  private void updateLayer() {
    if (mapboxMap != null) {
      mapboxMap.triggerRepaint();
    }
  }

  @Override
  protected void onStart() {
    super.onStart();
    mapView.onStart();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mapView.onResume();
  }

  @Override
  protected void onPause() {
    super.onPause();
    mapView.onPause();
  }

  @Override
  protected void onStop() {
    super.onStop();
    mapView.onStop();
  }

  @Override
  protected void onSaveInstanceState(Bundle outState) {
    super.onSaveInstanceState(outState);
    mapView.onSaveInstanceState(outState);
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    mapView.onDestroy();
  }

  @Override
  public void onLowMemory() {
    super.onLowMemory();
    mapView.onLowMemory();
  }

  @Override
  public boolean onCreateOptionsMenu(Menu menu) {
    getMenuInflater().inflate(R.menu.menu_custom_layer, menu);
    return true;
  }

  @Override
  public boolean onOptionsItemSelected(MenuItem item) {
    switch (item.getItemId()) {
      case R.id.action_update_layer:
        updateLayer();
        return true;
      case R.id.action_set_color_red:
        ExampleCustomLayer.setColor(1, 0, 0, 1);
        return true;
      case R.id.action_set_color_green:
        ExampleCustomLayer.setColor(0, 1, 0, 1);
        return true;
      case R.id.action_set_color_blue:
        ExampleCustomLayer.setColor(0, 0, 1, 1);
        return true;
      default:
        return super.onOptionsItemSelected(item);
    }
  }
}
