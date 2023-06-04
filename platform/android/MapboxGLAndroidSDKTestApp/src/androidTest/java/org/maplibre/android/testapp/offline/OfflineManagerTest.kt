package org.maplibre.android.testapp.offline

import android.content.Context
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.AppCenter
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.offline.OfflineRegion
import org.maplibre.android.storage.FileSource
import org.maplibre.android.testapp.activity.FeatureOverviewActivity
import org.maplibre.android.testapp.utils.FileUtils
import org.junit.FixMethodOrder
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.MethodSorters
import java.io.IOException
import java.util.concurrent.CountDownLatch

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
@RunWith(AndroidJUnit4ClassRunner::class)
class OfflineManagerTest : AppCenter() {

    companion object {
        private const val TEST_DB_FILE_NAME = "offline_test.db"
        private lateinit var mergedRegion: OfflineRegion
    }

    @Rule
    @JvmField
    var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

    private val context: Context by lazy { rule.activity }

    @Test(timeout = 30_000)
    fun a_copyFileFromAssets() {
        val latch = CountDownLatch(1)
        rule.activity.runOnUiThread {
            FileUtils.CopyFileFromAssetsTask(
                rule.activity,
                object : FileUtils.OnFileCopiedFromAssetsListener {
                    override fun onFileCopiedFromAssets() {
                        latch.countDown()
                    }

                    override fun onError() {
                        throw IOException("Unable to copy DB file.")
                    }
                }
            ).execute(TEST_DB_FILE_NAME, FileSource.getResourcesCachePath(rule.activity))
        }
        latch.await()
    }

    @Test(timeout = 30_000)
    fun b_mergeRegion() {
        val latch = CountDownLatch(1)
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).mergeOfflineRegions(
                FileSource.getResourcesCachePath(rule.activity) + "/" + TEST_DB_FILE_NAME,
                object : OfflineManager.MergeOfflineRegionsCallback {
                    override fun onMerge(offlineRegions: Array<OfflineRegion>?) {
                        assert(offlineRegions?.size == 1)
                        latch.countDown()
                    }

                    override fun onError(error: String) {
                        throw RuntimeException("Unable to merge external offline database. $error")
                    }
                }
            )
        }
        latch.await()
    }

    @Test(timeout = 30_000)
    fun c_listRegion() {
        val latch = CountDownLatch(1)
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).listOfflineRegions(object : OfflineManager.ListOfflineRegionsCallback {
                override fun onList(offlineRegions: Array<OfflineRegion>?) {
                    assert(offlineRegions?.size == 1)
                    mergedRegion = offlineRegions!![0]
                    latch.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to merge external offline database. $error")
                }
            })
        }
        latch.await()
    }

    @Test(timeout = 30_000)
    fun d_invalidateRegion() {
        val latch = CountDownLatch(1)
        rule.activity.runOnUiThread {
            mergedRegion.invalidate(object : OfflineRegion.OfflineRegionInvalidateCallback {
                override fun onInvalidate() {
                    latch.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to delete region")
                }
            })
        }
        latch.await()
    }

    @Test(timeout = 30_000)
    fun e_deleteRegion() {
        val latch = CountDownLatch(1)
        rule.activity.runOnUiThread {
            mergedRegion.delete(object : OfflineRegion.OfflineRegionDeleteCallback {
                override fun onDelete() {
                    latch.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to delete region")
                }
            })
        }
        latch.await()
    }
}
