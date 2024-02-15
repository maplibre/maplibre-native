package org.maplibre.android.testapp.utils

import java.util.ArrayList

class FpsStore {
    private val fpsValues = ArrayList<Double>(100000)

    fun add(fps: Double) {
        fpsValues.add(fps)
    }

    fun reset() {
        fpsValues.clear()
    }

    fun low1p(): Double {
        fpsValues.sort()
        return fpsValues.slice(0..(fpsValues.size / 100)).average()
    }

    fun average(): Double {
        return fpsValues.average()
    }
}

class FrameTimeStore {
    private val timeValues = ArrayList<Double>(100000)

    fun add(time: Double) {
        timeValues.add(time)
    }

    fun reset() {
        timeValues.clear()
    }

    fun low1p(): Double {
        timeValues.sort()
        return timeValues.slice((99 * timeValues.size / 100)..timeValues.size - 1).average()
    }

    fun average(): Double {
        return timeValues.average()
    }
}

/**
 * Result of single benchmark run
 */
data class BenchmarkResult(val average: Double, val low1p: Double)

data class BenchmarkResults(
    var resultsPerStyle: MutableMap<String, List<BenchmarkResult>> = mutableMapOf<String, List<BenchmarkResult>>().withDefault { emptyList() })
{

    fun addResult(styleName: String, fpsStore: FpsStore) {
        val newResults = resultsPerStyle.getValue(styleName).plus(
            BenchmarkResult(
                fpsStore.average(),
                fpsStore.low1p()
            )
        )
        resultsPerStyle[styleName] = newResults
    }

    fun addResult(styleName: String, frameTimeStore: FrameTimeStore) {
        val newResults = resultsPerStyle.getValue(styleName).plus(
            BenchmarkResult(
                frameTimeStore.average(),
                frameTimeStore.low1p()
            )
        )
        resultsPerStyle[styleName] = newResults
    }
}