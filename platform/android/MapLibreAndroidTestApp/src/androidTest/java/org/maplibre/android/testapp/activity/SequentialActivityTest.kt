package org.maplibre.android.testapp.activity

import android.app.Activity
import android.content.pm.PackageManager
import androidx.test.core.app.ActivityScenario
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.Parameterized

@RunWith(Parameterized::class)
class SequentialActivityTest(private val activity: Class<out Activity>) {

    companion object {
        // app currently has 100+ activities
        private const val USE_ALL_ACTIVITIES = true
        private const val ACTIVITY_DURATION = 5000L

        // ignores for both activity lists
        private val ignoredActivities = listOf(
            FeatureOverviewActivity::class.java,
            org.maplibre.android.testapp.activity.benchmark.BenchmarkActivity::class.java,
            org.maplibre.android.testapp.activity.telemetry.PerformanceMeasurementActivity::class.java,
            org.maplibre.android.testapp.activity.stability.LongRunningActivity::class.java,
            org.maplibre.android.testapp.activity.stability.UserMapActivity::class.java,
            org.maplibre.android.testapp.activity.stability.NavigationMapActivity::class.java,

            // need style updates
            org.maplibre.android.testapp.activity.turf.MapSnapshotterWithinExpression::class.java,
            org.maplibre.android.testapp.activity.turf.WithinExpressionActivity::class.java,
            org.maplibre.android.testapp.activity.style.HeatmapLayerActivity::class.java,
        )

        // activity list used when USE_ALL_ACTIVITIES is disabled
        private val activityList = listOf(
            org.maplibre.android.testapp.activity.annotation.BulkMarkerActivity::class.java,
            org.maplibre.android.testapp.activity.camera.CameraAnimatorActivity::class.java,
            org.maplibre.android.testapp.activity.customlayer.CustomLayerActivity::class.java,
            org.maplibre.android.testapp.activity.events.ObserverActivity::class.java,
            org.maplibre.android.testapp.activity.feature.QuerySourceFeaturesActivity::class.java,
            org.maplibre.android.testapp.activity.fragment.MapFragmentActivity::class.java,
            org.maplibre.android.testapp.activity.imagegenerator.SnapshotActivity::class.java,
            org.maplibre.android.testapp.activity.infowindow.InfoWindowActivity::class.java,
            org.maplibre.android.testapp.activity.location.LocationComponentActivationActivity::class.java,
            org.maplibre.android.testapp.activity.maplayout.DebugModeActivity::class.java,
            org.maplibre.android.testapp.activity.offline.OfflineActivity::class.java,
            org.maplibre.android.testapp.activity.options.MapOptionsXmlActivity::class.java,
            org.maplibre.android.testapp.activity.render.RenderTestActivity::class.java,
            org.maplibre.android.testapp.activity.snapshot.MapSnapshotterActivity::class.java,
            org.maplibre.android.testapp.activity.sources.VectorTileActivity::class.java,
            org.maplibre.android.testapp.activity.storage.UrlTransformActivity::class.java,
            org.maplibre.android.testapp.activity.style.AnimatedImageSourceActivity::class.java,
            org.maplibre.android.testapp.activity.turf.WithinExpressionActivity::class.java,
        )

        private fun getAllActivities() : List<Any> {
            val context = InstrumentationRegistry.getInstrumentation().targetContext

            val packageInfo = context.packageManager.getPackageInfo(
                context.packageName,
                PackageManager.GET_ACTIVITIES
            )

            val activities = packageInfo.activities
                .filter { info ->
                    info.name.startsWith("org.maplibre.android.testapp.activity")
                }
                .mapNotNull { info ->
                    try {
                        Class.forName(info.name)
                    } catch (_: ClassNotFoundException) { }
                }

            return activities
        }

        @JvmStatic
        @Parameterized.Parameters(name = "{index}: Activity {0}")
        fun activities() : List<Any> {
            return if (USE_ALL_ACTIVITIES) {
                getAllActivities().filter { it !in ignoredActivities }
            } else {
                activityList.filter { it !in ignoredActivities }
            }
        }
    }

    @Test
    fun launchActivity() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()

        val scenario = ActivityScenario.launch(activity)
        instrumentation.waitForIdleSync()

        Thread.sleep(ACTIVITY_DURATION)

        scenario.close()
    }
}
