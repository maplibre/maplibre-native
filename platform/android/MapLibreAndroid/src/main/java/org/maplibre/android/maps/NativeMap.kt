package org.maplibre.android.maps

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import androidx.annotation.IntRange
import com.google.gson.JsonObject
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.Polygon
import org.maplibre.android.annotations.Polyline
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.geometry.LatLngBounds
import org.maplibre.android.geometry.ProjectedMeters
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.Layer
import org.maplibre.android.style.layers.TransitionOptions
import org.maplibre.android.style.light.Light
import org.maplibre.android.style.sources.Source
import org.maplibre.geojson.Feature
import org.maplibre.geojson.Geometry

@Suppress("TooManyFunctions")
internal interface NativeMap {
    //
    // Lifecycle API
    //

    fun resizeView(
        width: Int,
        height: Int,
    )

    fun onLowMemory()

    fun destroy()

    val isDestroyed: Boolean

    //
    // Camera API
    //

    fun jumpTo(
        center: LatLng,
        zoom: Double,
        pitch: Double,
        bearing: Double,
        padding: DoubleArray?,
    )

    @Suppress("LongParameterList")
    fun easeTo(
        center: LatLng,
        zoom: Double,
        bearing: Double,
        pitch: Double,
        padding: DoubleArray?,
        duration: Long,
        easingInterpolator: Boolean,
    )

    @Suppress("LongParameterList")
    fun flyTo(
        center: LatLng,
        zoom: Double,
        bearing: Double,
        pitch: Double,
        padding: DoubleArray?,
        duration: Long,
    )

    fun moveBy(
        deltaX: Double,
        deltaY: Double,
        duration: Long,
    )

    val cameraPosition: CameraPosition

    // Note for implementors: the ordering of the padding is left, top, right, bottom
    fun getCameraForLatLngBounds(
        bounds: LatLngBounds,
        padding: IntArray,
        bearing: Double,
        pitch: Double,
    ): CameraPosition?

    fun getCameraForGeometry(
        geometry: Geometry,
        padding: IntArray,
        bearing: Double,
        pitch: Double,
    ): CameraPosition?

    fun resetPosition()

    fun setLatLng(
        latLng: LatLng,
        duration: Long,
    )

    val latLng: LatLng

    fun setLatLngBounds(latLngBounds: LatLngBounds?)

    fun setVisibleCoordinateBounds(
        coordinates: Array<LatLng>,
        padding: RectF,
        direction: Double,
        duration: Long,
    )

    fun setPitch(
        pitch: Double,
        duration: Long,
    )

    val pitch: Double

    fun setZoom(
        zoom: Double,
        focalPoint: PointF,
        duration: Long,
    )

    val zoom: Double

    var minZoom: Double

    var maxZoom: Double

    var minPitch: Double

    var maxPitch: Double

    fun resetZoom()

    fun rotateBy(
        sx: Double,
        sy: Double,
        ex: Double,
        ey: Double,
        duration: Long,
    )

    fun setBearing(
        degrees: Double,
        duration: Long,
    )

    fun setBearing(
        degrees: Double,
        fx: Double,
        fy: Double,
        duration: Long,
    )

    val bearing: Double

    fun resetNorth()

    fun cancelTransitions()

    //
    // Style API
    //

    var styleUri: String

    var styleJson: String

    fun isFullyLoaded(): Boolean

    fun addLayer(layer: Layer)

    fun addLayerBelow(
        layer: Layer,
        below: String,
    )

    fun addLayerAbove(
        layer: Layer,
        above: String,
    )

    fun addLayerAt(
        layer: Layer,
        @IntRange(from = 0) index: Int,
    )

    fun getLayers(): List<Layer>

    fun getLayer(layerId: String): Layer?

    fun removeLayer(layerId: String): Boolean

    fun removeLayer(layer: Layer): Boolean

    fun removeLayerAt(
        @IntRange(from = 0) index: Int,
    ): Boolean

    fun addSource(source: Source)

    fun getSources(): List<Source>

    fun getSource(sourceId: String): Source?

    fun removeSource(sourceId: String): Boolean

    fun removeSource(source: Source): Boolean

    var transitionOptions: TransitionOptions

    fun addImages(images: Array<Image>)

    fun getImage(name: String): Bitmap?

    fun removeImage(name: String)

    fun getLight(): Light?

    //
    // Content padding API
    //

    var contentPadding: DoubleArray?

    //
    // Query API
    //

    fun queryRenderedFeatures(
        coordinates: PointF,
        layerIds: Array<out String>?,
        filter: Expression?,
    ): List<Feature>

    fun queryRenderedFeatures(
        coordinates: RectF,
        layerIds: Array<out String>?,
        filter: Expression?,
    ): List<Feature>

    fun setFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String,
        state: JsonObject,
    )

    fun getFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String,
    ): JsonObject?

    fun removeFeatureState(
        sourceId: String,
        sourceLayerId: String?,
        featureId: String?,
        stateKey: String?,
    )

    //
    // Projection API
    //

    fun getMetersPerPixelAtLatitude(lat: Double): Double

    fun projectedMetersForLatLng(latLng: LatLng): ProjectedMeters

    fun latLngForProjectedMeters(projectedMeters: ProjectedMeters): LatLng

    fun pixelForLatLng(latLng: LatLng): PointF

    fun pixelsForLatLngs(
        input: DoubleArray,
        output: DoubleArray,
    )

    fun getVisibleCoordinateBounds(output: DoubleArray)

    fun latLngForPixel(pixel: PointF): LatLng

    fun latLngsForPixels(
        input: DoubleArray,
        output: DoubleArray,
    )

    //
    // Utils API
    //

    fun setOnFpsChangedListener(listener: MapLibreMap.OnFpsChangedListener?)

    fun setDebug(debug: Boolean)

    fun getDebug(): Boolean

    fun getActionJournalLogFiles(): Array<String>

    fun getActionJournalLog(): Array<String>

    fun clearActionJournalLog()

    fun setReachability(status: Boolean)

    fun setApiBaseUrl(baseUrl: String)

    var prefetchTiles: Boolean

    @setparam:IntRange(from = 0)
    @get:IntRange(from = 0)
    var prefetchZoomDelta: Int

    fun setTileCacheEnabled(enabled: Boolean)

    fun getTileCacheEnabled(): Boolean

    fun setTileLodMinRadius(radius: Double)

    fun getTileLodMinRadius(): Double

    fun setTileLodScale(scale: Double)

    fun getTileLodScale(): Double

    fun setTileLodPitchThreshold(threshold: Double)

    fun getTileLodPitchThreshold(): Double

    fun setTileLodZoomShift(shift: Double)

    fun getTileLodZoomShift(): Double

    fun setGestureInProgress(inProgress: Boolean)

    fun getPixelRatio(): Float

    fun triggerRepaint()

    fun isRenderingStatsViewEnabled(): Boolean

    fun enableRenderingStatsView(value: Boolean)

    fun setFrustumOffset(offset: RectF)

    fun setSwapBehaviorFlush(flush: Boolean)

    //
    // Deprecated Annotations API
    //

    fun addMarker(marker: Marker): Long

    fun addMarkers(markers: List<Marker>): LongArray

    fun addPolyline(polyline: Polyline): Long

    fun addPolylines(polylines: List<Polyline>): LongArray

    fun addPolygon(polygon: Polygon): Long

    fun addPolygons(polygons: List<Polygon>): LongArray

    fun updateMarker(marker: Marker)

    fun updatePolygon(polygon: Polygon)

    fun updatePolyline(polyline: Polyline)

    fun removeAnnotation(id: Long)

    fun removeAnnotations(ids: LongArray)

    fun getTopOffsetPixelsForAnnotationSymbol(symbolName: String): Double

    fun addAnnotationIcon(
        symbol: String,
        width: Int,
        height: Int,
        scale: Float,
        pixels: ByteArray,
    )

    fun removeAnnotationIcon(symbol: String)

    fun queryPointAnnotations(rectF: RectF): LongArray

    fun queryShapeAnnotations(rectF: RectF): LongArray

    fun getDensityDependantRectangle(rectangle: RectF): RectF

    val nativePtr: Long

    fun addSnapshotCallback(callback: MapLibreMap.SnapshotReadyCallback)
}
