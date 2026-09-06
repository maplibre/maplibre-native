package org.maplibre.android.style.light

import androidx.annotation.Keep

/**
 * Position of the light source relative to lit (extruded) geometries.
 *
 * The position is constructed out of a radial coordinate, an azimuthal angle and a polar angle.
 * where the radial coordinate indicates the distance from the center of the base of an object to its light, the
 * azimuthal angle indicates the position of the light relative to 0&#xB0; (0&#xB0; when
 * [org.maplibre.android.style.layers.Property.ANCHOR] is set to viewport corresponds to the top of the
 * viewport, or 0&#xB0; when [org.maplibre.android.style.layers.Property.ANCHOR] is set to map corresponds to due
 * north, and degrees proceed clockwise), and polar indicates the height of the light
 * (from 0&#xB0;, directly above, to 180&#xB0;, directly below).
 *
 * Creates a Position from a radial coordinate, an azimuthal angle and a polar angle.
 *
 * @param radialCoordinate the distance from the center of the base of an object to its light
 * @param azimuthalAngle the position of the light relative to 0&#xB0;
 * @param polarAngle the height of the light
 */
class Position(
    @field:Keep private val radialCoordinate: Float,
    @field:Keep private val azimuthalAngle: Float,
    @field:Keep private val polarAngle: Float,
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        val position = other as Position

        if (position.radialCoordinate.compareTo(radialCoordinate) != 0) {
            return false
        }
        if (position.azimuthalAngle.compareTo(azimuthalAngle) != 0) {
            return false
        }
        return position.polarAngle.compareTo(polarAngle) == 0
    }

    override fun hashCode(): Int {
        var result = if (radialCoordinate != +0.0f) radialCoordinate.toBits() else 0
        result = 31 * result + if (azimuthalAngle != +0.0f) azimuthalAngle.toBits() else 0
        result = 31 * result + if (polarAngle != +0.0f) polarAngle.toBits() else 0
        return result
    }

    override fun toString(): String =
        (
            "Position{" +
                "radialCoordinate=" + radialCoordinate +
                ", azimuthalAngle=" + azimuthalAngle +
                ", polarAngle=" + polarAngle +
                '}'
        )

    companion object {
        /**
         * Returns a Position from a radial coordinate, an azimuthal angle and a polar angle
         *
         * @param radialCoordinate the radial coordinate
         * @param azimuthalAngle the azimuthal angle
         * @param polarAngle the polar angle
         * @return the created Position object
         */
        @Keep
        @JvmStatic
        fun fromPosition(
            radialCoordinate: Float,
            azimuthalAngle: Float,
            polarAngle: Float,
        ): Position = Position(radialCoordinate, azimuthalAngle, polarAngle)
    }
}
