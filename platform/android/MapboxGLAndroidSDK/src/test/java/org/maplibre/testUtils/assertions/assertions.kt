package org.maplibre.testUtils.assertions

const val DELTA_FLOAT: Float = 1e-15f
const val DELTA_DOUBLE: Double = 1e-15

fun assertEquals(expected: Double, actual: Double) {
    org.junit.Assert.assertEquals(expected, actual, DELTA_DOUBLE)
}

fun assertEquals(expected: Float, actual: Float) {
    org.junit.Assert.assertEquals(expected, actual, DELTA_FLOAT)
}