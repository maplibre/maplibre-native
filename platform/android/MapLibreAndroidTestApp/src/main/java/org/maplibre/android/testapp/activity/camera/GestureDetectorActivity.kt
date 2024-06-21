package org.maplibre.android.testapp.activity.camera

import android.annotation.SuppressLint
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.*
import android.widget.RelativeLayout
import android.widget.TextView
import androidx.annotation.ColorInt
import androidx.annotation.IntDef
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.mapbox.android.gestures.*
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMap.*
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.FontCache
import org.maplibre.android.testapp.utils.ResourceUtils
import java.lang.annotation.Retention
import java.lang.annotation.RetentionPolicy

/** Test activity showcasing APIs around gestures implementation. */
class GestureDetectorActivity : AppCompatActivity() {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var recyclerView: RecyclerView
    private var gestureAlertsAdapter: GestureAlertsAdapter? = null
    private var gesturesManager: AndroidGesturesManager? = null
    private var marker: Marker? = null
    private var focalPointLatLng: LatLng? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_gesture_detector)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { maplibreMap: MapLibreMap ->
                this@GestureDetectorActivity.maplibreMap = maplibreMap
                maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
                initializeMap()
            }
        )
        recyclerView = findViewById(R.id.alerts_recycler)
        recyclerView.setLayoutManager(LinearLayoutManager(this))
        gestureAlertsAdapter = GestureAlertsAdapter()
        recyclerView.setAdapter(gestureAlertsAdapter)
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        gestureAlertsAdapter!!.cancelUpdates()
        mapView.onPause()
    }

    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    private fun initializeMap() {
        gesturesManager = maplibreMap.gesturesManager
        val layoutParams = recyclerView!!.layoutParams as RelativeLayout.LayoutParams
        layoutParams.height = (mapView.height / 1.75).toInt()
        layoutParams.width = mapView.width / 3
        recyclerView!!.layoutParams = layoutParams
        attachListeners()
        fixedFocalPointEnabled(maplibreMap.uiSettings.focalPoint != null)
    }

    fun attachListeners() {
        maplibreMap.addOnMoveListener(
            object : OnMoveListener {
                override fun onMoveBegin(detector: MoveGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_START, "MOVE START")
                    )
                }

                override fun onMove(detector: MoveGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_PROGRESS, "MOVE PROGRESS")
                    )
                }

                override fun onMoveEnd(detector: MoveGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_END, "MOVE END")
                    )
                    recalculateFocalPoint()
                }
            }
        )
        maplibreMap.addOnRotateListener(
            object : OnRotateListener {
                override fun onRotateBegin(detector: RotateGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_START, "ROTATE START")
                    )
                }

                override fun onRotate(detector: RotateGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_PROGRESS, "ROTATE PROGRESS")
                    )
                    recalculateFocalPoint()
                }

                override fun onRotateEnd(detector: RotateGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_END, "ROTATE END")
                    )
                }
            }
        )
        maplibreMap.addOnScaleListener(
            object : OnScaleListener {
                override fun onScaleBegin(detector: StandardScaleGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_START, "SCALE START")
                    )
                    if (focalPointLatLng != null) {
                        gestureAlertsAdapter!!.addAlert(
                            GestureAlert(
                                GestureAlert.TYPE_OTHER,
                                "INCREASING MOVE THRESHOLD"
                            )
                        )
                        gesturesManager!!.moveGestureDetector.moveThreshold =
                            ResourceUtils.convertDpToPx(this@GestureDetectorActivity, 175f)
                        gestureAlertsAdapter!!.addAlert(
                            GestureAlert(
                                GestureAlert.TYPE_OTHER,
                                "MANUALLY INTERRUPTING MOVE"
                            )
                        )
                        gesturesManager!!.moveGestureDetector.interrupt()
                    }
                    recalculateFocalPoint()
                }

                override fun onScale(detector: StandardScaleGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_PROGRESS, "SCALE PROGRESS")
                    )
                }

                override fun onScaleEnd(detector: StandardScaleGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_END, "SCALE END")
                    )
                    if (focalPointLatLng != null) {
                        gestureAlertsAdapter!!.addAlert(
                            GestureAlert(
                                GestureAlert.TYPE_OTHER,
                                "REVERTING MOVE THRESHOLD"
                            )
                        )
                        gesturesManager!!.moveGestureDetector.moveThreshold = 0f
                    }
                }
            }
        )
        maplibreMap.addOnShoveListener(
            object : OnShoveListener {
                override fun onShoveBegin(detector: ShoveGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_START, "SHOVE START")
                    )
                }

                override fun onShove(detector: ShoveGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_PROGRESS, "SHOVE PROGRESS")
                    )
                }

                override fun onShoveEnd(detector: ShoveGestureDetector) {
                    gestureAlertsAdapter!!.addAlert(
                        GestureAlert(GestureAlert.TYPE_END, "SHOVE END")
                    )
                }
            }
        )
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_gestures, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        val uiSettings = maplibreMap.uiSettings
        when (item.itemId) {
            R.id.menu_gesture_focus_point -> {
                fixedFocalPointEnabled(focalPointLatLng == null)
                return true
            }
            R.id.menu_gesture_animation -> {
                uiSettings.isScaleVelocityAnimationEnabled =
                    !uiSettings.isScaleVelocityAnimationEnabled
                uiSettings.isRotateVelocityAnimationEnabled =
                    !uiSettings.isRotateVelocityAnimationEnabled
                uiSettings.isFlingVelocityAnimationEnabled =
                    !uiSettings.isFlingVelocityAnimationEnabled
                return true
            }
            R.id.menu_gesture_rotate -> {
                uiSettings.isRotateGesturesEnabled = !uiSettings.isRotateGesturesEnabled
                return true
            }
            R.id.menu_gesture_tilt -> {
                uiSettings.isTiltGesturesEnabled = !uiSettings.isTiltGesturesEnabled
                return true
            }
            R.id.menu_gesture_zoom -> {
                uiSettings.isZoomGesturesEnabled = !uiSettings.isZoomGesturesEnabled
                return true
            }
            R.id.menu_gesture_scroll -> {
                uiSettings.isScrollGesturesEnabled = !uiSettings.isScrollGesturesEnabled
                return true
            }
            R.id.menu_gesture_double_tap -> {
                uiSettings.isDoubleTapGesturesEnabled = !uiSettings.isDoubleTapGesturesEnabled
                return true
            }
            R.id.menu_gesture_quick_zoom -> {
                uiSettings.isQuickZoomGesturesEnabled = !uiSettings.isQuickZoomGesturesEnabled
                return true
            }
            R.id.menu_gesture_scroll_horizontal -> {
                uiSettings.isHorizontalScrollGesturesEnabled =
                    !uiSettings.isHorizontalScrollGesturesEnabled
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    private fun fixedFocalPointEnabled(enabled: Boolean) {
        if (enabled) {
            focalPointLatLng = LatLng(51.50325, -0.12968)
            marker = maplibreMap.addMarker(MarkerOptions().position(focalPointLatLng))
            maplibreMap.easeCamera(
                CameraUpdateFactory.newLatLngZoom(focalPointLatLng!!, 16.0),
                object : CancelableCallback {
                    override fun onCancel() {
                        recalculateFocalPoint()
                    }

                    override fun onFinish() {
                        recalculateFocalPoint()
                    }
                }
            )
        } else {
            if (marker != null) {
                maplibreMap.removeMarker(marker!!)
                marker = null
            }
            focalPointLatLng = null
            maplibreMap.uiSettings.focalPoint = null
        }
    }

    private fun recalculateFocalPoint() {
        if (focalPointLatLng != null) {
            maplibreMap.uiSettings.focalPoint =
                maplibreMap.projection.toScreenLocation(focalPointLatLng!!)
        }
    }

    private class GestureAlertsAdapter : RecyclerView.Adapter<GestureAlertsAdapter.ViewHolder>() {
        private var isUpdating = false
        private val updateHandler = Handler(Looper.getMainLooper())
        private val alerts: MutableList<GestureAlert> = ArrayList()

        class ViewHolder internal constructor(view: View) : RecyclerView.ViewHolder(view) {
            var alertMessageTv: TextView

            @ColorInt var textColor = 0

            init {
                val typeface = FontCache.get("Roboto-Regular.ttf", view.context)
                alertMessageTv = view.findViewById(R.id.alert_message)
                alertMessageTv.typeface = typeface
            }
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
            val view =
                LayoutInflater.from(parent.context)
                    .inflate(R.layout.item_gesture_alert, parent, false)
            return ViewHolder(view)
        }

        override fun onBindViewHolder(holder: ViewHolder, position: Int) {
            val alert = alerts[position]
            holder.alertMessageTv.text = alert.message
            holder.alertMessageTv.setTextColor(
                ContextCompat.getColor(holder.alertMessageTv.context, alert.color)
            )
        }

        override fun getItemCount(): Int {
            return alerts.size
        }

        fun addAlert(alert: GestureAlert) {
            for (gestureAlert in alerts) {
                if (gestureAlert.alertType != GestureAlert.TYPE_PROGRESS) {
                    break
                }
                if (alert.alertType == GestureAlert.TYPE_PROGRESS && gestureAlert == alert) {
                    return
                }
            }
            if (itemCount >= MAX_NUMBER_OF_ALERTS) {
                alerts.removeAt(itemCount - 1)
            }
            alerts.add(0, alert)
            if (!isUpdating) {
                isUpdating = true
                updateHandler.postDelayed(updateRunnable, 250)
            }
        }

        @SuppressLint("NotifyDataSetChanged")
        private val updateRunnable = Runnable {
            notifyDataSetChanged()
            isUpdating = false
        }

        fun cancelUpdates() {
            updateHandler.removeCallbacksAndMessages(null)
        }
    }

    private class GestureAlert
    internal constructor(
        @field:Type @param:Type
        val alertType: Int,
        val message: String?
    ) {
        @Retention(RetentionPolicy.SOURCE)
        @IntDef(TYPE_NONE, TYPE_START, TYPE_PROGRESS, TYPE_END, TYPE_OTHER)
        internal annotation class Type

        @ColorInt var color = 0
        override fun equals(o: Any?): Boolean {
            if (this === o) {
                return true
            }
            if (o == null || javaClass != o.javaClass) {
                return false
            }
            val that = o as GestureAlert
            if (alertType != that.alertType) {
                return false
            }
            return if (message != null) message == that.message else that.message == null
        }

        override fun hashCode(): Int {
            var result = alertType
            result = 31 * result + (message?.hashCode() ?: 0)
            return result
        }

        companion object {
            const val TYPE_NONE = 0
            const val TYPE_START = 1
            const val TYPE_END = 2
            const val TYPE_PROGRESS = 3
            const val TYPE_OTHER = 4
        }

        init {
            when (alertType) {
                TYPE_NONE -> color = android.R.color.black
                TYPE_END -> color = android.R.color.holo_red_dark
                TYPE_OTHER -> color = android.R.color.holo_purple
                TYPE_PROGRESS -> color = android.R.color.holo_orange_dark
                TYPE_START -> color = android.R.color.holo_green_dark
            }
        }
    }

    companion object {
        private const val MAX_NUMBER_OF_ALERTS = 30
    }
}
