package com.mapbox.mapboxsdk.testapp.activity.snapshot

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.PointF
import android.os.Bundle
import android.view.MotionEvent
import android.view.View
import android.view.ViewTreeObserver.OnGlobalLayoutListener
import android.widget.ImageView
import androidx.annotation.VisibleForTesting
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.mapboxsdk.camera.CameraPosition
import com.mapbox.mapboxsdk.geometry.LatLng
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.snapshotter.MapSnapshot
import com.mapbox.mapboxsdk.snapshotter.MapSnapshotter
import com.mapbox.mapboxsdk.testapp.R
import timber.log.Timber

/**
 * Test activity showing how to use a the [MapSnapshotter] and overlay
 * [android.graphics.Bitmap]s on top.
 */
class MapSnapshotterBitMapOverlayActivity :
    AppCompatActivity(),
    MapSnapshotter.SnapshotReadyCallback {
    private var mapSnapshotter: MapSnapshotter? = null

    @get:VisibleForTesting
    var mapSnapshot: MapSnapshot? = null
        private set

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_snapshotter_marker)
        val container = findViewById<View>(R.id.container)
        container.viewTreeObserver
            .addOnGlobalLayoutListener(object : OnGlobalLayoutListener {
                override fun onGlobalLayout() {
                    container.viewTreeObserver.removeGlobalOnLayoutListener(this)
                    Timber.i("Starting snapshot")
                    mapSnapshotter = MapSnapshotter(
                        applicationContext,
                        MapSnapshotter.Options(
                            Math.min(container.measuredWidth, 1024),
                            Math.min(container.measuredHeight, 1024)
                        )
                            .withStyleBuilder(
                                Style.Builder().fromUri(Style.getPredefinedStyle("Outdoor"))
                            )
                            .withCameraPosition(
                                CameraPosition.Builder().target(LatLng(52.090737, 5.121420))
                                    .zoom(15.0).build()
                            )
                    )
                    mapSnapshotter!!.start(this@MapSnapshotterBitMapOverlayActivity)
                }
            })
    }

    override fun onStop() {
        super.onStop()
        mapSnapshotter!!.cancel()
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onSnapshotReady(snapshot: MapSnapshot) {
        mapSnapshot = snapshot
        Timber.i("Snapshot ready")
        val imageView = findViewById<View>(R.id.snapshot_image) as ImageView
        val image = addMarker(snapshot)
        imageView.setImageBitmap(image)
        imageView.setOnTouchListener { v: View?, event: MotionEvent ->
            if (event.action == MotionEvent.ACTION_DOWN) {
                val latLng = snapshot.latLngForPixel(PointF(event.x, event.y))
                Timber.e("Clicked LatLng is %s", latLng)
                return@setOnTouchListener true
            }
            false
        }
    }

    private fun addMarker(snapshot: MapSnapshot): Bitmap {
        val canvas = Canvas(snapshot.bitmap)
        val marker =
            BitmapFactory.decodeResource(resources, R.drawable.maplibre_marker_icon_default, null)
        // Dom toren
        val markerLocation = snapshot.pixelForLatLng(LatLng(52.090649433011315, 5.121310651302338))
        canvas.drawBitmap(
            marker, /* Subtract half of the width so we center the bitmap correctly */
            markerLocation.x - marker.width / 2, /* Subtract half of the height so we align the bitmap bottom correctly */
            markerLocation.y - marker.height / 2,
            null
        )
        return snapshot.bitmap
    }
}
