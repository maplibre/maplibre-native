package org.maplibre.android.testapp.activity.events

import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.RenderingStats
import java.lang.reflect.Field
import java.util.Timer
import kotlin.concurrent.fixedRateTimer

/**
 * Example class that tracks rendering statistics values (based on `RenderingStats` fields) and offers:
 * - periodic reports with min/max/average values
 * - threshold exceeded triggers
 * @see RenderingStats
 */
class RenderStatsTracker {
    private var min = RenderingStats()
    private var max = RenderingStats()
    private var total = RenderingStats()
    private var frameCount = 0

    private var reportTimer: Timer? = null
    private var reportFields = RenderingStats::class.java.fields
    private var reportListener: ((RenderingStats, RenderingStats, RenderingStats) -> Unit)? = null

    private var thresholds = HashMap<Int, Number>()
    private var thresholdExceededListener: ((HashMap<String, Number>,RenderingStats) -> Unit)? = null

    /**
     * Set fields to be tracked and updated every frame. An empty list will track all fields.
     * The report listener will provide `RenderingStats` objects with the tracked values.
     */
    @Synchronized fun setReportFields(values: List<String>?) {
        if (values == null) {
            reportFields = RenderingStats::class.java.fields
            return
        }

        reportFields = values.map { RenderingStats::class.java.getField(it) }.toTypedArray()
    }

    /**
     * Callback that provides min/max/average values for the configured interval (`start(interval)`)
     */
    @Synchronized fun setReportListener(listener: ((RenderingStats, RenderingStats, RenderingStats) -> Unit)?) {
        if (reportFields.isEmpty()) {
            setReportFields(null)
        }

        reportListener = listener
    }

    /**
     * Set the thresholds to be checked
     */
    @Synchronized fun setThresholds(values: HashMap<String, Number>) {
        thresholds.clear()
        val fields = RenderingStats::class.java.fields

        // cache index instead of name
        values.map { value ->
            val index = fields.indexOfFirst { it.name == value.key }
            if (index != -1) {
                thresholds[index] = value.value
            }
        }
    }

    /**
     * Callback that provides the exceeded threshold values in the last frame.
     */
    @Synchronized fun setThresholdExceededListener(listener: ((HashMap<String, Number>,RenderingStats) -> Unit)?) {
        thresholdExceededListener = listener
    }

    /**
     * Start periodic reports every `interval` seconds
     */
    @Synchronized fun startReports(interval: Long) {
        reset()

        reportTimer = fixedRateTimer(
            name = "RenderStatsReportTimer",
            initialDelay = interval * 1000L,
            period = interval * 1000L
        ) {
            reportListener?.invoke(min, max, getAvg())
            reset()
        }
    }

    /**
     * Stop periodic reports. Must be called before the object is destroyed
     */
    @Synchronized fun stopReports() {
        reportTimer?.cancel()
        reportTimer = null
    }

    /**
     * Add the last frame data to the report.
     * @see MapView.OnDidFinishRenderingFrameWithStatsListener(fully: Boolean, stats: RenderingStats)
     */
    @Synchronized fun addFrame(frameStats: RenderingStats) {
        updateReportValues(frameStats)
        checkThresholds(frameStats)
    }

    private fun updateReportValues(frameStats: RenderingStats) {
        if (reportListener == null || reportTimer == null) {
            return
        }

        ++frameCount;

        reportFields.map { field: Field ->
            when (val frameValue = field.get(frameStats)) {
                is Int -> {
                    if (field.getInt(min) > frameValue) {
                        field.setInt(min, frameValue)
                    }

                    if (field.getInt(max) < frameValue) {
                        field.setInt(max, frameValue)
                    }

                    field.setInt(total, frameValue + field.getInt(total))
                }

                is Long -> {
                    if (field.getLong(min) > frameValue) {
                        field.setLong(min, frameValue)
                    }

                    if (field.getLong(max) < frameValue) {
                        field.setLong(max, frameValue)
                    }

                    field.setLong(total, frameValue + field.getLong(total))
                }

                is Double -> {
                    if (field.getDouble(min) > frameValue) {
                        field.setDouble(min, frameValue)
                    }

                    if (field.getDouble(max) < frameValue) {
                        field.setDouble(max, frameValue)
                    }

                    field.setDouble(total, frameValue + field.getDouble(total))
                }

                else -> {}
            }
        }
    }

    private fun checkThresholds(frameStats: RenderingStats) {
        if (thresholdExceededListener == null && thresholds.isEmpty()) {
            return
        }

        val fields = RenderingStats::class.java.fields
        val exceededValues = HashMap<String, Number>()

        thresholds.map {
            when (val frameValue = fields[it.key].get(frameStats)) {
                is Int -> if (frameValue > it.value as Int) exceededValues[fields[it.key].name] = frameValue
                is Long -> if (frameValue > it.value as Long) exceededValues[fields[it.key].name] = frameValue
                is Double -> if (frameValue > it.value as Double) exceededValues[fields[it.key].name] = frameValue
                else -> {}
            }
        }

        if (exceededValues.isNotEmpty()) {
            thresholdExceededListener?.invoke(exceededValues, frameStats)
        }
    }

    private fun getAvg(): RenderingStats {
        val avg = RenderingStats()

        if (frameCount == 0) {
            return avg;
        }

        reportFields.map { field: Field ->
            when (val value = field.get(total)) {
                is Int -> field.setInt(avg, value / frameCount)
                is Long -> field.setLong(avg, value / frameCount)
                is Double -> field.setDouble(avg, value / frameCount)
                else -> {}
            }
        }

        return avg
    }

    private fun reset() {
        reportFields.map { field: Field -> field.set(min, Int.MAX_VALUE) }
        reportFields.map { field: Field -> field.set(max, -Int.MAX_VALUE) }
        reportFields.map { field: Field -> field.set(total, 0) }

        frameCount = 0
    }
}

fun RenderingStats.nonZeroValuesString(): String {
    return this::class.java.fields
        .filter { (it.get(this) as? Number)?.toDouble() != 0.0 }
        .joinToString(", ") { field ->
            "(${field.name}=${field.get(this)})"
        }
}
