package org.maplibre.android.testapp.activity.camera

import android.annotation.SuppressLint
import android.content.DialogInterface
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import android.widget.TextView
import android.widget.Toast
import androidx.annotation.IdRes
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.widget.Toolbar
import androidx.core.content.ContextCompat
import androidx.fragment.app.FragmentActivity
import com.google.android.material.floatingactionbutton.FloatingActionButton
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.constants.GeometryConstants
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.*
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import timber.log.Timber

/** Test activity showcasing how to listen to camera change events. */
class CameraPositionActivity : FragmentActivity(), OnMapReadyCallback, View.OnClickListener, OnMapLongClickListener {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var fab: FloatingActionButton
    private var logCameraChanges = false
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_camera_position)
        val toolbar = findViewById<Toolbar>(R.id.toolbar)
        toolbar.setTitle(R.string.activity_camera_position)
        toolbar.setNavigationIcon(R.drawable.ic_ab_back)
        toolbar.setNavigationOnClickListener { v: View? -> finish() }
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(map: MapLibreMap) {
        maplibreMap = map
        map.setStyle(TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid")) { style: Style? ->
            // add a listener to FAB
            fab = findViewById(R.id.fab)
            fab.setColorFilter(ContextCompat.getColor(this@CameraPositionActivity, R.color.primary))
            fab.setOnClickListener(this)
            toggleLogCameraChanges()

            // listen to long click events to toggle logging camera changes
            maplibreMap.addOnMapLongClickListener(this)
        }
    }

    override fun onMapLongClick(point: LatLng): Boolean {
        toggleLogCameraChanges()
        return false
    }

    @SuppressLint("InflateParams")
    override fun onClick(view: View) {
        val context = view.context
        val dialogContent = LayoutInflater.from(context).inflate(R.layout.dialog_camera_position, null)
        val builder = AlertDialog.Builder(context)
        builder.setTitle(R.string.dialog_camera_position)
        builder.setView(onInflateDialogContent(dialogContent))
        builder.setPositiveButton("Animate", DialogClickListener(maplibreMap, dialogContent))
        builder.setNegativeButton("Cancel", null)
        builder.setCancelable(false)
        builder.show()
    }

    private fun toggleLogCameraChanges() {
        logCameraChanges = !logCameraChanges
        if (logCameraChanges) {
            maplibreMap.addOnCameraIdleListener(idleListener)
            maplibreMap.addOnCameraMoveCancelListener(moveCanceledListener)
            maplibreMap.addOnCameraMoveListener(moveListener)
            maplibreMap.addOnCameraMoveStartedListener(moveStartedListener)
        } else {
            maplibreMap.removeOnCameraIdleListener(idleListener)
            maplibreMap.removeOnCameraMoveCancelListener(moveCanceledListener)
            maplibreMap.removeOnCameraMoveListener(moveListener)
            maplibreMap.removeOnCameraMoveStartedListener(moveStartedListener)
        }
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onDestroy() {
        super.onDestroy()
        if (::maplibreMap.isInitialized) {
            maplibreMap.removeOnMapLongClickListener(this)
        }
        if (::mapView.isInitialized) {
            mapView.onDestroy()
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    private fun onInflateDialogContent(view: View): View {
        linkTextView(view, R.id.value_lat, R.id.seekbar_lat, LatLngChangeListener(), 180 + 38)
        linkTextView(view, R.id.value_lon, R.id.seekbar_lon, LatLngChangeListener(), 180 - 77)
        linkTextView(view, R.id.value_zoom, R.id.seekbar_zoom, ValueChangeListener(), 6)
        linkTextView(view, R.id.value_bearing, R.id.seekbar_bearing, ValueChangeListener(), 90)
        linkTextView(view, R.id.value_tilt, R.id.seekbar_tilt, ValueChangeListener(), 40)
        return view
    }

    private fun linkTextView(view: View, @IdRes textViewRes: Int, @IdRes seekBarRes: Int, listener: ValueChangeListener, defaultValue: Int) {
        val value = view.findViewById<View>(textViewRes) as TextView
        val seekBar = view.findViewById<View>(seekBarRes) as SeekBar
        listener.setLinkedValueView(value)
        seekBar.setOnSeekBarChangeListener(listener)
        seekBar.progress = defaultValue
    }

    private val idleListener = OnCameraIdleListener {
        Timber.e("OnCameraIdle")
        fab.setColorFilter(
            ContextCompat.getColor(this@CameraPositionActivity, android.R.color.holo_green_dark)
        )
    }
    private val moveListener = OnCameraMoveListener {
        Timber.e("OnCameraMove")
        fab.setColorFilter(
            ContextCompat.getColor(this@CameraPositionActivity, android.R.color.holo_orange_dark)
        )
    }
    private val moveCanceledListener = OnCameraMoveCanceledListener {
        Timber.e("OnCameraMoveCanceled")
    }
    private val moveStartedListener: OnCameraMoveStartedListener = object : OnCameraMoveStartedListener {
        private val REASONS = arrayOf("REASON_API_GESTURE", "REASON_DEVELOPER_ANIMATION", "REASON_API_ANIMATION")

        override fun onCameraMoveStarted(reason: Int) {
            // reason ranges from 1 <-> 3
            fab.setColorFilter(
                ContextCompat.getColor(this@CameraPositionActivity, android.R.color.holo_red_dark)
            )
            Timber.e("OnCameraMoveStarted: %s", REASONS[reason - 1])
        }
    }

    private open inner class ValueChangeListener : OnSeekBarChangeListener {
        protected var textView: TextView? = null
        fun setLinkedValueView(textView: TextView?) {
            this.textView = textView
        }

        override fun onStartTrackingTouch(seekBar: SeekBar) {}
        override fun onStopTrackingTouch(seekBar: SeekBar) {}
        override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
            textView!!.text = progress.toString()
        }
    }

    private inner class LatLngChangeListener : ValueChangeListener() {
        override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
            super.onProgressChanged(seekBar, progress - 180, fromUser)
        }
    }

    private class DialogClickListener(private val maplibreMap: MapLibreMap?, private val dialogContent: View) : DialogInterface.OnClickListener {
        override fun onClick(dialog: DialogInterface, which: Int) {
            val latitude = (dialogContent.findViewById<View>(R.id.value_lat) as TextView).text.toString().toDouble()
            val longitude = (dialogContent.findViewById<View>(R.id.value_lon) as TextView).text.toString().toDouble()
            val zoom = (dialogContent.findViewById<View>(R.id.value_zoom) as TextView).text.toString().toDouble()
            val bearing = (dialogContent.findViewById<View>(R.id.value_bearing) as TextView).text.toString().toDouble()
            val tilt = (dialogContent.findViewById<View>(R.id.value_tilt) as TextView).text.toString().toDouble()
            if (latitude < GeometryConstants.MIN_LATITUDE || latitude > GeometryConstants.MAX_LATITUDE) {
                Toast.makeText(dialogContent.context, "latitude value must be set between " + GeometryConstants.MIN_LATITUDE + " and " + GeometryConstants.MAX_LATITUDE, Toast.LENGTH_SHORT).show()
                return
            }

            val cameraPosition = CameraPosition.Builder().target(LatLng(latitude, longitude)).zoom(zoom).bearing(bearing).tilt(tilt).build()

            maplibreMap?.animateCamera(
                CameraUpdateFactory.newCameraPosition(cameraPosition),
                5000,
                object : CancelableCallback {
                    override fun onCancel() {
                        Timber.v("OnCancel called")
                    }

                    override fun onFinish() {
                        Timber.v("OnFinish called")
                    }
                }
            )
            Timber.v(cameraPosition.toString())
        }
    }
}
