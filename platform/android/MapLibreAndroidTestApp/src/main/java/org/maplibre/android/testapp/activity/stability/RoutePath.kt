package org.maplibre.android.testapp.activity.stability

import org.maplibre.android.geometry.LatLng
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.pow
import kotlin.math.sin
import kotlin.math.sqrt

/**
 * Geodesic distances and linear lat/lng interpolation along each segment.
 */
class RoutePath(val coordinates: List<LatLng>) {
    /** Cumulative distance along the route from the start to coordinate `i`. */
    val prefixDistance: DoubleArray = if (coordinates.size >= 2) {
        DoubleArray(coordinates.size).also { prefix ->
            for (i in 0 until coordinates.size - 1) {
                prefix[i + 1] = prefix[i] + distance(coordinates[i], coordinates[i + 1])
            }
        }
    } else {
        doubleArrayOf(0.0)
    }

    val totalDistance: Double
        get() = prefixDistance.lastOrNull() ?: 0.0

    fun point(distance: Double): LatLng {
        val last = coordinates.lastOrNull() ?: return LatLng(0.0, 0.0)

        return if (distance <= 0) {
            coordinates[0]
        } else if (coordinates.size < 2 || distance >= totalDistance) {
            last
        } else {
            val index = vertexIndex(distance)
            val next = minOf(index + 1, coordinates.size - 1)
            val segmentLength = prefixDistance[next] - prefixDistance[index]
            val fraction = if (segmentLength > 0) (distance - prefixDistance[index]) / segmentLength else 0.0
            interpolate(coordinates[index], coordinates[next], fraction)
        }
    }

    /** Greatest vertex index with [prefixDistance] at most [distance]. */
    fun vertexIndex(distance: Double): Int {
        // The comparator never returns 0, so the result is always negative, indicating the
        // insertion point. The first value greater than [distance] is at `-result - 1`.
        val insertPoint = prefixDistance.asList().binarySearch {
            if (it <= distance) -1 else 1
        }
        return ((-insertPoint - 1) - 1).coerceAtLeast(0)
    }

    /**
     * Polyline along the route from [startDistance] to [endDistance].
     */
    fun coordinatesAlong(startDistance: Double, endDistance: Double): List<LatLng> {
        if (coordinates.size < 2) {
            val point = coordinates.lastOrNull() ?: LatLng(0.0, 0.0)
            return listOf(point, point)
        }
        val start = startDistance.coerceIn(0.0, totalDistance)
        val end = endDistance.coerceIn(start, totalDistance)
        if (start >= totalDistance) {
            val last = coordinates.last()
            return listOf(last, last)
        }

        val remaining = ArrayList<LatLng>()
        remaining.add(point(start))
        var index = vertexIndex(start) + 1
        while (index < coordinates.size && prefixDistance[index] <= end) {
            val vertex = coordinates[index]
            if (distance(remaining.last(), vertex) > DUPLICATE_DIST_METERS) {
                remaining.add(vertex)
            }
            index += 1
        }
        val endPoint = point(end)
        if (distance(remaining.last(), endPoint) > DUPLICATE_DIST_METERS) {
            remaining.add(endPoint)
        }
        if (remaining.size < 2) {
            remaining.add(remaining.last())
        }
        return remaining
    }

    /**
     * Points from [startDistance] to [endDistance]
     */
    fun samplesBetween(startDistance: Double, endDistance: Double): List<LatLng> {
        val start = startDistance.coerceIn(0.0, totalDistance)
        val end = endDistance.coerceIn(start, totalDistance)
        if (coordinates.size < 2) {
            return listOf(point(end))
        }
        val samples = ArrayList<LatLng>()
        samples.add(point(start))
        if (end - start <= DUPLICATE_DIST_METERS) {
            return samples
        }
        var index = vertexIndex(start) + 1
        while (index < coordinates.size && prefixDistance[index] < end - DUPLICATE_DIST_METERS) {
            if (distance(samples.last(), coordinates[index]) > DUPLICATE_DIST_METERS) {
                samples.add(coordinates[index])
            }
            index += 1
        }
        val endPoint = point(end)
        if (distance(samples.last(), endPoint) > DUPLICATE_DIST_METERS) {
            samples.add(endPoint)
        }
        return samples
    }

    /**
     * Distance along the route of the point closest to [target].
     */
    fun distanceNearest(target: LatLng, startDistance: Double, endDistance: Double): Double {
        if (coordinates.size < 2) {
            return 0.0
        }
        val start = startDistance.coerceIn(0.0, totalDistance)
        val end = endDistance.coerceIn(start, totalDistance)
        if (end - start <= DUPLICATE_DIST_METERS) {
            return start
        }

        var index = vertexIndex(start)
        if (index > 0 && prefixDistance[index] <= start + DUPLICATE_DIST_METERS) {
            index -= 1
        }
        val last = minOf(vertexIndex(end), coordinates.lastIndex - 1)
        var bestAlong = start
        var bestSeparation = Double.POSITIVE_INFINITY
        while (index <= last) {
            val from = maxOf(prefixDistance[index], start)
            val to = minOf(prefixDistance[index + 1], end)
            if (to >= from) {
                val (along, separation) = closestOnSegment(target, index, from, to)
                if (separation < bestSeparation) {
                    bestSeparation = separation
                    bestAlong = along
                }
            }
            index += 1
        }
        return bestAlong.coerceIn(start, end)
    }

    private fun closestOnSegment(
        target: LatLng,
        index: Int,
        fromDistance: Double,
        toDistance: Double,
    ): Pair<Double, Double> {
        val segmentLength = prefixDistance[index + 1] - prefixDistance[index]
        val startFraction = if (segmentLength > 0) {
            (fromDistance - prefixDistance[index]) / segmentLength
        } else {
            0.0
        }
        val endFraction = if (segmentLength > 0) {
            (toDistance - prefixDistance[index]) / segmentLength
        } else {
            0.0
        }
        val origin = coordinates[index]
        val destination = coordinates[index + 1]
        val originLat = origin.latitude + (destination.latitude - origin.latitude) * startFraction
        val originLng = origin.longitude + (destination.longitude - origin.longitude) * startFraction
        val destLat = origin.latitude + (destination.latitude - origin.latitude) * endFraction
        val destLng = origin.longitude + (destination.longitude - origin.longitude) * endFraction
        val latitudeSpan = destLat - originLat
        val longitudeSpan = destLng - originLng
        val denominator = latitudeSpan * latitudeSpan + longitudeSpan * longitudeSpan
        val fraction = if (denominator > 0.0) {
            ((target.latitude - originLat) * latitudeSpan + (target.longitude - originLng) * longitudeSpan) / denominator
        } else {
            0.0
        }.coerceIn(0.0, 1.0)
        val latitude = originLat + latitudeSpan * fraction
        val longitude = originLng + longitudeSpan * fraction
        val separation = distance(target, LatLng(latitude, longitude))
        val alongFraction = startFraction + (endFraction - startFraction) * fraction
        return (prefixDistance[index] + alongFraction * segmentLength) to separation
    }

    companion object {
        private const val METERS_PER_RADIAN = 6_373_000.0

        /** Ignore a consecutive points closer than this to avoid a zero-length segment. */
        private const val DUPLICATE_DIST_METERS = 0.001

        fun distance(from: LatLng, to: LatLng): Double {
            val latitudeA = Math.toRadians(from.latitude)
            val longitudeA = Math.toRadians(from.longitude)
            val latitudeB = Math.toRadians(to.latitude)
            val longitudeB = Math.toRadians(to.longitude)
            val haversine = sin((latitudeB - latitudeA) / 2).pow(2) +
                sin((longitudeB - longitudeA) / 2).pow(2) * cos(latitudeA) * cos(latitudeB)
            return 2 * atan2(sqrt(haversine), sqrt(1 - haversine)) * METERS_PER_RADIAN
        }

        fun interpolate(from: LatLng, to: LatLng, fraction: Double): LatLng {
            val t = minOf(1.0, maxOf(0.0, fraction))
            return LatLng(
                from.latitude + (to.latitude - from.latitude) * t,
                from.longitude + (to.longitude - from.longitude) * t
            )
        }

        /** Initial bearing in degrees, wrapped to `[0, 360)`. */
        fun direction(from: LatLng, to: LatLng): Double {
            val latitudeA = Math.toRadians(from.latitude)
            val longitudeA = Math.toRadians(from.longitude)
            val latitudeB = Math.toRadians(to.latitude)
            val longitudeB = Math.toRadians(to.longitude)
            val east = sin(longitudeB - longitudeA) * cos(latitudeB)
            val north = cos(latitudeA) * sin(latitudeB) -
                sin(latitudeA) * cos(latitudeB) * cos(longitudeB - longitudeA)
            val degrees = Math.toDegrees(atan2(east, north))
            return (degrees % 360.0 + 360.0) % 360.0
        }
    }
}
