package org.maplibre.android.maps

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import android.os.Handler
import android.os.Looper
import android.text.TextUtils
import androidx.annotation.IntRange
import androidx.annotation.Keep
import com.google.gson.JsonObject
import org.maplibre.android.LibraryLoader
import org.maplibre.android.MapStrictMode
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.Polygon
import org.maplibre.android.annotations.Polyline
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.exceptions.CalledFromWorkerThreadException
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.geometry.ProjectedMeters
import org.maplibre.android.log.Logger
import org.maplibre.android.maps.renderer.MapRenderer
import org.maplibre.android.storage.FileSource
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.CannotAddLayerException
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.light.Light
import org.maplibre.android.style.sources.CannotAddSourceException
import org.maplibre.android.style.sources.Source
import org.maplibre.android.tile.TileOperation
import org.maplibre.android.utils.BitmapUtils
import org.maplibre.geojson.Feature
import org.maplibre.geojson.Geometry
import kotlin.math.ceil

/**
 * Class that wraps the native methods for convenience.
 *
 * This class is referenced by name and the `nativePtr` field is read from the native peer,
 * they must not be renamed.
 */
@Suppress("TooManyFunctions", "LargeClass")
internal class NativeMapView(
    context: Context,
    nativeOptions: NativeMapOptions,
    viewCallback: ViewCallback?,
    stateCallback: StateCallback?,
    private val mapRenderer: MapRenderer,
) : NativeMap {
    // Hold a reference to prevent it from being GC'd as long as it's used on the native side
    private val fileSource: FileSource = FileSource.getInstance(context)

    // Used to validate if methods are called from the correct thread
    private val thread: Thread = Thread.currentThread()

    // Used for view callbacks
    private var viewCallback: ViewCallback? = viewCallback

    // Used for map change callbacks
    private val stateCallback: StateCallback? = stateCallback

    // Device density
    private val pixelRatio: Float = nativeOptions.pixelRatio()

    // Flag to indicate destroy was called
    private var destroyed = false

    // Cached to enable lazily set padding.
    // Whenever an animation is schedule, this value is cleared and the source of truth becomes the core transform state.
    private var edgeInsets: DoubleArray? = null

    // Holds the pointer to JNI NativeMapView
    @Keep
    override var nativePtr: Long = 0
        private set

    // Listener invoked to return a bitmap of the map
    private var snapshotReadyCallback: MapLibreMap.SnapshotReadyCallback? = null

    init {
        nativeInitialize(this, fileSource, mapRenderer, nativeOptions)
    }

    //
    // Constructors
    //

    constructor(
        context: Context,
        viewCallback: ViewCallback?,
        stateCallback: StateCallback?,
        mapRenderer: MapRenderer,
    ) : this(
        context,
        NativeMapOptions(context.resources.displayMetrics.density, false),
        viewCallback,
        stateCallback,
        mapRenderer,
    )

    constructor(
        context: Context,
        options: MapLibreMapOptions,
        viewCallback: ViewCallback?,
        stateCallback: StateCallback?,
        mapRenderer: MapRenderer,
    ) : this(context, NativeMapOptions(options), viewCallback, stateCallback, mapRenderer)

    //
    // Methods
    //

    private fun checkState(callingMethod: String): Boolean {
        // validate if invocation has occurred on the main thread
        if (thread !== Thread.currentThread()) {
            throw CalledFromWorkerThreadException(
                String.format(
                    "Map interactions should happen on the UI thread. Method invoked from wrong thread is %s.",
                    callingMethod,
                ),
            )
        }

        // validate if map has already been destroyed
        if (destroyed && !TextUtils.isEmpty(callingMethod)) {
            val message =
                String.format(
                    "You're calling `%s` after the `MapView` was destroyed, were you invoking it after `onDestroy()`?",
                    callingMethod,
                )
            Logger.e(TAG, message)

            MapStrictMode.strictModeViolation(message)
        }
        return destroyed
    }

    override fun destroy() {
        destroyed = true
        viewCallback = null
        nativeDestroy()
    }

    override fun resizeView(
        width: Int,
        height: Int,
    ) {
        if (checkState("resizeView")) {
            return
        }
        var scaledWidth = ceil(width / pixelRatio).toInt()
        var scaledHeight = ceil(height / pixelRatio).toInt()

        if (scaledWidth < 0) {
            Logger.e(
                TAG,
                String.format("Device returned a negative width size, setting value to 0 instead of %s", scaledWidth),
            )
            scaledWidth = 0
        }

        if (scaledHeight < 0) {
            Logger.e(
                TAG,
                String.format("Device returned a negative height size, setting value to 0 instead of %s", scaledHeight),
            )
            scaledHeight = 0
        }

        if (scaledWidth > MAX_DIMENSION) {
            // we have seen edge cases where devices return incorrect values #6111
            Logger.e(
                TAG,
                String.format(
                    "Device returned an out of range width size, capping value at 65535 instead of %s",
                    scaledWidth,
                ),
            )
            scaledWidth = MAX_DIMENSION
        }

        if (scaledHeight > MAX_DIMENSION) {
            // we have seen edge cases where devices return incorrect values #6111
            Logger.e(
                TAG,
                String.format(
                    "Device returned an out of range height size, capping value at 65535 instead of %s",
                    scaledHeight,
                ),
            )
            scaledHeight = MAX_DIMENSION
        }

        nativeResizeView(scaledWidth, scaledHeight)
    }

    override var styleUri: String
        get() {
            if (checkState("getStyleUri")) {
                return ""
            }
            return nativeGetStyleUrl()
        }
        set(value) {
            if (checkState("setStyleUri")) {
                return
            }
            nativeSetStyleUrl(value)
        }

    override var styleJson: String
        get() {
            if (checkState("getStyleJson")) {
                return ""
            }
            return nativeGetStyleJson()
        }
        set(value) {
            if (checkState("setStyleJson")) {
                return
            }
            nativeSetStyleJson(value)
        }

    override fun setLatLngBounds(latLngBounds: LatLngBounds?) {
        if (checkState("setLatLngBounds")) {
            return
        }
        nativeSetLatLngBounds(latLngBounds)
    }

    override fun cancelTransitions() {
        if (checkState("cancelTransitions")) {
            return
        }
        nativeCancelTransitions()
    }

    override fun setGestureInProgress(inProgress: Boolean) {
        if (checkState("setGestureInProgress")) {
            return
        }
        nativeSetGestureInProgress(inProgress)
    }

    override fun moveBy(
        deltaX: Double,
        deltaY: Double,
        duration: Long,
    ) {
        if (checkState("moveBy")) {
            return
        }

        try {
            nativeMoveBy(deltaX / pixelRatio, deltaY / pixelRatio, duration)
        } catch (error: Error) {
            // workaround for latitude must not be NaN issue
            // which is thrown when gl-native can't convert a screen coordinate to location
            Logger.d(TAG, "Error when executing NativeMapView#moveBy", error)
        }
    }

    override fun setLatLng(
        latLng: LatLng,
        duration: Long,
    ) {
        if (checkState("setLatLng")) {
            return
        }
        nativeSetLatLng(
            latLng.latitude,
            latLng.longitude,
            getAnimationPaddingAndClearCachedInsets(null),
            duration,
        )
    }

    override val latLng: LatLng
        get() {
            if (checkState("")) {
                return LatLng()
            }
            return nativeGetLatLng()
        }

    override fun getCameraForLatLngBounds(
        bounds: LatLngBounds,
        padding: IntArray,
        bearing: Double,
        pitch: Double,
    ): CameraPosition? {
        if (checkState("getCameraForLatLngBounds")) {
            return null
        }
        // Note that we have to juggle things a bit to match the ordering of arguments
        // to match the NativeMapView C++ interface.
        return nativeGetCameraForLatLngBounds(
            bounds,
            padding[1] / pixelRatio.toDouble(),
            padding[0] / pixelRatio.toDouble(),
            padding[3] / pixelRatio.toDouble(),
            padding[2] / pixelRatio.toDouble(),
            bearing,
            pitch,
        )
    }

    override fun getCameraForGeometry(
        geometry: Geometry,
        padding: IntArray,
        bearing: Double,
        pitch: Double,
    ): CameraPosition? {
        if (checkState("getCameraForGeometry")) {
            return null
        }
        return nativeGetCameraForGeometry(
            geometry,
            padding[1] / pixelRatio.toDouble(),
            padding[0] / pixelRatio.toDouble(),
            padding[3] / pixelRatio.toDouble(),
            padding[2] / pixelRatio.toDouble(),
            bearing,
            pitch,
        )
    }

    override fun resetPosition() {
        if (checkState("resetPosition")) {
            return
        }
        nativeResetPosition()
    }

    override val pitch: Double
        get() {
            if (checkState("getPitch")) {
                return 0.0
            }
            return nativeGetPitch()
        }

    override fun setPitch(
        pitch: Double,
        duration: Long,
    ) {
        if (checkState("setPitch")) {
            return
        }
        nativeSetPitch(pitch, duration)
    }

    override fun setZoom(
        zoom: Double,
        focalPoint: PointF,
        duration: Long,
    ) {
        if (checkState("setZoom")) {
            return
        }
        nativeSetZoom(zoom, (focalPoint.x / pixelRatio).toDouble(), (focalPoint.y / pixelRatio).toDouble(), duration)
    }

    override val zoom: Double
        get() {
            if (checkState("getZoom")) {
                return 0.0
            }
            return nativeGetZoom()
        }

    override fun resetZoom() {
        if (checkState("resetZoom")) {
            return
        }
        nativeResetZoom()
    }

    override var minZoom: Double
        get() {
            if (checkState("getMinZoom")) {
                return 0.0
            }
            return nativeGetMinZoom()
        }
        set(value) {
            if (checkState("setMinZoom")) {
                return
            }
            nativeSetMinZoom(value)
        }

    override var maxZoom: Double
        get() {
            if (checkState("getMaxZoom")) {
                return 0.0
            }
            return nativeGetMaxZoom()
        }
        set(value) {
            if (checkState("setMaxZoom")) {
                return
            }
            nativeSetMaxZoom(value)
        }

    override var minPitch: Double
        get() {
            if (checkState("getMinPitch")) {
                return 0.0
            }
            return nativeGetMinPitch()
        }
        set(value) {
            if (checkState("setMinPitch")) {
                return
            }
            nativeSetMinPitch(value)
        }

    override var maxPitch: Double
        get() {
            if (checkState("getMaxPitch")) {
                return 0.0
            }
            return nativeGetMaxPitch()
        }
        set(value) {
            if (checkState("setMaxPitch")) {
                return
            }
            nativeSetMaxPitch(value)
        }

    override fun rotateBy(
        sx: Double,
        sy: Double,
        ex: Double,
        ey: Double,
        duration: Long,
    ) {
        if (checkState("rotateBy")) {
            return
        }
        nativeRotateBy(sx / pixelRatio, sy / pixelRatio, ex, ey, duration)
    }

    override var contentPadding: DoubleArray?
        get() {
            if (checkState("getContentPadding")) {
                return doubleArrayOf(0.0, 0.0, 0.0, 0.0)
            }
            // if cached insets are not applied yet, return them, otherwise, get the padding from the camera
            return edgeInsets ?: cameraPosition.padding
        }
        set(padding) {
            if (checkState("setContentPadding")) {
                return
            }
            edgeInsets = padding
        }

    override fun setBearing(
        degrees: Double,
        duration: Long,
    ) {
        if (checkState("setBearing")) {
            return
        }
        nativeSetBearing(degrees, duration)
    }

    override fun setBearing(
        degrees: Double,
        fx: Double,
        fy: Double,
        duration: Long,
    ) {
        if (checkState("setBearing")) {
            return
        }
        nativeSetBearingXY(degrees, fx / pixelRatio, fy / pixelRatio, duration)
    }

    override val bearing: Double
        get() {
            if (checkState("getBearing")) {
                return 0.0
            }
            return nativeGetBearing()
        }

    override fun resetNorth() {
        if (checkState("resetNorth")) {
            return
        }
        nativeResetNorth()
    }

    override fun addMarker(marker: Marker): Long {
        if (checkState("addMarker")) {
            return 0
        }
        return nativeAddMarkers(arrayOf(marker))[0]
    }

    override fun addMarkers(markers: List<Marker>): LongArray {
        if (checkState("addMarkers")) {
            return longArrayOf()
        }
        return nativeAddMarkers(markers.toTypedArray())
    }

    override fun addPolyline(polyline: Polyline): Long {
        if (checkState("addPolyline")) {
            return 0
        }
        return nativeAddPolylines(arrayOf(polyline))[0]
    }

    override fun addPolylines(polylines: List<Polyline>): LongArray {
        if (checkState("addPolylines")) {
            return longArrayOf()
        }
        return nativeAddPolylines(polylines.toTypedArray())
    }

    override fun addPolygon(polygon: Polygon): Long {
        if (checkState("addPolygon")) {
            return 0
        }
        return nativeAddPolygons(arrayOf(polygon))[0]
    }

    override fun addPolygons(polygons: List<Polygon>): LongArray {
        if (checkState("addPolygons")) {
            return longArrayOf()
        }
        return nativeAddPolygons(polygons.toTypedArray())
    }

    override fun updateMarker(marker: Marker) {
        if (checkState("updateMarker")) {
            return
        }
        val position = marker.position!!
        val icon = marker.icon!!
        nativeUpdateMarker(marker.id, position.latitude, position.longitude, icon.id!!)
    }

    override fun updatePolygon(polygon: Polygon) {
        if (checkState("updatePolygon")) {
            return
        }
        nativeUpdatePolygon(polygon.id, polygon)
    }

    override fun updatePolyline(polyline: Polyline) {
        if (checkState("updatePolyline")) {
            return
        }
        nativeUpdatePolyline(polyline.id, polyline)
    }

    override fun removeAnnotation(id: Long) {
        if (checkState("removeAnnotation")) {
            return
        }
        removeAnnotations(longArrayOf(id))
    }

    override fun removeAnnotations(ids: LongArray) {
        if (checkState("removeAnnotations")) {
            return
        }
        nativeRemoveAnnotations(ids)
    }

    override fun queryPointAnnotations(rectF: RectF): LongArray {
        if (checkState("queryPointAnnotations")) {
            return longArrayOf()
        }
        return nativeQueryPointAnnotations(rectF)
    }

    override fun queryShapeAnnotations(rectF: RectF): LongArray {
        if (checkState("queryShapeAnnotations")) {
            return longArrayOf()
        }
        return nativeQueryShapeAnnotations(rectF)
    }

    override fun addAnnotationIcon(
        symbol: String,
        width: Int,
        height: Int,
        scale: Float,
        pixels: ByteArray,
    ) {
        if (checkState("addAnnotationIcon")) {
            return
        }
        nativeAddAnnotationIcon(symbol, width, height, scale, pixels)
    }

    override fun removeAnnotationIcon(symbol: String) {
        if (checkState("removeAnnotationIcon")) {
            return
        }
        nativeRemoveAnnotationIcon(symbol)
    }

    override fun setVisibleCoordinateBounds(
        coordinates: Array<LatLng>,
        padding: RectF,
        direction: Double,
        duration: Long,
    ) {
        if (checkState("setVisibleCoordinateBounds")) {
            return
        }
        nativeSetVisibleCoordinateBounds(coordinates, padding, direction, duration)
    }

    override fun onLowMemory() {
        if (checkState("onLowMemory")) {
            return
        }
        nativeOnLowMemory()
    }

    override fun setDebug(debug: Boolean) {
        if (checkState("setDebug")) {
            return
        }
        nativeSetDebug(debug)
    }

    override fun getDebug(): Boolean {
        if (checkState("getDebug")) {
            return false
        }
        return nativeGetDebug()
    }

    override fun getActionJournalLogFiles(): Array<String> {
        if (checkState("getActionJournalLogFiles")) {
            return emptyArray()
        }
        return nativeGetActionJournalLogFiles()
    }

    override fun getActionJournalLog(): Array<String> {
        if (checkState("getActionJournalLog")) {
            return emptyArray()
        }
        return nativeGetActionJournalLog()
    }

    override fun clearActionJournalLog() {
        if (checkState("clearActionJournalLog")) {
            return
        }
        nativeClearActionJournalLog()
    }

    override fun isFullyLoaded(): Boolean {
        if (checkState("isFullyLoaded")) {
            return false
        }
        return nativeIsFullyLoaded()
    }

    override fun setReachability(status: Boolean) {
        if (checkState("setReachability")) {
            return
        }
        nativeSetReachability(status)
    }

    override fun getMetersPerPixelAtLatitude(lat: Double): Double {
        if (checkState("getMetersPerPixelAtLatitude")) {
            return 0.0
        }
        return nativeGetMetersPerPixelAtLatitude(lat, zoom)
    }

    override fun projectedMetersForLatLng(latLng: LatLng): ProjectedMeters {
        if (checkState("projectedMetersForLatLng")) {
            return ProjectedMeters(0.0, 0.0)
        }
        return nativeProjectedMetersForLatLng(latLng.latitude, latLng.longitude)
    }

    override fun latLngForProjectedMeters(projectedMeters: ProjectedMeters): LatLng {
        if (checkState("latLngForProjectedMeters")) {
            return LatLng()
        }
        return nativeLatLngForProjectedMeters(projectedMeters.northing, projectedMeters.easting)
    }

    override fun pixelForLatLng(latLng: LatLng): PointF {
        if (checkState("pixelForLatLng")) {
            return PointF()
        }
        val pointF = nativePixelForLatLng(latLng.latitude, latLng.longitude)
        pointF.set(pointF.x * pixelRatio, pointF.y * pixelRatio)
        return pointF
    }

    override fun pixelsForLatLngs(
        input: DoubleArray,
        output: DoubleArray,
    ) {
        if (!checkState("pixelsForLatLngs")) {
            nativePixelsForLatLngs(input, output, pixelRatio)
        }
    }

    override fun getVisibleCoordinateBounds(output: DoubleArray) {
        if (!checkState("getVisibleCoordinateBounds")) {
            nativeGetVisibleCoordinateBounds(output)
        }
    }

    override fun latLngForPixel(pixel: PointF): LatLng {
        if (checkState("latLngForPixel")) {
            return LatLng()
        }
        return nativeLatLngForPixel(pixel.x / pixelRatio, pixel.y / pixelRatio)
    }

    override fun latLngsForPixels(
        input: DoubleArray,
        output: DoubleArray,
    ) {
        if (!checkState("latLngsForPixels")) {
            nativeLatLngsForPixels(input, output, pixelRatio)
        }
    }

    override fun getTopOffsetPixelsForAnnotationSymbol(symbolName: String): Double {
        if (checkState("getTopOffsetPixelsForAnnotationSymbol")) {
            return 0.0
        }
        return nativeGetTopOffsetPixelsForAnnotationSymbol(symbolName)
    }

    override fun jumpTo(
        center: LatLng,
        zoom: Double,
        pitch: Double,
        bearing: Double,
        padding: DoubleArray?,
    ) {
        if (checkState("jumpTo")) {
            return
        }
        nativeJumpTo(
            bearing,
            center.latitude,
            center.longitude,
            pitch,
            zoom,
            getAnimationPaddingAndClearCachedInsets(padding),
        )
    }

    @Suppress("LongParameterList")
    override fun easeTo(
        center: LatLng,
        zoom: Double,
        bearing: Double,
        pitch: Double,
        padding: DoubleArray?,
        duration: Long,
        easingInterpolator: Boolean,
    ) {
        if (checkState("easeTo")) {
            return
        }
        nativeEaseTo(
            bearing,
            center.latitude,
            center.longitude,
            duration,
            pitch,
            zoom,
            getAnimationPaddingAndClearCachedInsets(padding),
            easingInterpolator,
        )
    }

    @Suppress("LongParameterList")
    override fun flyTo(
        center: LatLng,
        zoom: Double,
        bearing: Double,
        pitch: Double,
        padding: DoubleArray?,
        duration: Long,
    ) {
        if (checkState("flyTo")) {
            return
        }
        nativeFlyTo(
            bearing,
            center.latitude,
            center.longitude,
            duration,
            pitch,
            zoom,
            getAnimationPaddingAndClearCachedInsets(padding),
        )
    }

    override val cameraPosition: CameraPosition
        get() {
            if (checkState("getCameraValues")) {
                return CameraPosition.Builder().build()
            }
            val edgeInsets = this.edgeInsets
            return if (edgeInsets != null) {
                CameraPosition.Builder(nativeGetCameraPosition()).padding(edgeInsets).build()
            } else {
                nativeGetCameraPosition()
            }
        }

    override var prefetchTiles: Boolean
        get() {
            if (checkState("getPrefetchTiles")) {
                return false
            }
            return nativeGetPrefetchTiles()
        }
        set(value) {
            if (checkState("setPrefetchTiles")) {
                return
            }
            nativeSetPrefetchTiles(value)
        }

    @setparam:IntRange(from = 0)
    @get:IntRange(from = 0)
    override var prefetchZoomDelta: Int
        get() {
            if (checkState("nativeGetPrefetchZoomDelta")) {
                return 0
            }
            return nativeGetPrefetchZoomDelta()
        }
        set(value) {
            if (checkState("nativeSetPrefetchZoomDelta")) {
                return
            }
            nativeSetPrefetchZoomDelta(value)
        }

    override fun setTileCacheEnabled(enabled: Boolean) {
        if (checkState("setTileCacheEnabled")) {
            return
        }
        nativeSetTileCacheEnabled(enabled)
    }

    override fun getTileCacheEnabled(): Boolean {
        if (checkState("getTileCacheEnabled")) {
            return false
        }
        return nativeGetTileCacheEnabled()
    }

    override fun setTileLodMinRadius(radius: Double) {
        if (checkState("setTileLodMinRadius")) {
            return
        }
        nativeSetTileLodMinRadius(radius)
    }

    override fun getTileLodMinRadius(): Double {
        if (checkState("getTileLodMinRadius")) {
            return 0.0
        }
        return nativeGetTileLodMinRadius()
    }

    override fun setTileLodScale(scale: Double) {
        if (checkState("setTileLodScale")) {
            return
        }
        nativeSetTileLodScale(scale)
    }

    override fun getTileLodScale(): Double {
        if (checkState("getTileLodScale")) {
            return 0.0
        }
        return nativeGetTileLodScale()
    }

    override fun setTileLodPitchThreshold(threshold: Double) {
        if (checkState("setTileLodPitchThreshold")) {
            return
        }
        nativeSetTileLodPitchThreshold(threshold)
    }

    override fun getTileLodPitchThreshold(): Double {
        if (checkState("getTileLodPitchThreshold")) {
            return 0.0
        }
        return nativeGetTileLodPitchThreshold()
    }

    override fun setTileLodZoomShift(shift: Double) {
        if (checkState("setTileLodZoomShift")) {
            return
        }
        nativeSetTileLodZoomShift(shift)
    }

    override fun getTileLodZoomShift(): Double {
        if (checkState("getTileLodZoomShift")) {
            return 0.0
        }
        return nativeGetTileLodZoomShift()
    }

    // Runtime style Api

    override var transitionOptions: TransitionOptions
        get() = nativeGetTransitionOptions()
        set(value) {
            nativeSetTransitionOptions(value)
        }

    override fun getLayers(): List<Layer> {
        if (checkState("getLayers")) {
            return ArrayList()
        }
        return nativeGetLayers().asList()
    }

    override fun getLayer(layerId: String): Layer? {
        if (checkState("getLayer")) {
            return null
        }
        return nativeGetLayer(layerId)
    }

    override fun addLayer(layer: Layer) {
        if (checkState("addLayer")) {
            return
        }
        nativeAddLayer(layer.nativePtr, null)
    }

    override fun addLayerBelow(
        layer: Layer,
        below: String,
    ) {
        if (checkState("addLayerBelow")) {
            return
        }
        nativeAddLayer(layer.nativePtr, below)
    }

    override fun addLayerAbove(
        layer: Layer,
        above: String,
    ) {
        if (checkState("addLayerAbove")) {
            return
        }
        nativeAddLayerAbove(layer.nativePtr, above)
    }

    override fun addLayerAt(
        layer: Layer,
        @IntRange(from = 0) index: Int,
    ) {
        if (checkState("addLayerAt")) {
            return
        }
        nativeAddLayerAt(layer.nativePtr, index)
    }

    override fun removeLayer(layerId: String): Boolean {
        if (checkState("removeLayer")) {
            return false
        }

        val layer = getLayer(layerId)
        if (layer != null) {
            return removeLayer(layer)
        }
        return false
    }

    override fun removeLayer(layer: Layer): Boolean {
        if (checkState("removeLayer")) {
            return false
        }
        if (layer.isDetached()) {
            Logger.w(TAG, "Ignoring removeLayer() call on detached layer reference.")
            return false
        }

        val layerNativePtr = layer.nativePtr
        if (layerNativePtr == 0L) {
            Logger.w(TAG, "Ignoring removeLayer() call on released layer pointer.")
            return false
        }
        return nativeRemoveLayer(layerNativePtr)
    }

    override fun removeLayerAt(
        @IntRange(from = 0) index: Int,
    ): Boolean {
        if (checkState("removeLayerAt")) {
            return false
        }
        return nativeRemoveLayerAt(index)
    }

    override fun getSources(): List<Source> {
        if (checkState("getSources")) {
            return ArrayList()
        }
        return nativeGetSources().asList()
    }

    override fun getSource(sourceId: String): Source? {
        if (checkState("getSource")) {
            return null
        }
        return nativeGetSource(sourceId)
    }

    override fun addSource(source: Source) {
        if (checkState("addSource")) {
            return
        }
        nativeAddSource(source, source.nativePtr)
    }

    override fun removeSource(sourceId: String): Boolean {
        if (checkState("removeSource")) {
            return false
        }
        val source = getSource(sourceId)
        if (source != null) {
            return removeSource(source)
        }
        return false
    }

    override fun removeSource(source: Source): Boolean {
        if (checkState("removeSource")) {
            return false
        }
        return nativeRemoveSource(source, source.nativePtr)
    }

    override fun addImages(images: Array<Image>) {
        if (checkState("addImages")) {
            return
        }
        nativeAddImages(images)
    }

    override fun removeImage(name: String) {
        if (checkState("removeImage")) {
            return
        }
        nativeRemoveImage(name)
    }

    override fun getImage(name: String): Bitmap? {
        if (checkState("getImage")) {
            return null
        }
        return nativeGetImage(name)
    }

    // Feature querying

    override fun queryRenderedFeatures(
        coordinates: PointF,
        layerIds: Array<out String>?,
        filter: Expression?,
    ): List<Feature> {
        if (checkState("queryRenderedFeatures")) {
            return ArrayList()
        }
        val features =
            nativeQueryRenderedFeaturesForPoint(
                coordinates.x / pixelRatio,
                coordinates.y / pixelRatio,
                layerIds,
                filter?.toArray(),
            )
        return features?.asList() ?: ArrayList()
    }

    override fun queryRenderedFeatures(
        coordinates: RectF,
        layerIds: Array<out String>?,
        filter: Expression?,
    ): List<Feature> {
        if (checkState("queryRenderedFeatures")) {
            return ArrayList()
        }
        val features =
            nativeQueryRenderedFeaturesForBox(
                coordinates.left / pixelRatio,
                coordinates.top / pixelRatio,
                coordinates.right / pixelRatio,
                coordinates.bottom / pixelRatio,
                layerIds,
                filter?.toArray(),
            )
        return features?.asList() ?: ArrayList()
    }

    override fun setFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String,
        state: JsonObject,
    ) {
        if (checkState("setFeatureState")) {
            return
        }
        nativeSetFeatureState(sourceId, sourceLayerId, featureId, state)
    }

    override fun getFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String,
    ): JsonObject? {
        if (checkState("getFeatureState")) {
            return null
        }
        return nativeGetFeatureState(sourceId, sourceLayerId, featureId)
    }

    override fun removeFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String?,
        stateKey: String?,
    ) {
        if (checkState("removeFeatureState")) {
            return
        }
        nativeRemoveFeatureState(sourceId, sourceLayerId, featureId, stateKey)
    }

    override fun setApiBaseUrl(baseUrl: String) {
        if (checkState("setApiBaseUrl")) {
            return
        }
        fileSource.setApiBaseUrl(baseUrl)
    }

    override fun getLight(): Light? {
        if (checkState("getLight")) {
            return null
        }
        return nativeGetLight()
    }

    override fun getPixelRatio(): Float = pixelRatio

    override fun triggerRepaint() {
        nativeTriggerRepaint()
    }

    override fun isRenderingStatsViewEnabled(): Boolean = nativeIsRenderingStatsViewEnabled()

    override fun enableRenderingStatsView(value: Boolean) {
        nativeEnableRenderingStatsView(value)
    }

    override fun setFrustumOffset(offset: RectF) {
        if (checkState("setFrustumOffset")) {
            return
        }
        nativeSetFrustumOffset(offset)
    }

    override fun setSwapBehaviorFlush(flush: Boolean) {
        mapRenderer.setSwapBehaviorFlush(flush)
    }

    override fun getDensityDependantRectangle(rectangle: RectF): RectF =
        RectF(
            rectangle.left / pixelRatio,
            rectangle.top / pixelRatio,
            rectangle.right / pixelRatio,
            rectangle.bottom / pixelRatio,
        )

    //
    // Callbacks
    //

    @Keep
    private fun onCameraWillChange(animated: Boolean) {
        stateCallback?.onCameraWillChange(animated)
    }

    @Keep
    private fun onCameraIsChanging() {
        stateCallback?.onCameraIsChanging()
    }

    @Keep
    private fun onCameraDidChange(animated: Boolean) {
        stateCallback?.onCameraDidChange(animated)
    }

    @Keep
    private fun onWillStartLoadingMap() {
        stateCallback?.onWillStartLoadingMap()
    }

    @Keep
    private fun onDidFinishLoadingMap() {
        stateCallback?.onDidFinishLoadingMap()
    }

    @Keep
    private fun onDidFailLoadingMap(error: String) {
        stateCallback?.onDidFailLoadingMap(error)
    }

    @Keep
    private fun onWillStartRenderingFrame() {
        stateCallback?.onWillStartRenderingFrame()
    }

    @Keep
    private fun onDidFinishRenderingFrame(
        fully: Boolean,
        stats: RenderingStats,
    ) {
        stateCallback?.onDidFinishRenderingFrame(fully, stats)
    }

    @Keep
    private fun onWillStartRenderingMap() {
        stateCallback?.onWillStartRenderingMap()
    }

    @Keep
    private fun onDidFinishRenderingMap(fully: Boolean) {
        stateCallback?.onDidFinishRenderingMap(fully)
    }

    @Keep
    private fun onDidBecomeIdle() {
        stateCallback?.onDidBecomeIdle()
    }

    @Keep
    private fun onDidFinishLoadingStyle() {
        stateCallback?.onDidFinishLoadingStyle()
    }

    @Keep
    private fun onSourceChanged(sourceId: String) {
        stateCallback?.onSourceChanged(sourceId)
    }

    @Keep
    private fun onStyleImageMissing(imageId: String) {
        stateCallback?.onStyleImageMissing(imageId)
    }

    @Keep
    private fun onCanRemoveUnusedStyleImage(imageId: String): Boolean = stateCallback?.onCanRemoveUnusedStyleImage(imageId) ?: true

    @Keep
    private fun onSnapshotReady(mapContent: Bitmap?) {
        if (checkState("OnSnapshotReady")) {
            return
        }

        try {
            val callback = snapshotReadyCallback
            if (callback != null && mapContent != null) {
                val viewCallback = this.viewCallback
                if (viewCallback == null) {
                    callback.onSnapshotReady(mapContent)
                } else {
                    val viewContent = viewCallback.getViewContent()
                    if (viewContent != null) {
                        callback.onSnapshotReady(BitmapUtils.mergeBitmaps(mapContent, viewContent))
                    }
                }
            }
        } catch (err: Throwable) {
            Logger.e(TAG, "Exception in onSnapshotReady", err)
            throw err
        }
    }

    @Keep
    private fun onPreCompileShader(
        id: Int,
        type: Int,
        additionalDefines: String,
    ) {
        stateCallback?.onPreCompileShader(id, type, additionalDefines)
    }

    @Keep
    private fun onPostCompileShader(
        id: Int,
        type: Int,
        additionalDefines: String,
    ) {
        stateCallback?.onPostCompileShader(id, type, additionalDefines)
    }

    @Keep
    private fun onShaderCompileFailed(
        id: Int,
        type: Int,
        additionalDefines: String,
    ) {
        stateCallback?.onShaderCompileFailed(id, type, additionalDefines)
    }

    @Keep
    private fun onGlyphsLoaded(
        stack: Array<String>,
        rangeStart: Int,
        rangeEnd: Int,
    ) {
        stateCallback?.onGlyphsLoaded(stack, rangeStart, rangeEnd)
    }

    @Keep
    private fun onGlyphsError(
        stack: Array<String>,
        rangeStart: Int,
        rangeEnd: Int,
    ) {
        stateCallback?.onGlyphsError(stack, rangeStart, rangeEnd)
    }

    @Keep
    private fun onGlyphsRequested(
        stack: Array<String>,
        rangeStart: Int,
        rangeEnd: Int,
    ) {
        stateCallback?.onGlyphsRequested(stack, rangeStart, rangeEnd)
    }

    @Keep
    @Suppress("LongParameterList")
    private fun onTileAction(
        op: TileOperation,
        x: Int,
        y: Int,
        z: Int,
        wrap: Int,
        overscaledZ: Int,
        sourceID: String,
    ) {
        stateCallback?.onTileAction(op, x, y, z, wrap, overscaledZ, sourceID)
    }

    @Keep
    private fun onSpriteLoaded(
        id: String,
        url: String,
    ) {
        stateCallback?.onSpriteLoaded(id, url)
    }

    @Keep
    private fun onSpriteError(
        id: String,
        url: String,
    ) {
        stateCallback?.onSpriteError(id, url)
    }

    @Keep
    private fun onSpriteRequested(
        id: String,
        url: String,
    ) {
        stateCallback?.onSpriteRequested(id, url)
    }

    @Keep
    private fun onRenderError() {
        stateCallback?.onRenderError()
    }

    @Keep
    private fun onSymbolError(message: String) {
        stateCallback?.onSymbolError(message)
    }

    //
    // JNI methods
    //

    @Keep
    private external fun nativeInitialize(
        nativeMap: NativeMapView,
        fileSource: FileSource,
        mapRenderer: MapRenderer,
        nativeOptions: NativeMapOptions,
    )

    @Keep
    private external fun nativeDestroy()

    @Keep
    private external fun nativeResizeView(
        width: Int,
        height: Int,
    )

    @Keep
    private external fun nativeSetStyleUrl(url: String)

    @Keep
    private external fun nativeGetStyleUrl(): String

    @Keep
    private external fun nativeSetStyleJson(newStyleJson: String)

    @Keep
    private external fun nativeGetStyleJson(): String

    @Keep
    private external fun nativeSetLatLngBounds(latLngBounds: LatLngBounds?)

    @Keep
    private external fun nativeCancelTransitions()

    @Keep
    private external fun nativeSetGestureInProgress(inProgress: Boolean)

    @Keep
    private external fun nativeMoveBy(
        dx: Double,
        dy: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeSetLatLng(
        latitude: Double,
        longitude: Double,
        padding: DoubleArray?,
        duration: Long,
    )

    @Keep
    private external fun nativeGetLatLng(): LatLng

    @Keep
    @Suppress("LongParameterList")
    private external fun nativeGetCameraForLatLngBounds(
        latLngBounds: LatLngBounds,
        top: Double,
        left: Double,
        bottom: Double,
        right: Double,
        bearing: Double,
        tilt: Double,
    ): CameraPosition

    @Keep
    @Suppress("LongParameterList")
    private external fun nativeGetCameraForGeometry(
        geometry: Geometry,
        top: Double,
        left: Double,
        bottom: Double,
        right: Double,
        bearing: Double,
        tilt: Double,
    ): CameraPosition

    @Keep
    private external fun nativeResetPosition()

    @Keep
    private external fun nativeGetPitch(): Double

    @Keep
    private external fun nativeSetPitch(
        pitch: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeSetZoom(
        zoom: Double,
        cx: Double,
        cy: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeGetZoom(): Double

    @Keep
    private external fun nativeResetZoom()

    @Keep
    private external fun nativeSetMinZoom(zoom: Double)

    @Keep
    private external fun nativeGetMinZoom(): Double

    @Keep
    private external fun nativeSetMaxZoom(zoom: Double)

    @Keep
    private external fun nativeGetMaxZoom(): Double

    @Keep
    private external fun nativeSetMinPitch(pitch: Double)

    @Keep
    private external fun nativeGetMinPitch(): Double

    @Keep
    private external fun nativeSetMaxPitch(pitch: Double)

    @Keep
    private external fun nativeGetMaxPitch(): Double

    @Keep
    private external fun nativeRotateBy(
        sx: Double,
        sy: Double,
        ex: Double,
        ey: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeSetBearing(
        degrees: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeSetBearingXY(
        degrees: Double,
        fx: Double,
        fy: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeGetBearing(): Double

    @Keep
    private external fun nativeResetNorth()

    @Keep
    private external fun nativeUpdateMarker(
        markerId: Long,
        lat: Double,
        lon: Double,
        iconId: String,
    )

    @Keep
    private external fun nativeAddMarkers(markers: Array<Marker>): LongArray

    @Keep
    private external fun nativeAddPolylines(polylines: Array<Polyline>): LongArray

    @Keep
    private external fun nativeAddPolygons(polygons: Array<Polygon>): LongArray

    @Keep
    private external fun nativeRemoveAnnotations(id: LongArray)

    @Keep
    private external fun nativeQueryPointAnnotations(rect: RectF): LongArray

    @Keep
    private external fun nativeQueryShapeAnnotations(rect: RectF): LongArray

    @Keep
    private external fun nativeAddAnnotationIcon(
        symbol: String,
        width: Int,
        height: Int,
        scale: Float,
        pixels: ByteArray,
    )

    @Keep
    private external fun nativeRemoveAnnotationIcon(symbol: String)

    @Keep
    private external fun nativeSetVisibleCoordinateBounds(
        coordinates: Array<LatLng>,
        padding: RectF,
        direction: Double,
        duration: Long,
    )

    @Keep
    private external fun nativeOnLowMemory()

    @Keep
    private external fun nativeSetDebug(debug: Boolean)

    @Keep
    private external fun nativeGetDebug(): Boolean

    @Keep
    private external fun nativeGetActionJournalLogFiles(): Array<String>

    @Keep
    private external fun nativeGetActionJournalLog(): Array<String>

    @Keep
    private external fun nativeClearActionJournalLog()

    @Keep
    private external fun nativeIsFullyLoaded(): Boolean

    @Keep
    private external fun nativeSetReachability(status: Boolean)

    @Keep
    private external fun nativeGetMetersPerPixelAtLatitude(
        lat: Double,
        zoom: Double,
    ): Double

    @Keep
    private external fun nativeProjectedMetersForLatLng(
        latitude: Double,
        longitude: Double,
    ): ProjectedMeters

    @Keep
    private external fun nativeLatLngForProjectedMeters(
        northing: Double,
        easting: Double,
    ): LatLng

    @Keep
    private external fun nativePixelForLatLng(
        lat: Double,
        lon: Double,
    ): PointF

    @Keep
    private external fun nativePixelsForLatLngs(
        input: DoubleArray,
        output: DoubleArray,
        pixelRatio: Float,
    )

    @Keep
    private external fun nativeGetVisibleCoordinateBounds(output: DoubleArray)

    @Keep
    private external fun nativeLatLngForPixel(
        x: Float,
        y: Float,
    ): LatLng

    @Keep
    private external fun nativeLatLngsForPixels(
        input: DoubleArray,
        output: DoubleArray,
        pixelRatio: Float,
    )

    @Keep
    private external fun nativeGetTopOffsetPixelsForAnnotationSymbol(symbolName: String): Double

    @Keep
    @Suppress("LongParameterList")
    private external fun nativeJumpTo(
        angle: Double,
        latitude: Double,
        longitude: Double,
        pitch: Double,
        zoom: Double,
        padding: DoubleArray?,
    )

    @Keep
    @Suppress("LongParameterList")
    private external fun nativeEaseTo(
        angle: Double,
        latitude: Double,
        longitude: Double,
        duration: Long,
        pitch: Double,
        zoom: Double,
        padding: DoubleArray?,
        easingInterpolator: Boolean,
    )

    @Keep
    @Suppress("LongParameterList")
    private external fun nativeFlyTo(
        angle: Double,
        latitude: Double,
        longitude: Double,
        duration: Long,
        pitch: Double,
        zoom: Double,
        padding: DoubleArray?,
    )

    @Keep
    private external fun nativeGetCameraPosition(): CameraPosition

    @Keep
    private external fun nativeSetTransitionOptions(transitionOptions: TransitionOptions)

    @Keep
    private external fun nativeGetTransitionOptions(): TransitionOptions

    @Keep
    @Suppress("unused")
    private external fun nativeGetTransitionDuration(): Long

    @Keep
    @Suppress("unused")
    private external fun nativeSetTransitionDuration(duration: Long)

    @Keep
    @Suppress("unused")
    private external fun nativeGetTransitionDelay(): Long

    @Keep
    @Suppress("unused")
    private external fun nativeSetTransitionDelay(delay: Long)

    @Keep
    private external fun nativeGetLayers(): Array<Layer>

    @Keep
    private external fun nativeGetLayer(layerId: String): Layer

    @Keep
    @Throws(CannotAddLayerException::class)
    private external fun nativeAddLayer(
        layerPtr: Long,
        before: String?,
    )

    @Keep
    @Throws(CannotAddLayerException::class)
    private external fun nativeAddLayerAbove(
        layerPtr: Long,
        above: String,
    )

    @Keep
    @Throws(CannotAddLayerException::class)
    private external fun nativeAddLayerAt(
        layerPtr: Long,
        index: Int,
    )

    @Keep
    private external fun nativeRemoveLayer(layerId: Long): Boolean

    @Keep
    private external fun nativeRemoveLayerAt(index: Int): Boolean

    @Keep
    private external fun nativeGetSources(): Array<Source>

    @Keep
    private external fun nativeGetSource(sourceId: String): Source

    @Keep
    @Throws(CannotAddSourceException::class)
    private external fun nativeAddSource(
        source: Source,
        sourcePtr: Long,
    )

    @Keep
    private external fun nativeRemoveSource(
        source: Source,
        sourcePtr: Long,
    ): Boolean

    @Keep
    @Suppress("unused")
    private external fun nativeAddImage(
        name: String,
        bitmap: Bitmap,
        pixelRatio: Float,
        sdf: Boolean,
    )

    @Keep
    private external fun nativeAddImages(images: Array<Image>)

    @Keep
    private external fun nativeRemoveImage(name: String)

    @Keep
    private external fun nativeGetImage(name: String): Bitmap

    @Keep
    private external fun nativeUpdatePolygon(
        polygonId: Long,
        polygon: Polygon,
    )

    @Keep
    private external fun nativeUpdatePolyline(
        polylineId: Long,
        polyline: Polyline,
    )

    @Keep
    private external fun nativeTakeSnapshot()

    @Keep
    private external fun nativeQueryRenderedFeaturesForPoint(
        x: Float,
        y: Float,
        layerIds: Array<out String>?,
        filter: Array<Any?>?,
    ): Array<Feature>?

    @Keep
    @Suppress("LongParameterList")
    private external fun nativeQueryRenderedFeaturesForBox(
        left: Float,
        top: Float,
        right: Float,
        bottom: Float,
        layerIds: Array<out String>?,
        filter: Array<Any?>?,
    ): Array<Feature>?

    @Keep
    private external fun nativeSetFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String,
        state: JsonObject,
    )

    @Keep
    private external fun nativeGetFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String,
    ): JsonObject?

    @Keep
    private external fun nativeRemoveFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String?,
        stateKey: String?,
    )

    @Keep
    private external fun nativeGetLight(): Light

    @Keep
    private external fun nativeSetPrefetchTiles(enable: Boolean)

    @Keep
    private external fun nativeGetPrefetchTiles(): Boolean

    @Keep
    private external fun nativeSetPrefetchZoomDelta(delta: Int)

    @Keep
    private external fun nativeSetTileCacheEnabled(enabled: Boolean)

    @Keep
    private external fun nativeGetTileCacheEnabled(): Boolean

    @Keep
    private external fun nativeGetPrefetchZoomDelta(): Int

    @Keep
    private external fun nativeSetTileLodMinRadius(radius: Double)

    @Keep
    private external fun nativeGetTileLodMinRadius(): Double

    @Keep
    private external fun nativeSetTileLodScale(scale: Double)

    @Keep
    private external fun nativeGetTileLodScale(): Double

    @Keep
    private external fun nativeSetTileLodPitchThreshold(threshold: Double)

    @Keep
    private external fun nativeGetTileLodPitchThreshold(): Double

    @Keep
    private external fun nativeSetTileLodZoomShift(shift: Double)

    @Keep
    private external fun nativeGetTileLodZoomShift(): Double

    @Keep
    private external fun nativeTriggerRepaint()

    @Keep
    private external fun nativeIsRenderingStatsViewEnabled(): Boolean

    @Keep
    private external fun nativeEnableRenderingStatsView(enabled: Boolean)

    @Keep
    private external fun nativeSetFrustumOffset(offsset: RectF)

    //
    // Snapshot
    //

    override fun addSnapshotCallback(callback: MapLibreMap.SnapshotReadyCallback) {
        if (checkState("addSnapshotCallback")) {
            return
        }
        snapshotReadyCallback = callback
        nativeTakeSnapshot()
    }

    override fun setOnFpsChangedListener(listener: MapLibreMap.OnFpsChangedListener?) {
        val handler = Handler(Looper.getMainLooper())
        mapRenderer.queueEvent {
            mapRenderer.onFpsChangedListener =
                if (listener != null) {
                    MapLibreMap.OnFpsChangedListener { fps -> handler.post { listener.onFpsChanged(fps) } }
                } else {
                    null
                }
        }
    }

    override val isDestroyed: Boolean
        get() = destroyed

    private fun getAnimationPaddingAndClearCachedInsets(providedPadding: DoubleArray?): DoubleArray? {
        val padding = providedPadding ?: edgeInsets
        edgeInsets = null
        return if (padding == null) {
            null
        } else {
            doubleArrayOf(
                padding[1] / pixelRatio,
                padding[0] / pixelRatio,
                padding[3] / pixelRatio,
                padding[2] / pixelRatio,
            )
        }
    }

    interface ViewCallback {
        fun getViewContent(): Bitmap?
    }

    internal interface StyleCallback {
        fun onWillStartLoadingMap()

        fun onDidFinishLoadingStyle()
    }

    @Suppress("TooManyFunctions")
    internal interface StateCallback : StyleCallback {
        fun onCameraWillChange(animated: Boolean)

        fun onCameraIsChanging()

        fun onCameraDidChange(animated: Boolean)

        fun onDidFinishLoadingMap()

        fun onDidFailLoadingMap(error: String)

        fun onWillStartRenderingFrame()

        fun onDidFinishRenderingFrame(
            fully: Boolean,
            stats: RenderingStats,
        )

        fun onWillStartRenderingMap()

        fun onDidFinishRenderingMap(fully: Boolean)

        fun onDidBecomeIdle()

        fun onSourceChanged(sourceId: String)

        fun onStyleImageMissing(imageId: String)

        fun onCanRemoveUnusedStyleImage(imageId: String): Boolean

        fun onPreCompileShader(
            id: Int,
            type: Int,
            additionalDefines: String,
        )

        fun onPostCompileShader(
            id: Int,
            type: Int,
            additionalDefines: String,
        )

        fun onShaderCompileFailed(
            id: Int,
            type: Int,
            additionalDefines: String,
        )

        fun onGlyphsLoaded(
            stack: Array<String>,
            rangeStart: Int,
            rangeEnd: Int,
        )

        fun onGlyphsError(
            stack: Array<String>,
            rangeStart: Int,
            rangeEnd: Int,
        )

        fun onGlyphsRequested(
            stack: Array<String>,
            rangeStart: Int,
            rangeEnd: Int,
        )

        @Suppress("LongParameterList")
        fun onTileAction(
            op: TileOperation,
            x: Int,
            y: Int,
            z: Int,
            wrap: Int,
            overscaledZ: Int,
            sourceID: String,
        )

        fun onSpriteLoaded(
            id: String,
            url: String,
        )

        fun onSpriteError(
            id: String,
            url: String,
        )

        fun onSpriteRequested(
            id: String,
            url: String,
        )

        fun onRenderError()

        fun onSymbolError(message: String)
    }

    private companion object {
        const val TAG = "Mbgl-NativeMapView"

        const val MAX_DIMENSION = 65535

        init {
            LibraryLoader.load()
        }
    }
}
