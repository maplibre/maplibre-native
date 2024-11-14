package org.maplibre.android.testapp.activity.snapshot

import android.os.Bundle
import android.view.View
import android.view.ViewTreeObserver.OnGlobalLayoutListener
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import okio.ByteString.Companion.readByteString
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.Style
import org.maplibre.android.snapshotter.MapSnapshot
import org.maplibre.android.snapshotter.MapSnapshotter
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.utils.ResourceUtils
import timber.log.Timber
import java.io.IOException
import java.lang.RuntimeException

/**
 * Test activity showing how to use a the MapSnapshotter with a local style
 */
class MapSnapshotterLocalStyleActivity : AppCompatActivity() {
    private lateinit var mapSnapshotter: MapSnapshotter

    companion object {
        private const val LATITUDE = 52.090737
        private const val LONGITUDE = 5.121420
        private const val ZOOM = 2.0
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_snapshotter_marker)
        val container = findViewById<View>(R.id.container)
        container.viewTreeObserver
            .addOnGlobalLayoutListener(object : OnGlobalLayoutListener {
                override fun onGlobalLayout() {
                    container.viewTreeObserver.removeOnGlobalLayoutListener(this)
                    // # --8<-- [start:readStyleJson]
                    val styleJson = resources.openRawResource(R.raw.demotiles).reader().readText()
                    // # --8<-- [end:readStyleJson]
                    Timber.i("Starting snapshot")
                    // # --8<-- [start:createMapSnapshotter]
                    mapSnapshotter = MapSnapshotter(
                        applicationContext,
                        MapSnapshotter.Options(
                            container.measuredWidth.coerceAtMost(1024),
                            container.measuredHeight.coerceAtMost(1024)
                        )
                            .withStyleBuilder(Style.Builder().fromJson(styleJson))
                            .withCameraPosition(
                                CameraPosition.Builder().target(LatLng(LATITUDE, LONGITUDE))
                                    .zoom(ZOOM).build()
                            )
                    )
                    // # --8<-- [end:createMapSnapshotter]
                    // # --8<-- [start:createSnapshot]
                    mapSnapshotter.start({ snapshot ->
                        Timber.i("Snapshot ready")
                        val imageView = findViewById<View>(R.id.snapshot_image) as ImageView
                        imageView.setImageBitmap(snapshot.bitmap)
                    }) { error -> Timber.e(error )}
                    // # --8<-- [end:createSnapshot]
                }
            })
    }

    override fun onStop() {
        super.onStop()
        if (this::mapSnapshotter.isInitialized) {
            mapSnapshotter.cancel()
        }
    }
}
