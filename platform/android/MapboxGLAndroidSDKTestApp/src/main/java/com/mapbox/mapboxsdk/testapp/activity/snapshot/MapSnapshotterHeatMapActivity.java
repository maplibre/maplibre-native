package com.mapbox.mapboxsdk.testapp.activity.snapshot;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.view.View;
import android.view.ViewTreeObserver;
import android.widget.ImageView;

import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.snapshotter.MapSnapshot;
import com.mapbox.mapboxsdk.snapshotter.MapSnapshotter;
import com.mapbox.mapboxsdk.style.layers.HeatmapLayer;
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource;
import com.mapbox.mapboxsdk.style.sources.Source;
import com.mapbox.mapboxsdk.testapp.R;

import org.jetbrains.annotations.NotNull;

import java.net.URI;
import java.net.URISyntaxException;

import androidx.appcompat.app.AppCompatActivity;
import timber.log.Timber;

import static com.mapbox.mapboxsdk.style.expressions.Expression.get;
import static com.mapbox.mapboxsdk.style.expressions.Expression.heatmapDensity;
import static com.mapbox.mapboxsdk.style.expressions.Expression.interpolate;
import static com.mapbox.mapboxsdk.style.expressions.Expression.linear;
import static com.mapbox.mapboxsdk.style.expressions.Expression.literal;
import static com.mapbox.mapboxsdk.style.expressions.Expression.rgb;
import static com.mapbox.mapboxsdk.style.expressions.Expression.rgba;
import static com.mapbox.mapboxsdk.style.expressions.Expression.stop;
import static com.mapbox.mapboxsdk.style.expressions.Expression.zoom;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.heatmapColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.heatmapIntensity;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.heatmapOpacity;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.heatmapRadius;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.heatmapWeight;

/**
 * Test activity showing how to use a the {@link MapSnapshotter} and heatmap layer on it.
 */
public class MapSnapshotterHeatMapActivity extends AppCompatActivity implements MapSnapshotter.SnapshotReadyCallback {
  private static final String EARTHQUAKE_SOURCE_URL = "https://www.mapbox.com/mapbox-gl-js/assets/earthquakes.geojson";
  private static final String EARTHQUAKE_SOURCE_ID = "earthquakes";
  private static final String HEATMAP_LAYER_ID = "earthquakes-heat";
  private static final String HEATMAP_LAYER_SOURCE = "earthquakes";
  private MapSnapshotter mapSnapshotter;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_map_snapshotter_marker);

    final View container = findViewById(R.id.container);
    container.getViewTreeObserver()
      .addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
        @Override
        public void onGlobalLayout() {
          //noinspection deprecation
          container.getViewTreeObserver().removeGlobalOnLayoutListener(this);

          Timber.i("Starting snapshot");

          Style.Builder builder = new Style.Builder().fromUri(Style.OUTDOORS)
            .withSource(getEarthquakeSource())
            .withLayerAbove(getHeatmapLayer(), "waterway-label");

          mapSnapshotter = new MapSnapshotter(
            getApplicationContext(),
            new MapSnapshotter
              .Options(container.getMeasuredWidth(), container.getMeasuredHeight())
              .withStyleBuilder(builder)
              .withCameraPosition(new CameraPosition.Builder()
                .target(new LatLng(15, -94))
                .zoom(5)
                .padding(1, 1, 1, 1)
                .build()
              )
          );
          mapSnapshotter.start(MapSnapshotterHeatMapActivity.this);
        }
      });
  }

  @NotNull
  private HeatmapLayer getHeatmapLayer() {
    HeatmapLayer layer = new HeatmapLayer(HEATMAP_LAYER_ID, EARTHQUAKE_SOURCE_ID);
    layer.setMaxZoom(9);
    layer.setSourceLayer(HEATMAP_LAYER_SOURCE);
    layer.setProperties(

      // Color ramp for heatmap.  Domain is 0 (low) to 1 (high).
      // Begin color ramp at 0-stop with a 0-transparency color
      // to create a blur-like effect.
      heatmapColor(
        interpolate(
          linear(), heatmapDensity(),
          literal(0), rgba(33, 102, 172, 0),
          literal(0.2), rgb(103, 169, 207),
          literal(0.4), rgb(209, 229, 240),
          literal(0.6), rgb(253, 219, 199),
          literal(0.8), rgb(239, 138, 98),
          literal(1), rgb(178, 24, 43)
        )
      ),

      // Increase the heatmap weight based on frequency and property magnitude
      heatmapWeight(
        interpolate(
          linear(), get("mag"),
          stop(0, 0),
          stop(6, 1)
        )
      ),

      // Increase the heatmap color weight weight by zoom level
      // heatmap-intensity is a multiplier on top of heatmap-weight
      heatmapIntensity(
        interpolate(
          linear(), zoom(),
          stop(0, 1),
          stop(9, 3)
        )
      ),

      // Adjust the heatmap radius by zoom level
      heatmapRadius(
        interpolate(
          linear(), zoom(),
          stop(0, 2),
          stop(9, 20)
        )
      ),

      // Transition from heatmap to circle layer by zoom level
      heatmapOpacity(
        interpolate(
          linear(), zoom(),
          stop(7, 1),
          stop(9, 0)
        )
      )
    );
    return layer;
  }

  @Override
  protected void onStop() {
    super.onStop();
    mapSnapshotter.cancel();
  }

  @SuppressLint("ClickableViewAccessibility")
  @Override
  public void onSnapshotReady(MapSnapshot snapshot) {
    Timber.i("Snapshot ready");
    ImageView imageView = findViewById(R.id.snapshot_image);
    imageView.setImageBitmap(snapshot.getBitmap());
  }

  private Source getEarthquakeSource() {
    Source source = null;
    try {
      source = new GeoJsonSource(EARTHQUAKE_SOURCE_ID, new URI(EARTHQUAKE_SOURCE_URL));
    } catch (URISyntaxException uriSyntaxException) {
      Timber.e(uriSyntaxException, "That's not an url... ");
    }
    return source;
  }

}
