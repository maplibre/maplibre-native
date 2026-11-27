package org.maplibre.android.testapp.projection

import org.maplibre.android.geometry.CoordinateReferenceSystem
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.atan
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.hypot
import kotlin.math.pow
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.math.tan

/**
 * EPSG:27700, the British National Grid, from WGS 84 through the Helmert transformation, on Ordnance Survey's tile
 * matrix set, where a tile's zoom is its OS level plus [LEVEL_0_ZOOM]. The formulas and constants are those of
 * Ordnance Survey's "A Guide to Coordinate Systems in Great Britain", V3.6 (2020): annex A for the constants, B for
 * Cartesian coordinates, C for the transverse Mercator projection and section 6.6 for the Helmert transformation.
 */
object NationalGrid {
    const val NAME = "EPSG:27700"

    /** The side of an Ordnance Survey level 0 tile in meters. Level 0 is 5 by 6 of them from E -238375, N 1376256. */
    const val LEVEL_0_TILE_SIZE = 229376.0

    /** The tile zoom of OS level 0: the registered tile 0/0/0 is the 8 by 8 square of level 0 tiles around Britain. */
    const val LEVEL_0_ZOOM = 3

    private const val RADIANS_PER_DEGREE = PI / 180
    private const val RADIANS_PER_ARC_SECOND = RADIANS_PER_DEGREE / 3600

    private class Ellipsoid(val a: Double, val b: Double) {
        val e2 = (a * a - b * b) / (a * a)
    }

    private val airy1830 = Ellipsoid(6377563.396, 6356256.909)
    private val grs80 = Ellipsoid(6378137.0, 6356752.3141)

    private class LatLngHeight(val latitude: Double, val longitude: Double, val height: Double)

    /** Latitude and longitude in degrees and height above the ellipsoid in meters to Cartesian coordinates (B2 to B5). */
    private fun cartesianFromLatLngHeight(ellipsoid: Ellipsoid, position: LatLngHeight): DoubleArray {
        val phi = position.latitude * RADIANS_PER_DEGREE
        val lambda = position.longitude * RADIANS_PER_DEGREE
        val nu = ellipsoid.a / sqrt(1 - ellipsoid.e2 * sin(phi) * sin(phi))
        return doubleArrayOf(
            (nu + position.height) * cos(phi) * cos(lambda),
            (nu + position.height) * cos(phi) * sin(lambda),
            ((1 - ellipsoid.e2) * nu + position.height) * sin(phi)
        )
    }

    /**
     * Cartesian coordinates to latitude, longitude and height (B6 to B9), iterating until the latitude moves by less
     * than a micrometer.
     */
    private fun latLngHeightFromCartesian(ellipsoid: Ellipsoid, xyz: DoubleArray): LatLngHeight {
        val latitudeTolerance = 1e-6 / 6378137
        val (x, y, z) = xyz
        val p = hypot(x, y)
        fun nuAt(phi: Double) = ellipsoid.a / sqrt(1 - ellipsoid.e2 * sin(phi) * sin(phi))
        var phi = atan(z / (p * (1 - ellipsoid.e2)))
        do {
            val next = atan((z + ellipsoid.e2 * nuAt(phi) * sin(phi)) / p)
            val change = next - phi
            phi = next
        } while (abs(change) >= latitudeTolerance)
        return LatLngHeight(phi / RADIANS_PER_DEGREE, atan2(y, x) / RADIANS_PER_DEGREE, p / cos(phi) - nuAt(phi))
    }

    /** A seven-parameter Helmert transformation of Cartesian coordinates (section 6.2, equation 3). */
    private class Helmert(val translation: DoubleArray, val scalePpm: Double, val rotationArcSeconds: DoubleArray) {
        fun transform(xyz: DoubleArray): DoubleArray {
            val s = 1 + scalePpm * 1e-6
            val rx = rotationArcSeconds[0] * RADIANS_PER_ARC_SECOND
            val ry = rotationArcSeconds[1] * RADIANS_PER_ARC_SECOND
            val rz = rotationArcSeconds[2] * RADIANS_PER_ARC_SECOND
            val (x, y, z) = xyz
            return doubleArrayOf(
                translation[0] + s * x - rz * y + ry * z,
                translation[1] + rz * x + s * y - rx * z,
                translation[2] - ry * x + rx * y + s * z
            )
        }

        /** The reverse transformation, as section 6.2 gives it: every parameter negated. */
        fun reversed() = Helmert(
            DoubleArray(3) { -translation[it] },
            -scalePpm,
            DoubleArray(3) { -rotationArcSeconds[it] }
        )
    }

    /**
     * WGS 84 or ETRS89, which differ negligibly here, to OSGB36 (section 6.6, table 4). It is up to 3.5 m (95%) from
     * the National Grid's definitive transformation, OSTN15.
     */
    private val etrs89ToOsgb36 = Helmert(
        doubleArrayOf(-446.448, 125.157, -542.060),
        20.4894,
        doubleArrayOf(-0.1502, -0.2470, -0.8421)
    )
    private val osgb36ToEtrs89 = etrs89ToOsgb36.reversed()

    /** The National Grid's transverse Mercator projection of OSGB36 (annexes A.2 and C). */
    private object Projection {
        private val ellipsoid = airy1830
        private const val SCALE = 0.9996012717
        private const val ORIGIN_LATITUDE = 49.0
        private const val CENTRAL_LONGITUDE = -2.0
        private const val FALSE_EASTING = 400000.0
        private const val FALSE_NORTHING = -100000.0

        /** Latitude and longitude in degrees to easting and northing in meters (C1 to C5). */
        fun forward(latitude: Double, longitude: Double): DoubleArray {
            val phi = latitude * RADIANS_PER_DEGREE
            val sinPhi = sin(phi)
            val cosPhi = cos(phi)
            val tan2 = tan(phi) * tan(phi)
            val (nu, rho, eta2) = radii(phi)
            val i = meridianArc(phi) + FALSE_NORTHING
            val ii = nu / 2 * sinPhi * cosPhi
            val iii = nu / 24 * sinPhi * cosPhi.pow(3) * (5 - tan2 + 9 * eta2)
            val iiia = nu / 720 * sinPhi * cosPhi.pow(5) * (61 - 58 * tan2 + tan2 * tan2)
            val iv = nu * cosPhi
            val v = nu / 6 * cosPhi.pow(3) * (nu / rho - tan2)
            val vi = nu / 120 * cosPhi.pow(5) * (5 - 18 * tan2 + tan2 * tan2 + 14 * eta2 - 58 * tan2 * eta2)
            val dLambda = (longitude - CENTRAL_LONGITUDE) * RADIANS_PER_DEGREE
            return doubleArrayOf(
                FALSE_EASTING + iv * dLambda + v * dLambda.pow(3) + vi * dLambda.pow(5),
                i + ii * dLambda * dLambda + iii * dLambda.pow(4) + iiia * dLambda.pow(6)
            )
        }

        /**
         * Easting and northing in meters to latitude and longitude in degrees (C6 to C9), iterating until the
         * northing of the latitude found is within 0.01 mm.
         */
        fun inverse(easting: Double, northing: Double): DoubleArray {
            val northingTolerance = 0.01e-3
            val aF0 = ellipsoid.a * SCALE
            var phi = (northing - FALSE_NORTHING) / aF0 + ORIGIN_LATITUDE * RADIANS_PER_DEGREE
            var residual = northing - FALSE_NORTHING - meridianArc(phi)
            while (abs(residual) >= northingTolerance) {
                phi += residual / aF0
                residual = northing - FALSE_NORTHING - meridianArc(phi)
            }
            val tanPhi = tan(phi)
            val tan2 = tanPhi * tanPhi
            val secPhi = 1 / cos(phi)
            val (nu, rho, eta2) = radii(phi)
            val vii = tanPhi / (2 * rho * nu)
            val viii = tanPhi / (24 * rho * nu.pow(3)) * (5 + 3 * tan2 + eta2 - 9 * tan2 * eta2)
            val ix = tanPhi / (720 * rho * nu.pow(5)) * (61 + 90 * tan2 + 45 * tan2 * tan2)
            val x = secPhi / nu
            val xi = secPhi / (6 * nu.pow(3)) * (nu / rho + 2 * tan2)
            val xii = secPhi / (120 * nu.pow(5)) * (5 + 28 * tan2 + 24 * tan2 * tan2)
            val xiia = secPhi / (5040 * nu.pow(7)) * (61 + 662 * tan2 + 1320 * tan2 * tan2 + 720 * tan2.pow(3))
            val dE = easting - FALSE_EASTING
            val latitude = phi - vii * dE * dE + viii * dE.pow(4) - ix * dE.pow(6)
            val dLambda = x * dE - xi * dE.pow(3) + xii * dE.pow(5) - xiia * dE.pow(7)
            return doubleArrayOf(latitude / RADIANS_PER_DEGREE, CENTRAL_LONGITUDE + dLambda / RADIANS_PER_DEGREE)
        }

        /**
         * The radii of curvature in the prime vertical and along the meridian, scaled by F0, and their ratio less
         * one (C2).
         */
        private fun radii(phi: Double): DoubleArray {
            val w = 1 - ellipsoid.e2 * sin(phi) * sin(phi)
            val nu = ellipsoid.a * SCALE / sqrt(w)
            val rho = ellipsoid.a * SCALE * (1 - ellipsoid.e2) / w.pow(1.5)
            return doubleArrayOf(nu, rho, nu / rho - 1)
        }

        /** The meridional arc from the true origin's latitude to `phi`, scaled by F0 (C3). */
        private fun meridianArc(phi: Double): Double {
            val n = (ellipsoid.a - ellipsoid.b) / (ellipsoid.a + ellipsoid.b)
            val n2 = n * n
            val n3 = n2 * n
            val phi0 = ORIGIN_LATITUDE * RADIANS_PER_DEGREE
            val d = phi - phi0
            val s = phi + phi0
            return ellipsoid.b * SCALE * (
                (1 + n + 5 * n2 / 4 + 5 * n3 / 4) * d -
                    (3 * n + 3 * n2 + 21 * n3 / 8) * sin(d) * cos(s) +
                    (15 * n2 / 8 + 15 * n3 / 8) * sin(2 * d) * cos(2 * s) -
                    35 * n3 / 24 * sin(3 * d) * cos(3 * s)
                )
        }
    }

    /** A WGS 84 latitude and longitude at sea level to National Grid easting and northing. */
    fun eastingNorthingFromWgs84(latitude: Double, longitude: Double): DoubleArray {
        val osgb36 = latLngHeightFromCartesian(
            airy1830,
            etrs89ToOsgb36.transform(cartesianFromLatLngHeight(grs80, LatLngHeight(latitude, longitude, 0.0)))
        )
        return Projection.forward(osgb36.latitude, osgb36.longitude)
    }

    /** A National Grid easting and northing to WGS 84 latitude and longitude. */
    fun wgs84FromEastingNorthing(easting: Double, northing: Double): DoubleArray {
        val (latitude, longitude) = Projection.inverse(easting, northing)
        val wgs84 = latLngHeightFromCartesian(
            grs80,
            osgb36ToEtrs89.transform(cartesianFromLatLngHeight(airy1830, LatLngHeight(latitude, longitude, 0.0)))
        )
        return doubleArrayOf(wgs84.latitude, wgs84.longitude)
    }

    val coordinateReferenceSystem = CoordinateReferenceSystem(
        NAME,
        CoordinateReferenceSystem.TileMatrixSet(-238375.0, 1376256.0, LEVEL_0_TILE_SIZE * (1 shl LEVEL_0_ZOOM)),
        object : CoordinateReferenceSystem.Projection {
            override fun forward(lngLats: DoubleArray, positions: DoubleArray, count: Int) {
                for (i in 0 until count) {
                    val (easting, northing) = eastingNorthingFromWgs84(lngLats[2 * i + 1], lngLats[2 * i])
                    positions[2 * i] = easting
                    positions[2 * i + 1] = northing
                }
            }

            override fun inverse(positions: DoubleArray, lngLats: DoubleArray, count: Int) {
                for (i in 0 until count) {
                    val (latitude, longitude) = wgs84FromEastingNorthing(positions[2 * i], positions[2 * i + 1])
                    lngLats[2 * i] = longitude
                    lngLats[2 * i + 1] = latitude
                }
            }
        }
    )
}
