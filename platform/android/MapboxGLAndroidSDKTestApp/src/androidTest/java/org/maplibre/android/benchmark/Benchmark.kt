package org.maplibre.android.benchmark

import android.app.Activity
import androidx.test.core.app.launchActivityForResult
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.testapp.activity.benchmark.BenchmarkActivity

@RunWith(AndroidJUnit4ClassRunner::class)
class Benchmark {
    @Test
    fun worldTourBenchmark() {
        val scenario = launchActivityForResult<BenchmarkActivity>()
        assertEquals(scenario.result.resultCode, Activity.RESULT_OK)
    }
}