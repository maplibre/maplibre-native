package org.maplibre.android.testapp.offline

import android.content.Context
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.offline.OfflineManager
import org.maplibre.android.testapp.activity.FeatureOverviewActivity
import org.junit.Assert
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith
import java.util.concurrent.CountDownLatch

@RunWith(AndroidJUnit4ClassRunner::class)
class CacheTest {

    @Rule
    @JvmField
    var rule = ActivityTestRule(FeatureOverviewActivity::class.java)

    private val context: Context by lazy { rule.activity }

    private val countDownLatch = CountDownLatch(1)

    @Test
    fun testSetMaximumAmbientCacheSize() {
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).setMaximumAmbientCacheSize(
                10000000,
                object : OfflineManager.FileSourceCallback {
                    override fun onSuccess() {
                        countDownLatch.countDown()
                    }

                    override fun onError(message: String) {
                        Assert.assertNull("onError should not be called", message)
                    }
                }
            )
        }
        countDownLatch.await()
    }

    @Test
    fun testSetClearAmbientCache() {
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).clearAmbientCache(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    countDownLatch.countDown()
                }

                override fun onError(message: String) {
                    Assert.assertNull("onError should not be called", message)
                }
            })
        }
        countDownLatch.await()
    }

    @Test
    fun testSetInvalidateAmbientCache() {
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).invalidateAmbientCache(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    countDownLatch.countDown()
                }

                override fun onError(message: String) {
                    Assert.assertNull("onError should not be called", message)
                }
            })
        }
        countDownLatch.await()
    }

    @Test
    fun testSetResetDatabase() {
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).resetDatabase(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    countDownLatch.countDown()
                }

                override fun onError(message: String) {
                    Assert.assertNull("onError should not be called", message)
                }
            })
        }
        countDownLatch.await()
    }

    @Test
    fun testSetPackDatabase() {
        rule.activity.runOnUiThread {
            OfflineManager.getInstance(context).packDatabase(object : OfflineManager.FileSourceCallback {
                override fun onSuccess() {
                    countDownLatch.countDown()
                }

                override fun onError(message: String) {
                    Assert.assertNull("onError should not be called", message)
                }
            })
        }
        countDownLatch.await()
    }
}
