package org.maplibre.android.utils

// Make converting from Java to Kotlin easier
fun Double.Companion.isNaN(d: Double) = d.isNaN()
fun Float.Companion.isNaN(f: Float) = f.isNaN()
fun Double.Companion.isInfinite(d: Double) = d.isInfinite()
fun Float.Companion.isInfinite(f: Double) = f.isInfinite()
