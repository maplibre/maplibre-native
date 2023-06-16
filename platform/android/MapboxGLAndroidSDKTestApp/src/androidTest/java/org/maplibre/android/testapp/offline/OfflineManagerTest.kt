package org.maplibre.android.testapp.offline

import android.content.Context
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import java.io.IOException
import java.util.concurrent.CountDownLatch
import org.junit.After
import org.junit.Assert
import org.junit.Before
import org.junit.FixMethodOrder
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.MethodSorters
import org.maplibre.android.AppCenter
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.offline.OfflineRegion
import org.maplibre.android.storage.FileSource
import org.maplibre.android.testapp.activity.FeatureOverviewActivity
import org.maplibre.android.testapp.utils.FileUtils

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
@RunWith(AndroidJUnit4ClassRunner::class)
class OfflineManagerTest : AppCenter() {

    companion object {
        private const val TEST_DB_FILE_NAME = "offline_test.db"
    }

    @Rule
    @JvmField
    var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

    private val context: Context by lazy { rule.activity }

    @Before
    @After
    fun resetDatabase() {
        val latch = CountDownLatch(1)
        OfflineManager.getInstance(context).resetDatabase(object : OfflineManager.FileSourceCallback {
            override fun onSuccess() {
                latch.countDown()
            }

            override fun onError(message: String) {
                throw IOException("Unable to reset database before / after tests.")
            }
        })
        latch.await()
    }

    @Test(timeout = 30_000)
    fun combinedTest() {

        lateinit var mergedRegion: OfflineRegion

        // Copy file from assets

        val latch1 = CountDownLatch(1)
        rule.activity.runOnUiThread {
            FileUtils.CopyFileFromAssetsTask(
                rule.activity,
                object : FileUtils.OnFileCopiedFromAssetsListener {
                    override fun onFileCopiedFromAssets() {
                        latch1.countDown()
                    }

                    override fun onError() {
                        throw IOException("Unable to copy DB file.")
                    }
                }
            ).execute(TEST_DB_FILE_NAME, FileSource.getResourcesCachePath(rule.activity))
        }
        latch1.await()

        // Merge a second region

        val latch2 = CountDownLatch(1)
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).mergeOfflineRegions(
                FileSource.getResourcesCachePath(rule.activity) + "/" + TEST_DB_FILE_NAME,
                object : OfflineManager.MergeOfflineRegionsCallback {
                    override fun onMerge(offlineRegions: Array<OfflineRegion>?) {
                        Assert.assertEquals(1, offlineRegions?.size)
                        latch2.countDown()
                    }

                    override fun onError(error: String) {
                        throw RuntimeException("Unable to merge external offline database. $error")
                    }
                }
            )
        }
        latch2.await()

        // List regions (ensure merge yielded one region only)

        val latch3 = CountDownLatch(1)
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).listOfflineRegions(object : OfflineManager.ListOfflineRegionsCallback {
                override fun onList(offlineRegions: Array<OfflineRegion>?) {
                    Assert.assertEquals(1, offlineRegions?.size)
                    mergedRegion = offlineRegions!![0]
                    latch3.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to list regions in offline database. $error")
                }
            })
        }
        latch3.await()

        // Invalidate region

        val latch4 = CountDownLatch(1)
        rule.activity.runOnUiThread {
            mergedRegion.invalidate(object : OfflineRegion.OfflineRegionInvalidateCallback {
                override fun onInvalidate() {
                    latch4.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to invalidate region")
                }
            })
        }
        latch4.await()

        // Delete region

        val latch5 = CountDownLatch(1)
        rule.activity.runOnUiThread {
            mergedRegion.delete(object : OfflineRegion.OfflineRegionDeleteCallback {
                override fun onDelete() {
                    latch5.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to delete region")
                }
            })
        }
        latch5.await()

        // List regions (ensure deletion was successful)

        val latch6 = CountDownLatch(1)
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).listOfflineRegions(object : OfflineManager.ListOfflineRegionsCallback {
                override fun onList(offlineRegions: Array<OfflineRegion>?) {
                    Assert.assertEquals(0, offlineRegions?.size)
                    latch6.countDown()
                }

                override fun onError(error: String) {
                    throw RuntimeException("Unable to list regions in offline database. $error")
                }
            })
        }
        latch6.await()
    }
}
