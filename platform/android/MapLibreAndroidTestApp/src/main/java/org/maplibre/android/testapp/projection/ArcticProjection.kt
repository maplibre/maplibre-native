package org.maplibre.android.testapp.projection

import org.maplibre.android.geometry.CoordinateReferenceSystem
import kotlin.math.PI
import kotlin.math.atan
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.hypot
import kotlin.math.pow
import kotlin.math.sin
import kotlin.math.sqrt
import kotlin.math.tan

/**
 * EPSG:3413, the polar stereographic projection of the WGS 84 ellipsoid with true scale at 70 N and 45 W at the
 * bottom, on the tile matrix set NASA GIBS serves it in. The formulas are Snyder's (Map Projections: A Working Manual,
 * 1987, equations 21-33, 21-34, 7-9 and 3-5).
 */
object ArcticProjection {
    const val NAME = "EPSG:3413"

    private const val SEMI_MAJOR_AXIS = 6378137.0
    private const val FLATTENING = 1 / 298.257223563
    private const val CENTRAL_LONGITUDE = -45.0
    private const val TRUE_SCALE_LATITUDE = 70.0
    private const val RADIANS_PER_DEGREE = PI / 180

    private const val E2 = FLATTENING * (2 - FLATTENING)
    private val e = sqrt(E2)

    /** Snyder's t, the conformal latitude term, for a latitude in radians. */
    fun conformalTerm(latitude: Double): Double {
        val sinLatitude = sin(latitude)
        return tan(PI / 4 - latitude / 2) / ((1 - e * sinLatitude) / (1 + e * sinLatitude)).pow(e / 2)
    }

    private val rhoPerT = run {
        val trueScaleLatitude = TRUE_SCALE_LATITUDE * RADIANS_PER_DEGREE
        val sinTrueScaleLatitude = sin(trueScaleLatitude)
        val m = cos(trueScaleLatitude) / sqrt(1 - E2 * sinTrueScaleLatitude * sinTrueScaleLatitude)
        SEMI_MAJOR_AXIS * m / conformalTerm(trueScaleLatitude)
    }

    // --8<-- [start:coordinateReferenceSystem]
    val coordinateReferenceSystem = CoordinateReferenceSystem(
        NAME,
        CoordinateReferenceSystem.TileMatrixSet(-4194304.0, 4194304.0, 8388608.0),
        object : CoordinateReferenceSystem.Projection {
            override fun forward(lngLats: DoubleArray, positions: DoubleArray, count: Int) {
                for (i in 0 until count) {
                    val rho = rhoPerT * conformalTerm(lngLats[2 * i + 1] * RADIANS_PER_DEGREE)
                    val angle = (lngLats[2 * i] - CENTRAL_LONGITUDE) * RADIANS_PER_DEGREE
                    positions[2 * i] = rho * sin(angle)
                    positions[2 * i + 1] = -rho * cos(angle)
                }
            }

            override fun inverse(positions: DoubleArray, lngLats: DoubleArray, count: Int) {
                val e4 = E2 * E2
                val e6 = e4 * E2
                val e8 = e6 * E2
                for (i in 0 until count) {
                    val x = positions[2 * i]
                    val y = positions[2 * i + 1]
                    val chi = PI / 2 - 2 * atan(hypot(x, y) / rhoPerT)
                    val latitude = chi + (E2 / 2 + 5 * e4 / 24 + e6 / 12 + 13 * e8 / 360) * sin(2 * chi) +
                        (7 * e4 / 48 + 29 * e6 / 240 + 811 * e8 / 11520) * sin(4 * chi) +
                        (7 * e6 / 120 + 81 * e8 / 1120) * sin(6 * chi) +
                        (4279 * e8 / 161280) * sin(8 * chi)
                    lngLats[2 * i] = CENTRAL_LONGITUDE + atan2(x, -y) / RADIANS_PER_DEGREE
                    lngLats[2 * i + 1] = latitude / RADIANS_PER_DEGREE
                }
            }
        }
    )
    // --8<-- [end:coordinateReferenceSystem]
}
