package org.maplibre.android.testapp.storage

import androidx.test.annotation.UiThreadTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.AppCenter
import org.maplibre.android.storage.FileSource
import org.maplibre.android.testapp.activity.espresso.EspressoTestActivity
import org.junit.After
import org.junit.Assert
import org.junit.Before
import org.junit.Rule
import org.junit.Test
import org.junit.rules.TestName
import org.junit.runner.RunWith
import java.util.concurrent.CountDownLatch

@RunWith(AndroidJUnit4ClassRunner::class)
open class FileSourceMapTest : AppCenter() {

    private lateinit var fileSourceTestUtils: FileSourceTestUtils

    @get:Rule
    val rule = ActivityTestRule(EspressoTestActivity::class.java)

    @get:Rule
    val testName = TestName()

    @Before
    @UiThreadTest
    fun setup() {
        fileSourceTestUtils = FileSourceTestUtils(rule.activity)
        fileSourceTestUtils.setup()
    }

    @Test
    fun changeResourcesPathWhileMapVisible() {
        val latch = CountDownLatch(1)
        rule.activity.runOnUiThread {
            FileSource.setResourcesCachePath(
                fileSourceTestUtils.testPath,
                object : FileSource.ResourcesCachePathChangeCallback {
                    override fun onSuccess(path: String) {
                        latch.countDown()
                        Assert.assertEquals(fileSourceTestUtils.testPath, path)
                    }

                    override fun onError(message: String) {
                        Assert.fail("Resources path can be changed while the map is running")
                    }
                }
            )
        }
        latch.await()
    }

    @After
    fun cleanup() {
        fileSourceTestUtils.cleanup()
    }
}
