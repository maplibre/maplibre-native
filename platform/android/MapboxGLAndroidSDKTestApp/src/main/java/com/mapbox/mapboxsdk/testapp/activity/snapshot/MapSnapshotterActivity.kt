package com.mapbox.mapboxsdk.testapp.activity.snapshot

import android.graphics.Color
import android.os.Bundle
import android.view.ViewTreeObserver.OnGlobalLayoutListener
import android.widget.GridLayout
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import com.google.gson.JsonObject
import com.google.gson.JsonPrimitive
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.Point
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.constants.MapboxConstants
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.geometry.LatLngBounds
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.snapshotter.MapSnapshot
import com.mapbox.mapboxsdk.snapshotter.MapSnapshotter
import com.mapbox.mapboxsdk.style.expressions.Expression
import com.mapbox.mapboxsdk.style.layers.Property
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.style.layers.RasterLayer
import com.mapbox.mapboxsdk.style.layers.SymbolLayer
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.style.sources.RasterSource
import com.mapbox.mapboxsdk.style.sources.Source
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.utils.BitmapUtils
import timber.log.Timber
import java.util.Objects
import java.util.Random

/**
 * Test activity showing how to use a the [com.mapbox.mapboxsdk.snapshotter.MapSnapshotter]
 */
class MapSnapshotterActivity : AppCompatActivity() {
    lateinit var grid: GridLayout
    private val snapshotters: MutableList<MapSnapshotter> = ArrayList()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_snapshotter)

        // Find the grid view and start snapshotting as soon
        // as the view is measured
        grid = findViewById(R.id.snapshot_grid)
        grid.getViewTreeObserver()
            .addOnGlobalLayoutListener(object : OnGlobalLayoutListener {
                override fun onGlobalLayout() {
                    grid.getViewTreeObserver().removeGlobalOnLayoutListener(this)
                    addSnapshots()
                }
            })
    }

    private fun addSnapshots() {
        Timber.i("Creating snapshotters")
        for (row in 0 until grid.rowCount) {
            for (column in 0 until grid.columnCount) {
                startSnapShot(row, column)
            }
        }
    }

    private fun startSnapShot(row: Int, column: Int) {
        // Optionally the style
        val builder = Style.Builder()
            .fromUri(
                Style.getPredefinedStyle(
                    if ((column + row) % 2 == 0) {
                        "Streets"
                    } else {
                        "Pastel"
                    }
                )
            )

        // Define the dimensions
        val options = MapSnapshotter.Options(
            grid.measuredWidth / grid.columnCount,
            grid.measuredHeight / grid.rowCount
        ) // Optionally the pixel ratio
            .withPixelRatio(1f)
            .withLocalIdeographFontFamily(MapboxConstants.DEFAULT_FONT)

        // Optionally the visible region
        if (row % 2 == 0) {
            options.withRegion(
                LatLngBounds.Builder()
                    .include(
                        LatLng(
                            randomInRange(-80f, 80f).toDouble(),
                            randomInRange(-160f, 160f)
                                .toDouble()
                        )
                    )
                    .include(
                        LatLng(
                            randomInRange(-80f, 80f).toDouble(),
                            randomInRange(-160f, 160f)
                                .toDouble()
                        )
                    )
                    .build()
            )
        }

        // Optionally the camera options
        if (column % 2 == 0) {
            options.withCameraPosition(
                CameraPosition.Builder()
                    .target(
                        if (options.region != null) {
                            options.region!!.center
                        } else {
                            LatLng(
                                randomInRange(-80f, 80f).toDouble(),
                                randomInRange(-160f, 160f)
                                    .toDouble()
                            )
                        }
                    )
                    .bearing(randomInRange(0f, 360f).toDouble())
                    .tilt(randomInRange(0f, 60f).toDouble())
                    .zoom(randomInRange(0f, 10f).toDouble())
                    .padding(1.0, 1.0, 1.0, 1.0)
                    .build()
            )
        }
        if (row == 0 && column == 0) {
            // Add a source
            val source: Source =
                RasterSource("my-raster-source", "maptiler://sources/satellite", 512)
            builder.withLayerAbove(
                RasterLayer("satellite-layer", "my-raster-source"),
                "country_1"
            )
            builder.withSource(source)
        } else if (row == 0 && column == 2) {
            val carBitmap = BitmapUtils.getBitmapFromDrawable(
                ResourcesCompat.getDrawable(resources, R.drawable.ic_directions_car_black, null)
            )

            // marker source
            val markerCollection = FeatureCollection.fromFeatures(
                arrayOf(
                    Feature.fromGeometry(
                        Point.fromLngLat(4.91638, 52.35673),
                        featureProperties("1", "Android")
                    ),
                    Feature.fromGeometry(
                        Point.fromLngLat(4.91638, 12.34673),
                        featureProperties("2", "Car")
                    )
                )
            )
            val markerSource: Source = GeoJsonSource(MARKER_SOURCE, markerCollection)

            // marker layer
            val markerSymbolLayer = SymbolLayer(MARKER_LAYER, MARKER_SOURCE)
                .withProperties(
                    PropertyFactory.iconImage(Expression.get(TITLE_FEATURE_PROPERTY)),
                    PropertyFactory.iconIgnorePlacement(true),
                    PropertyFactory.iconAllowOverlap(true),
                    PropertyFactory.iconSize(
                        Expression.switchCase(
                            Expression.toBool(
                                Expression.get(
                                    SELECTED_FEATURE_PROPERTY
                                )
                            ),
                            Expression.literal(1.5f),
                            Expression.literal(1.0f)
                        )
                    ),
                    PropertyFactory.iconAnchor(Property.ICON_ANCHOR_BOTTOM),
                    PropertyFactory.iconColor(Color.BLUE)
                )
            builder.withImage("Car", Objects.requireNonNull(carBitmap!!), false)
                .withSources(markerSource)
                .withLayers(markerSymbolLayer)
            options
                .withRegion(null)
                .withCameraPosition(
                    CameraPosition.Builder()
                        .target(
                            LatLng(
                                5.537109374999999,
                                52.07950600379697
                            )
                        )
                        .zoom(1.0)
                        .padding(1.0, 1.0, 1.0, 1.0)
                        .build()
                )
        }
        options.withStyleBuilder(builder)
        val snapshotter = MapSnapshotter(this@MapSnapshotterActivity, options)

        snapshotter.setObserver(object : MapSnapshotter.Observer {
            override fun onDidFinishLoadingStyle() {
                Timber.i("onDidFinishLoadingStyle")
            }

            override fun onStyleImageMissing(imageName: String) {
                val androidIcon =
                    BitmapUtils.getBitmapFromDrawable(ResourcesCompat.getDrawable(resources, R.drawable.ic_android_2, null))
                snapshotter.addImage(imageName, androidIcon!!, false)
            }
        })

        snapshotter.start(
            object : MapSnapshotter.SnapshotReadyCallback {
                override fun onSnapshotReady(snapshot: MapSnapshot) {
                    Timber.i("Got the snapshot")
                    val imageView = ImageView(this@MapSnapshotterActivity)
                    imageView.setImageBitmap(snapshot.bitmap)
                    grid.addView(
                        imageView,
                        GridLayout.LayoutParams(GridLayout.spec(row), GridLayout.spec(column))
                    )
                }
            }
        )

        snapshotters.add(snapshotter)
    }

    public override fun onPause() {
        super.onPause()

        // Make sure to stop the snapshotters on pause
        for (snapshotter in snapshotters) {
            snapshotter.cancel()
        }
        snapshotters.clear()
    }

    private fun featureProperties(id: String, title: String): JsonObject {
        val `object` = JsonObject()
        `object`.add(ID_FEATURE_PROPERTY, JsonPrimitive(id))
        `object`.add(TITLE_FEATURE_PROPERTY, JsonPrimitive(title))
        `object`.add(SELECTED_FEATURE_PROPERTY, JsonPrimitive(false))
        return `object`
    }

    companion object {
        private const val ID_FEATURE_PROPERTY = "id"
        private const val SELECTED_FEATURE_PROPERTY = "selected"
        private const val TITLE_FEATURE_PROPERTY = "title"

        // layer & source constants
        private const val MARKER_SOURCE = "marker-source"
        private const val MARKER_LAYER = "marker-layer"
        private val random = Random()
        fun randomInRange(min: Float, max: Float): Float {
            return random.nextFloat() * (max - min) + min
        }
    }
}
