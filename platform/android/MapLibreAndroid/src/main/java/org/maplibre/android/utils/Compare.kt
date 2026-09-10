package org.maplibre.android.utils

/**
 * Comparisons from std sdk, which aren't available in API level 15 and below
 */
object Compare {
    /**
     * @see java.lang.Integer.compare
     * @param x left side
     * @param y right side
     * @return std compare value
     */
    @JvmStatic
    fun compare(
        x: Int,
        y: Int,
    ): Int =
        if (x < y) {
            -1
        } else if (x == y) {
            0
        } else {
            1
        }

    /**
     * @see java.lang.Boolean.compare
     * @param x left side
     * @param y right side
     * @return std compare value
     */
    @JvmStatic
    fun compare(
        x: Boolean,
        y: Boolean,
    ): Int =
        if (x == y) {
            0
        } else if (x) {
            1
        } else {
            -1
        }
}
