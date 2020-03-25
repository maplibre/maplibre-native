package com.mapbox.mapboxsdk.testapp.activity.snapshot;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Bundle;
import android.view.ViewTreeObserver;
import android.widget.GridLayout;
import android.widget.ImageView;

import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.FeatureCollection;
import com.mapbox.geojson.Point;
import com.mapbox.mapboxsdk.camera.CameraPosition;
import com.mapbox.mapboxsdk.constants.MapboxConstants;
import com.mapbox.mapboxsdk.geometry.LatLng;
import com.mapbox.mapboxsdk.geometry.LatLngBounds;
import com.mapbox.mapboxsdk.maps.Style;
import com.mapbox.mapboxsdk.snapshotter.MapSnapshotter;
import com.mapbox.mapboxsdk.style.layers.Property;
import com.mapbox.mapboxsdk.style.layers.RasterLayer;
import com.mapbox.mapboxsdk.style.layers.SymbolLayer;
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource;
import com.mapbox.mapboxsdk.style.sources.RasterSource;
import com.mapbox.mapboxsdk.style.sources.Source;
import com.mapbox.mapboxsdk.testapp.R;
import com.mapbox.mapboxsdk.utils.BitmapUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Random;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import timber.log.Timber;

import static com.mapbox.mapboxsdk.style.expressions.Expression.get;
import static com.mapbox.mapboxsdk.style.expressions.Expression.literal;
import static com.mapbox.mapboxsdk.style.expressions.Expression.switchCase;
import static com.mapbox.mapboxsdk.style.expressions.Expression.toBool;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconAllowOverlap;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconAnchor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconColor;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconIgnorePlacement;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconImage;
import static com.mapbox.mapboxsdk.style.layers.PropertyFactory.iconSize;

/**
 * Test activity showing how to use a the {@link com.mapbox.mapboxsdk.snapshotter.MapSnapshotter}
 */
public class MapSnapshotterActivity extends AppCompatActivity {
  private static final String ID_FEATURE_PROPERTY = "id";
  private static final String SELECTED_FEATURE_PROPERTY = "selected";
  private static final String TITLE_FEATURE_PROPERTY = "title";
  // layer & source constants
  private static final String MARKER_SOURCE = "marker-source";
  private static final String MARKER_LAYER = "marker-layer";

  public GridLayout grid;
  private List<MapSnapshotter> snapshotters = new ArrayList<>();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_map_snapshotter);

    // Find the grid view and start snapshotting as soon
    // as the view is measured
    grid = findViewById(R.id.snapshot_grid);
    grid.getViewTreeObserver()
      .addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
        @Override
        public void onGlobalLayout() {
          //noinspection deprecation
          grid.getViewTreeObserver().removeGlobalOnLayoutListener(this);
          addSnapshots();
        }
      });
  }

  private void addSnapshots() {
    Timber.i("Creating snapshotters");

    for (int row = 0; row < grid.getRowCount(); row++) {
      for (int column = 0; column < grid.getColumnCount(); column++) {
        startSnapShot(row, column);
      }
    }
  }

  private void startSnapShot(final int row, final int column) {
    // Optionally the style
    Style.Builder builder = new Style.Builder()
      .fromUri((column + row) % 2 == 0 ? Style.MAPBOX_STREETS : Style.DARK);

    // Define the dimensions
    MapSnapshotter.Options options = new MapSnapshotter.Options(
      grid.getMeasuredWidth() / grid.getColumnCount(),
      grid.getMeasuredHeight() / grid.getRowCount()
    )
      // Optionally the pixel ratio
      .withPixelRatio(1)
      .withLocalIdeographFontFamily(MapboxConstants.DEFAULT_FONT);

    // Optionally the visible region
    if (row % 2 == 0) {
      options.withRegion(new LatLngBounds.Builder()
        .include(new LatLng(randomInRange(-80, 80), randomInRange(-160, 160)))
        .include(new LatLng(randomInRange(-80, 80), randomInRange(-160, 160)))
        .build()
      );
    }

    // Optionally the camera options
    if (column % 2 == 0) {
      options.withCameraPosition(new CameraPosition.Builder()
        .target(options.getRegion() != null
          ? options.getRegion().getCenter()
          : new LatLng(randomInRange(-80, 80), randomInRange(-160, 160)))
        .bearing(randomInRange(0, 360))
        .tilt(randomInRange(0, 60))
        .zoom(randomInRange(0, 10))
        .padding(1, 1, 1, 1)
        .build()
      );
    }
    if (row == 0 && column == 0) {
      // Add a source
      Source source = new RasterSource("my-raster-source", "mapbox://mapbox.satellite", 512);
      builder.withLayerAbove(new RasterLayer("satellite-layer", "my-raster-source"), "country-label");
      builder.withSource(source);
    } else if (row == 0 && column == 2) {

      Bitmap carBitmap = BitmapUtils.getBitmapFromDrawable(
        getResources().getDrawable(R.drawable.ic_directions_car_black));

      // marker source
      FeatureCollection markerCollection = FeatureCollection.fromFeatures(new Feature[] {
        Feature.fromGeometry(Point.fromLngLat(4.91638, 52.34673), featureProperties("2", "Car"))
      });
      Source markerSource = new GeoJsonSource(MARKER_SOURCE, markerCollection);

      // marker layer
      SymbolLayer markerSymbolLayer = new SymbolLayer(MARKER_LAYER, MARKER_SOURCE)
        .withProperties(
          iconImage(get(TITLE_FEATURE_PROPERTY)),
          iconIgnorePlacement(true),
          iconAllowOverlap(true),
          iconSize(switchCase(toBool(get(SELECTED_FEATURE_PROPERTY)), literal(1.5f), literal(1.0f))),
          iconAnchor(Property.ICON_ANCHOR_BOTTOM),
          iconColor(Color.BLUE)
        );

      builder.withImage("Car", Objects.requireNonNull(carBitmap), false)
        .withSources(markerSource)
        .withLayers(markerSymbolLayer);
      options.withCameraPosition(new CameraPosition.Builder()
        .target(new LatLng(5.537109374999999,
          52.07950600379697))
        .zoom(1)
        .padding(1, 1, 1, 1)
        .build()
      );
    }

    options.withStyleBuilder(builder);
    MapSnapshotter snapshotter = new MapSnapshotter(MapSnapshotterActivity.this, options);
    snapshotter.start(snapshot -> {
      Timber.i("Got the snapshot");
      ImageView imageView = new ImageView(MapSnapshotterActivity.this);
      imageView.setImageBitmap(snapshot.getBitmap());
      grid.addView(
        imageView,
        new GridLayout.LayoutParams(GridLayout.spec(row), GridLayout.spec(column))
      );
    });
    snapshotters.add(snapshotter);
  }

  @Override
  public void onPause() {
    super.onPause();

    // Make sure to stop the snapshotters on pause
    for (MapSnapshotter snapshotter : snapshotters) {
      snapshotter.cancel();
    }
    snapshotters.clear();
  }

  private static Random random = new Random();

  public static float randomInRange(float min, float max) {
    return (random.nextFloat() * (max - min)) + min;
  }

  private JsonObject featureProperties(@NonNull String id, @NonNull String title) {
    JsonObject object = new JsonObject();
    object.add(ID_FEATURE_PROPERTY, new JsonPrimitive(id));
    object.add(TITLE_FEATURE_PROPERTY, new JsonPrimitive(title));
    object.add(SELECTED_FEATURE_PROPERTY, new JsonPrimitive(false));
    return object;
  }
}
