package com.mapbox.mapboxsdk.testapp.activity.style;

import android.graphics.Bitmap;
import android.os.AsyncTask;
import android.os.Bundle;

import com.mapbox.geojson.Feature;
import com.mapbox.geojson.FeatureCollection;
import com.mapbox.geojson.Point;
import com.mapbox.mapboxsdk.maps.ImageContent;
import com.mapbox.mapboxsdk.maps.ImageStretches;
import com.mapbox.mapboxsdk.maps.MapView;
import com.mapbox.mapboxsdk.maps.MapboxMap;
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.style.layers.SymbolLayer;
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.mapboxsdk.testapp.utils.GeoParseUtil;
import com.mapbox.mapboxsdk.utils.BitmapUtils;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import timber.log.Timber;

import static com.mapbox.mapboxsdk.style.expressions.Expression.get;
import static com.mapbox.mapboxsdk.style.layers.Property.ICON_TEXT_FIT_BOTH;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconAllowOverlap;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconImage;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconTextFit;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.textAllowOverlap;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.textField;

/**
 * Test stretchable image as a background for text..
 */
public class StretchableImageActivity extends AppCompatActivity implements OnMapReadyCallback {
  private static final String NAME_POPUP = "popup";
  private static final String NAME_POPUP_DEBUG = "popup-debug";
  private static final String STRETCH_SOURCE = "STRETCH_SOURCE";
  private static final String STRETCH_LAYER = "STRETCH_LAYER";
  private static final String ORIGINAL_SOURCE = "ORIGINAL_SOURCE";
  private static final String ORIGINAL_LAYER = "ORIGINAL_LAYER";
  private MapboxMap mapboxMap;
  private MapView mapView;

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_stretchable_image);
    mapView = findViewById(R.id.mapView);
    mapView.onCreate(savedInstanceState);
    mapView.getMapAsync(this);
  }

  @Override
  public void onMapReady(@NonNull MapboxMap mapboxMap) {
    this.mapboxMap = mapboxMap;
    mapboxMap.setStyle(Style.MAPBOX_STREETS, style -> {
      Bitmap popup = BitmapUtils.getBitmapFromDrawable(getResources().getDrawable(R.drawable.popup));
      Bitmap popupDebug = BitmapUtils.getBitmapFromDrawable(getResources().getDrawable(R.drawable.popup_debug));

      // The two (blue) columns of pixels that can be stretched horizontally:
      //   - the pixels between x: 25 and x: 55 can be stretched
      //   - the pixels between x: 85 and x: 115 can be stretched.
      List<ImageStretches> stretchX = new ArrayList<>();
      stretchX.add(new ImageStretches(25, 55));
      stretchX.add(new ImageStretches(85, 115));

      // The one (red) row of pixels that can be stretched vertically:
      //   - the pixels between y: 25 and y: 100 can be stretched
      List<ImageStretches> stretchY = new ArrayList<>();
      stretchY.add(new ImageStretches(25, 100));

      // This part of the image that can contain text ([x1, y1, x2, y2]):
      ImageContent content = new ImageContent(25, 25, 115, 100);

      style.addImage(NAME_POPUP, popup, stretchX, stretchY, content);
      style.addImage(NAME_POPUP_DEBUG, popupDebug, stretchX, stretchY, content);
      new LoadFeatureTask(StretchableImageActivity.this).execute();
    });
  }

  private void onFeatureLoaded(String json) {
    if (json == null) {
      Timber.e("json is null.");
      return;
    }

    Style style = mapboxMap.getStyle();
    if (style != null) {
      FeatureCollection featureCollection = FeatureCollection.fromJson(json);
      GeoJsonSource stretchSource = new GeoJsonSource(STRETCH_SOURCE, featureCollection);
      SymbolLayer stretchLayer = new SymbolLayer(STRETCH_LAYER, STRETCH_SOURCE)
        .withProperties(
          textField(get("name")),
          iconImage(get("image-name")),
          iconAllowOverlap(true),
          textAllowOverlap(true),
          iconTextFit(ICON_TEXT_FIT_BOTH));

      // the original, unstretched image for comparison
      Point point = Point.fromLngLat(-70, 0);
      Feature feature = Feature.fromGeometry(point);
      FeatureCollection originalCollection = FeatureCollection.fromFeature(feature);
      GeoJsonSource originalSource = new GeoJsonSource(ORIGINAL_SOURCE, originalCollection);
      SymbolLayer originalLayer = new SymbolLayer(ORIGINAL_LAYER, ORIGINAL_SOURCE);

      style.addSource(stretchSource);
      style.addSource(originalSource);
      style.addLayer(stretchLayer);
      style.addLayer(originalLayer);
    }
  }

  private static class LoadFeatureTask extends AsyncTask<Void, Integer, String> {

    private WeakReference<StretchableImageActivity> activity;

    private LoadFeatureTask(StretchableImageActivity activity) {
      this.activity = new WeakReference<>(activity);
    }

    @Override
    protected String doInBackground(Void... params) {
      StretchableImageActivity activity = this.activity.get();
      if (activity != null) {
        String json = null;
        try {
          json = GeoParseUtil.loadStringFromAssets(activity.getApplicationContext(), "stretchable_image.geojson");
        } catch (IOException exception) {
          Timber.e(exception, "Could not read feature");
        }

        return json;
      }
      return null;
    }

    @Override
    protected void onPostExecute(String json) {
      super.onPostExecute(json);
      StretchableImageActivity activity = this.activity.get();
      if (activity != null) {
        activity.onFeatureLoaded(json);
      }
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
  public void onSaveInstanceState(Bundle outState) {
    super.onSaveInstanceState(outState);
    mapView.onSaveInstanceState(outState);
  }

  @Override
  public void onLowMemory() {
    super.onLowMemory();
    mapView.onLowMemory();
  }

  @Override
  public void onDestroy() {
    super.onDestroy();
    mapView.onDestroy();
  }

}
