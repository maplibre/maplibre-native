package org.maplibre.android.testapp.utils

import android.app.ActivityManager
import android.content.Context
import android.net.TrafficStats
import android.os.Build
import android.os.Debug
import android.os.StrictMode
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.addJsonObject
import kotlinx.serialization.json.buildJsonObject
import kotlinx.serialization.json.putJsonArray
import org.maplibre.android.BuildConfig.GIT_REVISION
import org.maplibre.android.testapp.BuildConfig
import java.io.BufferedReader
import java.io.InputStreamReader
import java.util.ArrayList
import java.util.Timer
import kotlin.concurrent.fixedRateTimer

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
    val thermalState: Int,
    val advancedMetrics: BenchmarkAdvancedMetrics?
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

                    run.second.advancedMetrics?.let { metrics ->
                        put("cpu", buildJsonObject {
                            put("min", JsonPrimitive(metrics.min.cpu.value))
                            put("max", JsonPrimitive(metrics.max.cpu.value))
                            put("avg", JsonPrimitive(metrics.avg.cpu.value))
                        })

                        put("memory", buildJsonObject {
                            // convert to MB
                            put("min", JsonPrimitive(metrics.min.memory.value / 1024) )
                            put("max", JsonPrimitive(metrics.max.memory.value / 1024))
                            put("avg", JsonPrimitive(metrics.avg.memory.value / 1024))
                        })

                        // convert to MB
                        put("traffic", JsonPrimitive(metrics.traffic / 1024))
                    }
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

class BenchmarkAdvancedMetrics {
    class Metric<T : Comparable<T>>(var value: T) {

        fun min(newValue: Metric<T>) {
            if (value > newValue.value) {
                value = newValue.value
            }
        }

        fun max(newValue: Metric<T>) {
            if (value < newValue.value) {
                value = newValue.value
            }
        }

        @Suppress("UNCHECKED_CAST")
        operator fun plusAssign(newValue: Metric<T>) {
            when (value) {
                is Int -> value = ((value as Int) + (newValue.value as Int)) as T
                is Long -> value = ((value as Long) + (newValue.value as Long)) as T
                is Float -> value = ((value as Float) + (newValue.value as Float)) as T
                is Double -> value = ((value as Double) + (newValue.value as Double)) as T
            }
        }

        @Suppress("UNCHECKED_CAST")
        operator fun div(newValue: Int): Metric<T> {
            return when (value) {
                is Int -> Metric(((value as Int) / newValue) as T)
                is Long -> Metric(((value as Long) / newValue) as T)
                is Float -> Metric(((value as Float) / newValue) as T)
                is Double -> Metric(((value as Double) / newValue) as T)
                else -> this
            }
        }
    }

    class Snapshot (
        var cpu: Metric<Float>,
        var memory: Metric<Long>,
    ) {
        fun min(snapshot: Snapshot) {
            cpu.min(snapshot.cpu)
            memory.min(snapshot.memory)
        }

        fun max(snapshot: Snapshot) {
            cpu.max(snapshot.cpu)
            memory.max(snapshot.memory)
        }

        operator fun plusAssign(snapshot: Snapshot) {
            cpu += snapshot.cpu
            memory += snapshot.memory
        }

        operator fun div(value: Int): Snapshot {
            return Snapshot(
                cpu / value,
                memory / value,
            )
        }
    }

    private var startTime = 0L
    private var stopTime = 0L
    private var trafficStart = 0L
    private var trafficStop = 0L
    private var timer: Timer? = null

    public var min = Snapshot(Metric(Float.MAX_VALUE), Metric(Long.MAX_VALUE))
    public var max = Snapshot(Metric(Float.MIN_VALUE), Metric(Long.MIN_VALUE))
    // track total/avg or keep values for median/low/high?
    public var total = Snapshot(Metric(0.0f), Metric(0))
    public var frameCount = 0
    public val avg: Snapshot get() { return total / frameCount }
    public val time: Long get() { return stopTime - startTime }
    public val traffic: Long get() { return trafficStop - trafficStart }
    public val enabled: Boolean get() { return time > 0.0 }

    @Synchronized public fun start(collectInterval: Long = 1000L) {
        reset()

        startTime = System.currentTimeMillis()
        trafficStart = TrafficStats.getTotalRxBytes()

        timer = fixedRateTimer(
            name = "BenchmarkAdvancedMetrics",
            period = collectInterval
        ) {
            collect()
        }
    }

    @Synchronized public fun stop() {
        timer?.cancel()
        timer = null

        stopTime = System.currentTimeMillis()
        trafficStop = TrafficStats.getTotalRxBytes()
    }

    private fun collect() {
        val snapshot = Snapshot(Metric(getCPU()), Metric(getMemory()))

        min.min(snapshot)
        max.max(snapshot)
        total += snapshot

        ++frameCount
    }

    private fun reset() {
        min = Snapshot(Metric(Float.MAX_VALUE), Metric(Long.MAX_VALUE))
        max = Snapshot(Metric(Float.MIN_VALUE), Metric(Long.MIN_VALUE))
        total = Snapshot(Metric(0.0f), Metric(0))
        frameCount = 0
    }

    public fun toJson(): JsonObject {
        return buildJsonObject {
            put("cpu", buildJsonObject {
                put("min", JsonPrimitive(min.cpu.value))
                put("max", JsonPrimitive(max.cpu.value))
                put("avg", JsonPrimitive(avg.cpu.value))
            })

            put("memory", buildJsonObject {
                put("min", JsonPrimitive(min.memory.value))
                put("max", JsonPrimitive(max.memory.value))
                put("avg", JsonPrimitive(avg.memory.value))
            })

            put("traffic", JsonPrimitive(traffic))
        }
    }

    companion object {
        public fun getCPU(): Float {
            val currentPolicy = StrictMode.allowThreadDiskReads()

            try {
                val pid = android.os.Process.myPid().toString()
                val cores = Runtime.getRuntime().availableProcessors()
                val process = Runtime.getRuntime().exec("top -n 1 -o PID,%CPU")
                val bufferedReader = BufferedReader(InputStreamReader(process.inputStream))
                var line = bufferedReader.readLine()
                while (line != null) {
                    if (line.contains(pid)) {
                        val rawCpu = line.split(" ").last().toFloat()
                        return rawCpu / cores
                    }
                    line = bufferedReader.readLine()
                }
            } catch (e: Exception) {
                return 0.0f
            } finally {
                StrictMode.setThreadPolicy(currentPolicy)
            }
            return 0.0f
        }

        public fun getMemory(): Long {
            val debugMemInfo = Debug.MemoryInfo()
            Debug.getMemoryInfo(debugMemInfo)
            return debugMemInfo.totalPss.toLong()
        }
    }
}
