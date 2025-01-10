package org.maplibre.android.testapp.utils

import android.os.Build
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.addJsonObject
import kotlinx.serialization.json.buildJsonArray
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.putJsonArray
import org.maplibre.android.BuildConfig.GIT_REVISION
import org.maplibre.android.testapp.BuildConfig
import java.util.ArrayList

data class BenchmarkInputData(
    val styleNames: List<String>,
    val styleURLs: List<String>,
) {
    init {
        if (styleNames.size != styleURLs.size)
            throw Error("Different size: styleNames=$styleNames, styleURLs=$styleURLs")
    }
}

data class BenchmarkRun(
    val styleName: String,
    val styleURL: String,
    val syncRendering: Boolean,
    val duration: Int
)

data class BenchmarkRunResult(
    val fps: Double,
    val encodingTimeStore: FrameTimeStore,
    val renderingTimeStore: FrameTimeStore,
    val thermalState: Int
)

data class BenchmarkResult (
    var runs: ArrayList<Pair<BenchmarkRun, BenchmarkRunResult>>
)

//@SuppressLint("NewApi")
fun jsonPayload(benchmarkResult: BenchmarkResult): JsonObject {
    return buildJsonObject {
        putJsonArray("results") {
            for (run in benchmarkResult.runs) {
                addJsonObject {
                    put("styleName", JsonPrimitive(run.first.styleName))
                    put("syncRendering", JsonPrimitive(run.first.syncRendering))
                    put("thermalState", JsonPrimitive(run.second.thermalState))
                    put("fps", JsonPrimitive(run.second.fps))
                    put("avgEncodingTime", JsonPrimitive(run.second.encodingTimeStore.average()))
                    put("avgRenderingTime", JsonPrimitive(run.second.renderingTimeStore.average()))
                    put("low1pEncodingTime", JsonPrimitive(run.second.encodingTimeStore.low1p()))
                    put("low1pRenderingTime", JsonPrimitive(run.second.renderingTimeStore.low1p()))
                }
            }
        }
        put("deviceManufacturer", JsonPrimitive(Build.MANUFACTURER))
        put("model", JsonPrimitive(Build.MODEL))
        put("renderer", JsonPrimitive(BuildConfig.FLAVOR))
        put("debugBuild", JsonPrimitive(BuildConfig.DEBUG))
        put("gitRevision", JsonPrimitive(GIT_REVISION))
        put("timestamp", JsonPrimitive(System.currentTimeMillis()))
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
        return timeValues.slice((99 * timeValues.size / 100)..<timeValues.size).average()
    }

    fun average(): Double {
        return timeValues.average()
    }
}
