package org.maplibre.android.annotations

import android.graphics.PointF
import android.os.Build
import android.text.TextUtils
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.ViewTreeObserver
import android.widget.FrameLayout
import android.widget.TextView
import androidx.annotation.LayoutRes
import org.maplibre.android.R
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import java.lang.ref.WeakReference

/**
 * `InfoWindow` is a tooltip shown when a [Marker] is tapped. Only
 * one info window is displayed at a time. When the user clicks on a marker, the currently open info
 * window will be closed and the new info window will be displayed. If the user clicks the same
 * marker while its info window is currently open, the info window will be closed.
 *
 * The info window is drawn oriented against the device's screen, centered above its associated
 * marker by default. The default info window contains the title in bold and snippet text below the
 * title. While either the title and snippet are optional, at least one is required to open the
 * info window.
 */
@Deprecated(
    "As of 7.0.0, use " +
        "[MapLibre Annotation Plugin](https://github.com/maplibre/maplibre-plugins-android) instead",
)
open class InfoWindow {
    private var boundMarkerRef: WeakReference<Marker?>? = null
    private var maplibreMap: WeakReference<MapLibreMap?>

    @JvmField
    protected var viewRef: WeakReference<View?>

    private var markerHeightOffset = 0f
    private var markerWidthOffset = 0f
    private var viewWidthOffset = 0f
    private var viewHeightOffset = 0f
    private var coordinates: PointF? = null

    /**
     * Whether this [InfoWindow] is currently shown on the map.
     */
    internal var isVisible = false
        private set

    @LayoutRes
    private var layoutRes = 0

    internal constructor(
        mapView: MapView,
        @LayoutRes layoutResId: Int,
        maplibreMap: MapLibreMap?,
    ) {
        layoutRes = layoutResId
        val view = LayoutInflater.from(mapView.context).inflate(layoutResId, mapView, false)
        this.maplibreMap = WeakReference(maplibreMap)
        this.viewRef = WeakReference(view)
        initialize(view, maplibreMap)
    }

    internal constructor(view: View, maplibreMap: MapLibreMap?) {
        this.maplibreMap = WeakReference(maplibreMap)
        this.viewRef = WeakReference(view)
        initialize(view, maplibreMap)
    }

    private fun initialize(
        view: View,
        maplibreMap: MapLibreMap?,
    ) {
        this.maplibreMap = WeakReference(maplibreMap)
        isVisible = false
        this.viewRef = WeakReference(view)

        view.setOnClickListener {
            val map = this.maplibreMap.get()
            if (map != null) {
                val onInfoWindowClickListener = map.onInfoWindowClickListener
                val marker = boundMarker
                var handledDefaultClick = false
                if (onInfoWindowClickListener != null && marker != null) {
                    handledDefaultClick = onInfoWindowClickListener.onInfoWindowClick(marker)
                }

                if (!handledDefaultClick) {
                    // default behavior: close it when clicking on the tooltip:
                    closeInfoWindow()
                }
            }
        }

        view.setOnLongClickListener {
            val map = this.maplibreMap.get()
            val marker = boundMarker
            if (map != null && marker != null) {
                map.onInfoWindowLongClickListener?.onInfoWindowLongClick(marker)
            }
            true
        }
    }

    private fun closeInfoWindow() {
        val map = maplibreMap.get()
        val marker = boundMarkerRef?.get()
        if (marker != null && map != null) {
            map.deselectMarker(marker)
        }
        close()
    }

    /**
     * Open the info window at the specified position.
     *
     * @param boundMarker The marker on which is hooked the view.
     * @param position    to place the window on the map.
     * @param offsetX     The offset of the view to the position, in pixels. This allows to offset
     *                    the view from the object position.
     * @param offsetY     The offset of the view to the position, in pixels. This allows to offset
     *                    the view from the object position.
     * @return this [InfoWindow].
     */
    internal fun open(
        mapView: MapView,
        boundMarker: Marker?,
        position: LatLng,
        offsetX: Int,
        offsetY: Int,
    ): InfoWindow {
        setBoundMarker(boundMarker)

        val lp =
            FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT,
                FrameLayout.LayoutParams.WRAP_CONTENT,
            )

        val maplibreMap = this.maplibreMap.get()
        val view = this.viewRef.get()
        if (view != null && maplibreMap != null) {
            view.measure(View.MeasureSpec.UNSPECIFIED, View.MeasureSpec.UNSPECIFIED)

            markerHeightOffset = offsetY.toFloat()
            markerWidthOffset = -offsetX.toFloat()

            // Calculate default Android x,y coordinate
            val coordinates = maplibreMap.projection.toScreenLocation(position)
            this.coordinates = coordinates
            var x = coordinates.x - (view.measuredWidth / 2) + offsetX
            val y = coordinates.y - view.measuredHeight + offsetY

            if (view is BubbleLayout) {
                // only apply repositioning/margin for InfoWindowView
                val resources = mapView.context.resources

                // get right/left popup window
                var rightSideInfowWindow = x + view.measuredWidth
                var leftSideInfoWindow = x

                // get right/left map view
                val mapRight = mapView.right.toFloat()
                val mapLeft = mapView.left.toFloat()

                val marginHorizontal = resources.getDimension(R.dimen.maplibre_infowindow_margin)
                val tipViewOffset = resources.getDimension(R.dimen.maplibre_infowindow_tipview_width) / 2
                var tipViewMarginLeft = view.measuredWidth / 2 - tipViewOffset

                var outOfBoundsLeft = false
                var outOfBoundsRight = false

                // only optimise margins if view is inside current viewport
                if (coordinates.x >= 0 && coordinates.x <= mapView.width &&
                    coordinates.y >= 0 && coordinates.y <= mapView.height
                ) {
                    // if out of bounds right
                    if (rightSideInfowWindow > mapRight) {
                        outOfBoundsRight = true
                        x -= rightSideInfowWindow - mapRight
                        tipViewMarginLeft += rightSideInfowWindow - mapRight + tipViewOffset
                        rightSideInfowWindow = x + view.measuredWidth
                    }

                    // fit screen left
                    if (leftSideInfoWindow < mapLeft) {
                        outOfBoundsLeft = true
                        x += mapLeft - leftSideInfoWindow
                        tipViewMarginLeft -= mapLeft - leftSideInfoWindow + tipViewOffset
                        leftSideInfoWindow = x
                    }

                    // Add margin right
                    if (outOfBoundsRight && mapRight - rightSideInfowWindow < marginHorizontal) {
                        x -= marginHorizontal - (mapRight - rightSideInfowWindow)
                        tipViewMarginLeft += marginHorizontal - (mapRight - rightSideInfowWindow) - tipViewOffset
                        leftSideInfoWindow = x
                    }

                    // Add margin left
                    if (outOfBoundsLeft && leftSideInfoWindow - mapLeft < marginHorizontal) {
                        x += marginHorizontal - (leftSideInfoWindow - mapLeft)
                        tipViewMarginLeft -= (marginHorizontal - (leftSideInfoWindow - mapLeft)) - tipViewOffset
                    }
                }

                // Adjust tipView
                view.setArrowPosition(tipViewMarginLeft)
            }

            // set anchor popupwindowview
            view.x = x
            view.y = y

            // Calculate x-offset and y-offset for update method
            viewWidthOffset = x - coordinates.x - offsetX
            viewHeightOffset = -view.measuredHeight + offsetY.toFloat()

            close() // if it was already opened
            mapView.addView(view, lp)
            isVisible = true
        }
        return this
    }

    /**
     * Close this [InfoWindow] if it is visible, otherwise calling this will do nothing.
     *
     * @return This [InfoWindow]
     */
    internal fun close(): InfoWindow {
        val maplibreMap = this.maplibreMap.get()
        if (isVisible && maplibreMap != null) {
            isVisible = false
            val view = this.viewRef.get()
            val parent = view?.parent
            if (parent != null) {
                (parent as ViewGroup).removeView(view)
            }

            val marker = boundMarker
            if (marker != null) {
                maplibreMap.onInfoWindowCloseListener?.onInfoWindowClose(marker)
            }

            setBoundMarker(null)
        }
        return this
    }

    /**
     * Constructs the view that is displayed when the InfoWindow opens. This retrieves data from
     * overlayItem and shows it in the tooltip.
     *
     * @param overlayItem the tapped overlay item
     */
    internal fun adaptDefaultMarker(
        overlayItem: Marker,
        maplibreMap: MapLibreMap?,
        mapView: MapView,
    ) {
        var view = this.viewRef.get()
        if (view == null) {
            view = LayoutInflater.from(mapView.context).inflate(layoutRes, mapView, false)
            initialize(view, maplibreMap)
        }
        this.maplibreMap = WeakReference(maplibreMap)

        val title = overlayItem.title
        val titleTextView = view.findViewById<TextView>(R.id.infowindow_title)
        if (!TextUtils.isEmpty(title)) {
            titleTextView.text = title
            titleTextView.visibility = View.VISIBLE
        } else {
            titleTextView.visibility = View.GONE
        }

        val snippet = overlayItem.snippet
        val snippetTextView = view.findViewById<TextView>(R.id.infowindow_description)
        if (!TextUtils.isEmpty(snippet)) {
            snippetTextView.text = snippet
            snippetTextView.visibility = View.VISIBLE
        } else {
            snippetTextView.visibility = View.GONE
        }
    }

    internal fun setBoundMarker(boundMarker: Marker?): InfoWindow {
        this.boundMarkerRef = WeakReference(boundMarker)
        return this
    }

    /**
     * The [Marker] this [InfoWindow] is currently bound to.
     */
    internal val boundMarker: Marker?
        get() = boundMarkerRef?.get()

    /**
     * Will result in getting this [InfoWindow] and updating the view being displayed.
     */
    fun update() {
        val maplibreMap = this.maplibreMap.get()
        val marker = boundMarkerRef?.get()
        val view = this.viewRef.get()
        if (maplibreMap != null && marker != null && view != null) {
            val coordinates = maplibreMap.projection.toScreenLocation(marker.position!!)
            this.coordinates = coordinates

            if (view is BubbleLayout) {
                view.x = coordinates.x + viewWidthOffset - markerWidthOffset
            } else {
                view.x = coordinates.x - (view.measuredWidth / 2) - markerWidthOffset
            }
            view.y = coordinates.y + viewHeightOffset
        }
    }

    internal fun onContentUpdate() {
        // recalculate y-offset and update position
        val view = this.viewRef.get()
        if (view != null) {
            val viewTreeObserver = view.viewTreeObserver
            if (viewTreeObserver.isAlive) {
                viewTreeObserver.addOnGlobalLayoutListener(contentUpdateListener)
            }
        }
    }

    private val contentUpdateListener =
        object : ViewTreeObserver.OnGlobalLayoutListener {
            override fun onGlobalLayout() {
                val view = this@InfoWindow.viewRef.get()
                if (view != null) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                        view.viewTreeObserver.removeOnGlobalLayoutListener(this)
                    } else {
                        @Suppress("DEPRECATION")
                        view.viewTreeObserver.removeGlobalOnLayoutListener(this)
                    }
                    viewHeightOffset = -view.measuredHeight + markerHeightOffset
                    update()
                }
            }
        }

    /**
     * Retrieve this [InfoWindow]'s current view being used.
     *
     * @return This [InfoWindow]'s current View.
     */
    val view: View? get() = viewRef.get()
}
