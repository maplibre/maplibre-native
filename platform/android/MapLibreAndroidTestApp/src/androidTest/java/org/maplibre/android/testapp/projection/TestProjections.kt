package org.maplibre.android.testapp.projection

import org.maplibre.android.geometry.CoordinateReferenceSystem
import kotlin.math.PI
import kotlin.math.atan
import kotlin.math.atan2
import kotlin.math.cos
import kotlin.math.hypot
import kotlin.math.sin
import kotlin.math.tan

/** Projections for the tests, the same as core's in `test/include/mln/test/crs.hpp` where they share a name. */
object TestProjections {
    private const val DEGREES = 180 / PI

    /**
     * GL JS's test polar stereographic projection: EPSG:3413's true scale at 70 N and central meridian at 45 W, on a
     * sphere, with EPSG:3413's tile matrix set.
     */
    fun polarStereographic(name: String): CoordinateReferenceSystem {
        val scale = 6378137 * (1 + sin(70 / DEGREES))
        val centralLongitude = -45.0
        return CoordinateReferenceSystem(
            name,
            CoordinateReferenceSystem.TileMatrixSet(-4194304.0, 4194304.0, 8388608.0),
            object : CoordinateReferenceSystem.Projection {
                override fun forward(lngLats: DoubleArray, positions: DoubleArray, count: Int) {
                    for (i in 0 until count) {
                        val rho = scale * tan(PI / 4 - lngLats[2 * i + 1] / DEGREES / 2)
                        val angle = (lngLats[2 * i] - centralLongitude) / DEGREES
                        positions[2 * i] = rho * sin(angle)
                        positions[2 * i + 1] = -rho * cos(angle)
                    }
                }

                override fun inverse(positions: DoubleArray, lngLats: DoubleArray, count: Int) {
                    for (i in 0 until count) {
                        val x = positions[2 * i]
                        val y = positions[2 * i + 1]
                        lngLats[2 * i] = centralLongitude + atan2(x, -y) * DEGREES
                        lngLats[2 * i + 1] = 90 - 2 * atan(hypot(x, y) / scale) * DEGREES
                    }
                }
            }
        )
    }

    /** The longitudes between which [banded]'s forward has no position. */
    const val BAND_WEST = 15.0
    const val BAND_EAST = 25.0

    /**
     * Identity coordinates over the world, whose forward has no position for a longitude between [BAND_WEST] and
     * [BAND_EAST]: it writes NaN there, or throws for the whole call if [throws] is set.
     */
    fun banded(name: String, throws: Boolean): CoordinateReferenceSystem = CoordinateReferenceSystem(
        name,
        CoordinateReferenceSystem.TileMatrixSet(-180.0, 180.0, 360.0),
        object : CoordinateReferenceSystem.Projection {
            override fun forward(lngLats: DoubleArray, positions: DoubleArray, count: Int) {
                for (i in 0 until count) {
                    val inBand = lngLats[2 * i] > BAND_WEST && lngLats[2 * i] < BAND_EAST
                    check(!(inBand && throws)) { "No position for longitude ${lngLats[2 * i]}" }
                    positions[2 * i] = if (inBand) Double.NaN else lngLats[2 * i]
                    positions[2 * i + 1] = if (inBand) Double.NaN else lngLats[2 * i + 1]
                }
            }

            override fun inverse(positions: DoubleArray, lngLats: DoubleArray, count: Int) {
                positions.copyInto(lngLats, endIndex = 2 * count)
            }
        }
    )
}
