package org.maplibre.android.testapp.activity.stability

import android.annotation.SuppressLint
import android.app.ActivityManager
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import org.maplibre.android.testapp.R
import java.util.logging.Logger

class LongRunningActivity : AppCompatActivity() {

    // config
    companion object {
        private val LOG = Logger.getLogger(LongRunningActivity::class.java.name)

        // activity lifetime (seconds)
        private const val DURATION = 5//10 * 60 * 60
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_long_running_maps)

        LOG.info("Activity running for $DURATION seconds")

        lifecycleScope.launch {
            delay(DURATION * 1000L)

            printStats()
            finish()
        }
    }

    @SuppressLint("NewApi")
    private fun printStats() {
        val activityManager = this.getSystemService(ACTIVITY_SERVICE) as ActivityManager
        val sysMemInfo = ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(sysMemInfo)

        LOG.info("System: \n" +
            "\tavailable memory - ${sysMemInfo.availMem / 1048576} MB\n" +
            "\ttotal memory - ${sysMemInfo.totalMem / 1048576} MB\n" +
            "\tlow memory threshold - ${sysMemInfo.threshold / 1048576} MB\n" +
            "\tlow memory - ${sysMemInfo.lowMemory}\n"
        )

        val appMemInfo = activityManager.getProcessMemoryInfo(intArrayOf(android.os.Process.myPid())).first()

        LOG.info("Application memory: \n" +
            appMemInfo.memoryStats.map { "\t${it.key} - ${it.value} KB" }.joinToString("\n")
        )
    }
}
