package org.maplibre.android.testapp.activity.snapshot

import android.os.Bundle
import android.view.View
import android.view.ViewTreeObserver.OnGlobalLayoutListener
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
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
class MapSnapshotterLocalStyleActivity : AppCompatActivity(), MapSnapshotter.SnapshotReadyCallback {
    private var mapSnapshotter: MapSnapshotter? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_snapshotter_marker)
        val container = findViewById<View>(R.id.container)
        container.viewTreeObserver
            .addOnGlobalLayoutListener(object : OnGlobalLayoutListener {
                override fun onGlobalLayout() {
                    container.viewTreeObserver.removeOnGlobalLayoutListener(this)
                    val styleJson: String
                    styleJson = try {
                        ResourceUtils.readRawResource(
                            this@MapSnapshotterLocalStyleActivity,
                            R.raw.sat_style
                        )
                    } catch (exception: IOException) {
                        throw RuntimeException(exception)
                    }
                    Timber.i("Starting snapshot")
                    mapSnapshotter = MapSnapshotter(
                        applicationContext,
                        MapSnapshotter.Options(
                            Math.min(container.measuredWidth, 1024),
                            Math.min(container.measuredHeight, 1024)
                        )
                            .withStyleBuilder(Style.Builder().fromJson(styleJson))
                            .withCameraPosition(
                                CameraPosition.Builder().target(LatLng(52.090737, 5.121420))
                                    .zoom(18.0).build()
                            )
                    )
                    mapSnapshotter!!.start(
                        this@MapSnapshotterLocalStyleActivity,
                        object : MapSnapshotter.ErrorHandler {
                            override fun onError(error: String) {
                                Timber.e(error)
                            }
                        }
                    )
                }
            })
    }

    override fun onStop() {
        super.onStop()
        mapSnapshotter!!.cancel()
    }

    override fun onSnapshotReady(snapshot: MapSnapshot) {
        Timber.i("Snapshot ready")
        val imageView = findViewById<View>(R.id.snapshot_image) as ImageView
        imageView.setImageBitmap(snapshot.bitmap)
    }
}
