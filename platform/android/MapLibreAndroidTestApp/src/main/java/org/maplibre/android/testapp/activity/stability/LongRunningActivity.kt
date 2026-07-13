package org.maplibre.android.testapp.activity.stability

import android.annotation.SuppressLint
import android.app.ActivityManager
import android.app.ActivityOptions
import android.content.Context
import android.content.Intent
import android.hardware.display.DisplayManager
import android.os.Build
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
        private const val DURATION = 72 * 60 * 60
        // start one activity/view per display if available
        private const val USE_SECONDARY_DISPLAY = true

        @SuppressLint("NewApi")
        fun printStats(context: Context) {
            val activityManager = context.getSystemService(ACTIVITY_SERVICE) as ActivityManager
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

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_long_running_maps)

        if (USE_SECONDARY_DISPLAY && Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val displayManager = getSystemService(DISPLAY_SERVICE) as DisplayManager
            val displays = displayManager.displays

            if (displays.size > 1) {
                // remove navigation map from layout
                supportFragmentManager
                    .beginTransaction()
                    .remove(supportFragmentManager.findFragmentById(R.id.navigation_map)!!)
                    .commit()

                // and move it to it's own activity
                val activityOptions = ActivityOptions.makeBasic()
                activityOptions.launchDisplayId = displays[1].displayId

                startActivity(Intent(this, NavigationMapActivity::class.java), activityOptions.toBundle())
            }
        }

        LOG.info("Activity running for $DURATION seconds")

        val context = this

        lifecycleScope.launch {
            delay(DURATION * 1000L)

            printStats(context)
            finish()
        }
    }
}
