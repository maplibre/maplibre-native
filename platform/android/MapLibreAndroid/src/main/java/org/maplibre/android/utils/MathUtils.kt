package org.maplibre.android.utils

// TODO Remove this class if we finally include it within MAS 3.x (GeoJSON)
object MathUtils {
    /**
     * Test a value in specified range, returning minimum if it's below, and maximum if it's above
     *
     * @param value Value to test
     * @param min   Minimum value of range
     * @param max   Maximum value of range
     * @return value if it's between min and max, min if it's below, max if it's above
     */
    @JvmStatic
    fun clamp(
        value: Double,
        min: Double,
        max: Double,
    ): Double = Math.max(min, Math.min(max, value))

    /**
     * Test a value in specified range, returning minimum if it's below, and maximum if it's above
     *
     * @param value Value to test
     * @param min   Minimum value of range
     * @param max   Maximum value of range
     * @return value if it's between min and max, min if it's below, max if it's above
     */
    @JvmStatic
    fun clamp(
        value: Float,
        min: Float,
        max: Float,
    ): Float = Math.max(min, Math.min(max, value))

    /**
     * Constrains value to the given range (including min, excluding max) via modular arithmetic.
     *
     * Same formula as used in Core GL (wrap.hpp)
     * std::fmod((std::fmod((value - min), d) + d), d) + min;
     *
     * @param value Value to wrap
     * @param min   Minimum value
     * @param max   Maximum value
     * @return Wrapped value
     */
    @JvmStatic
    fun wrap(
        value: Double,
        min: Double,
        max: Double,
    ): Double {
        val delta = max - min

        val firstMod = (value - min) % delta
        val secondMod = (firstMod + delta) % delta

        return secondMod + min
    }

    /**
     * Scale a value from an arbitrary range to a normalized range.
     *
     * @param x              The value to be normalized.
     * @param dataLow        lowest expected value from a data set
     * @param dataHigh       highest expected value from a data set
     * @param normalizedLow  normalized lowest value
     * @param normalizedHigh normalized highest value
     * @return The result of the normalization.
     */
    @JvmStatic
    fun normalize(
        x: Double,
        dataLow: Double,
        dataHigh: Double,
        normalizedLow: Double,
        normalizedHigh: Double,
    ): Double = ((x - dataLow) / (dataHigh - dataLow)) * (normalizedHigh - normalizedLow) + normalizedLow
}
