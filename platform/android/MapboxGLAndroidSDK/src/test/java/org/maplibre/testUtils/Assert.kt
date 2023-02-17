package org.maplibre.testUtils

const val DELTA_FLOAT: Float = 1e-15f
const val DELTA_DOUBLE: Double = 1e-15

object Assert {
    fun assertEquals(expected: Double, actual: Double) {
        org.junit.Assert.assertEquals(expected, actual, DELTA_DOUBLE)
    }

    fun assertEquals(expected: Float, actual: Float) {
        org.junit.Assert.assertEquals(expected, actual, DELTA_FLOAT)
    }
}
