package org.maplibre.android.testapp.activity.stability

import android.graphics.Color
import android.graphics.PointF
import android.os.SystemClock
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.LineLayer
import org.maplibre.android.style.layers.Property
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.sources.GeoJsonOptions
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.geojson.Feature
import org.maplibre.geojson.LineString
import org.maplibre.geojson.Point
import kotlin.math.hypot

/**
 * Manages a GeoJSON route line dynamically trimmed to a moving location on a map.
 * Split mode keeps a head at the puck and uploads the tail only when the join vertex moves forward.
 * Whole-line mode uploads the remaining route on every trim tick.
 */
class GeoJsonRoute(
    private val map: MapLibreMap,
    private val mapView: MapView,
    private val path: RoutePath,
    private val destination: LatLng,
) {
    var splitLine = true
        set(value) {
            if (field == value) {
                return
            }
            field = value
            reapplyDisplayedGeometry()
        }

    var displayedPointCount = 0
        private set

    /** True while the trim is interpolating toward the latest location sample. */
    val isTrimAnimating: Boolean
        get() = trimAnimationActive

    private val geoJsonOptions = GeoJsonOptions().withSynchronousUpdate(true)
    private var destinationMarker: Marker? = null
    private var displayedTrimDistance: Double? = null
    private var fullRouteJoinIndex = -1
    private var trimAnimationActive = false
    private var trimFromDistance = 0.0
    private var trimToDistance = 0.0
    private var trimStartedAt = 0L
    private var trimDuration = 0L
    private var trimClockAnchored = false
    private var lastTrimT = 0.0
    private var isUpdatingDisplayedGeometry = false
    private var queuedTrim: PendingTrim? = null

    init {
        installLine(path.coordinates)
        installDestination()
    }

    /**
     * Points the animator should visit after the current, including the target.
     */
    fun animationPoints(targetDistance: Double): List<LatLng> {
        val from = trimStart(targetDistance)
        return path.samplesBetween(from, maxOf(targetDistance, from)).drop(1)
    }

    fun updateDisplayedGeometry(
        currentDistance: Double,
        referenceTime: Long?,
        durationMs: Long,
    ) {
        if (isUpdatingDisplayedGeometry) {
            queuedTrim = PendingTrim(currentDistance, referenceTime, durationMs)
            return
        }
        isUpdatingDisplayedGeometry = true
        try {
            val from = trimStart(currentDistance)
            trimFromDistance = from
            trimToDistance = maxOf(currentDistance, from)
            trimStartedAt = referenceTime ?: SystemClock.elapsedRealtime()
            trimDuration = durationMs
            trimClockAnchored = false
            lastTrimT = 0.0
            // A zero-duration sample snaps the puck on the next frame. getAnimatedValue() is
            // still the previous cut until then, so apply the target now and stay idle.
            trimAnimationActive = (durationMs > 0 && trimToDistance > trimFromDistance)
            applyTrim(if (trimAnimationActive) from else trimToDistance, force = false)
        } finally {
            isUpdatingDisplayedGeometry = false
            flushQueuedTrim()
        }
    }

    fun tickDisplayedGeometry(time: Long = SystemClock.elapsedRealtime()) {
        if (!trimAnimationActive || isUpdatingDisplayedGeometry) {
            return
        }
        isUpdatingDisplayedGeometry = true
        try {
            val puck = currentPuck()
            if (puck != null && trimToDistance > trimFromDistance) {
                val distance = path.distanceNearest(puck, trimFromDistance, trimToDistance)
                applyTrim(distance, force = false)
                if (trimToDistance - distance <= ARRIVED_METERS) {
                    trimAnimationActive = false
                }
            } else if (!trimClockAnchored) {
                trimStartedAt = time
                trimClockAnchored = true
                lastTrimT = 0.0
                applyTrim(trimFromDistance, force = false)
            } else {
                applyTrim(trimDistance(time), force = false)
                if (trimDuration <= 0 || time - trimStartedAt >= trimDuration) {
                    trimAnimationActive = false
                }
            }
        } finally {
            isUpdatingDisplayedGeometry = false
            flushQueuedTrim()
        }
    }

    /** Puts the current line back after a style reload removes style sources. */
    fun reinstallDisplayedLine() {
        val style = map.style ?: return
        removeLineLayers(style)
        fullRouteJoinIndex = -1
        installDestination()
        val distance = displayedTrimDistance
        if (distance != null) {
            applyTrim(distance, force = true)
        } else {
            installLine(path.coordinates)
        }
    }

    fun reapplyDisplayedGeometry() {
        if (isUpdatingDisplayedGeometry) {
            return
        }
        isUpdatingDisplayedGeometry = true
        try {
            applyTrim(displayedTrimDistance ?: 0.0, force = true)
        } finally {
            isUpdatingDisplayedGeometry = false
        }
    }

    fun unload() {
        trimAnimationActive = false
        displayedTrimDistance = null
        queuedTrim = null
        fullRouteJoinIndex = -1
        map.style?.let { removeLineLayers(it) }
        destinationMarker?.let { marker ->
            map.removeAnnotation(marker)
        }
        destinationMarker = null
    }

    private fun flushQueuedTrim() {
        val pending = queuedTrim ?: return
        queuedTrim = null
        updateDisplayedGeometry(pending.distance, pending.referenceTime, pending.durationMs)
    }

    private fun trimStart(targetDistance: Double): Double {
        val target = targetDistance.coerceIn(0.0, path.totalDistance)
        val displayed = displayedTrimDistance ?: return target
        val puck = currentPuck() ?: return displayed
        val start = minOf(trimFromDistance, displayed)
        val end = maxOf(trimToDistance, target, displayed)
        return path.distanceNearest(puck, start, end)
    }

    /** Animated puck position, or null before the location component has a fix. */
    private fun currentPuck(): LatLng? {
        val component = map.locationComponent
        if (!component.isLocationComponentActivated) {
            return null
        }
        return component.userLocation
    }

    private fun applyTrim(distance: Double, force: Boolean) {
        val clamped = distance.coerceIn(0.0, path.totalDistance)
        if (splitLine) {
            applySplitTrim(clamped, force)
        } else {
            applyWholeTrim(clamped, force)
        }
    }

    private fun applySplitTrim(distance: Double, force: Boolean) {
        val style = map.style ?: return
        val join = joinIndex(distance)
        val expectsBody = path.coordinates.size - join >= 2
        val headMissing = style.getSource(SOURCE_ID) == null
        val bodyMissing = expectsBody && style.getSource(BODY_SOURCE_ID) == null
        val headChanged = force || headMissing || displayedTrimDistance != distance
        val bodyChanged = force || bodyMissing || join != fullRouteJoinIndex
        if (!headChanged && !bodyChanged) {
            return
        }

        // Overlap the body by a couple of pixels so the two butt caps do not leave a seam.
        // The join itself follows the trim distance only; the overlap must not pull it backward
        // when meters-per-pixel changes with tilt.
        val joinDistance = path.prefixDistance.getOrElse(join) { path.totalDistance }
        val headEnd = minOf(path.totalDistance, joinDistance + leadMeters())
        val head = path.coordinatesAlong(distance, headEnd)
        displayedTrimDistance = distance
        if (headChanged) {
            ensureLine(head, SOURCE_ID, LAYER_ID, style)
        }
        if (bodyChanged) {
            fullRouteJoinIndex = join
            val body = if (join < path.coordinates.size) {
                path.coordinates.subList(join, path.coordinates.size).toList()
            } else {
                emptyList()
            }
            ensureLine(body, BODY_SOURCE_ID, BODY_LAYER_ID, style)
        }
        displayedPointCount = head.size + maxOf(0, path.coordinates.size - join)
    }

    /**
     * First vertex strictly after [distance]. Once a vertex is consumed it stays consumed,
     * even if a later sample asks for an earlier join.
     */
    private fun joinIndex(distance: Double): Int {
        val last = path.coordinates.lastIndex
        if (last < 1) {
            return 0
        }
        val nextVertex = minOf(path.vertexIndex(distance) + 1, last)
        return maxOf(nextVertex, fullRouteJoinIndex)
    }

    private fun leadMeters(): Double {
        val minLeading = minOf(metersPerPixel() * 2, 2.0)
        return maxOf(minLeading, 0.05)
    }

    private fun applyWholeTrim(distance: Double, force: Boolean) {
        val style = map.style ?: return
        removeBodyLine(style)
        fullRouteJoinIndex = -1
        if (!force && displayedTrimDistance == distance && style.getSource(SOURCE_ID) != null) {
            return
        }
        val coordinates = path.coordinatesAlong(distance, path.totalDistance)
        displayedTrimDistance = distance
        displayedPointCount = coordinates.size
        ensureLine(coordinates, SOURCE_ID, LAYER_ID, style)
    }

    private fun trimDistance(time: Long): Double {
        val interpolated = if (trimDuration <= 0 || trimToDistance <= trimFromDistance) {
            trimToDistance
        } else {
            var t = ((time - trimStartedAt).toDouble() / trimDuration.toDouble()).coerceIn(0.0, 1.0)
            if (t < lastTrimT) {
                t = lastTrimT
            }
            lastTrimT = t
            trimFromDistance + (trimToDistance - trimFromDistance) * t
        }
        return maxOf(interpolated, displayedTrimDistance ?: interpolated)
    }

    private fun metersPerPixel(): Double {
        if (mapView.width <= 0 || mapView.height <= 0) {
            return 1.0
        }
        val centerX = mapView.width / 2f
        val centerY = mapView.height / 2f
        val offset = 10f
        val start = map.projection.fromScreenLocation(PointF(centerX - offset, centerY - offset))
        val end = map.projection.fromScreenLocation(PointF(centerX + offset, centerY + offset))
        val meters = start.distanceTo(end) / hypot(2.0 * offset, 2.0 * offset)
        return if (meters > 0 && meters.isFinite()) meters else 1.0
    }

    private fun installLine(coordinates: List<LatLng>) {
        val style = map.style ?: return
        displayedPointCount = coordinates.size
        ensureLine(coordinates, SOURCE_ID, LAYER_ID, style)
    }

    private fun installDestination() {
        destinationMarker?.let { map.removeAnnotation(it) }
        destinationMarker = map.addMarker(MarkerOptions().position(destination))
    }

    private fun ensureLine(coordinates: List<LatLng>, sourceId: String, layerId: String, style: Style) {
        if (coordinates.size < 2) {
            removeLine(sourceId, layerId, style)
            return
        }
        val feature = feature(coordinates)
        val existing = style.getSource(sourceId) as? GeoJsonSource
        if (existing != null) {
            existing.setGeoJson(feature)
            if (style.getLayer(layerId) == null) {
                style.addLineLayer(sourceId, layerId)
            }
            return
        }
        style.addSource(GeoJsonSource(sourceId, feature, geoJsonOptions))
        style.addLineLayer(sourceId, layerId)
    }

    private fun Style.addLineLayer(sourceId: String, layerId: String) {
        if (getLayer(layerId) != null) {
            return
        }
        val layer = LineLayer(layerId, sourceId).withProperties(
            PropertyFactory.lineWidth(
                Expression.interpolate(
                    Expression.linear(),
                    Expression.zoom(),
                    Expression.stop(10, 12),
                    Expression.stop(13, 13.5),
                    Expression.stop(16, 16.5),
                    Expression.stop(19, 33),
                    Expression.stop(22, 42),
                )
            ),
            PropertyFactory.lineColor(Color.BLUE),
            PropertyFactory.lineOpacity(1f),
            PropertyFactory.lineCap(Property.LINE_CAP_BUTT),
            PropertyFactory.lineJoin(Property.LINE_JOIN_ROUND),
        )
        val lastLine = layers.lastOrNull { it is LineLayer }
        if (lastLine != null) {
            addLayerAbove(layer, lastLine.id)
        } else {
            addLayer(layer)
        }
    }

    private fun feature(coordinates: List<LatLng>): Feature {
        val points = coordinates.map { Point.fromLngLat(it.longitude, it.latitude) }
        return Feature.fromGeometry(LineString.fromLngLats(points))
    }

    private fun removeBodyLine(style: Style) {
        if (style.getSource(BODY_SOURCE_ID) == null && style.getLayer(BODY_LAYER_ID) == null) {
            return
        }
        removeLine(BODY_SOURCE_ID, BODY_LAYER_ID, style)
        fullRouteJoinIndex = -1
    }

    private fun removeLineLayers(style: Style) {
        removeLine(BODY_SOURCE_ID, BODY_LAYER_ID, style)
        removeLine(SOURCE_ID, LAYER_ID, style)
    }

    private fun removeLine(sourceId: String, layerId: String, style: Style) {
        style.getLayer(layerId)?.let { style.removeLayer(it) }
        style.getSource(sourceId)?.let { style.removeSource(it) }
    }

    private class PendingTrim(
        val distance: Double,
        val referenceTime: Long?,
        val durationMs: Long,
    )

    companion object {
        private const val SOURCE_ID = "routeSource"
        private const val LAYER_ID = "routeLayer"
        private const val BODY_SOURCE_ID = "routeBodySource"
        private const val BODY_LAYER_ID = "routeBodyLayer"
        private const val ARRIVED_METERS = 0.05
    }
}
