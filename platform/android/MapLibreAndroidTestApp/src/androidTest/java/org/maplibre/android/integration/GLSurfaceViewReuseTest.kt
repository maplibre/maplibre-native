package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.testapp.activity.maplayout.GLSurfaceRecyclerViewActivity
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Regression test that validates if a GLSurfaceView surface can be recreated without crashing.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class GLSurfaceViewReuseTest : BaseIntegrationTest() {

    @get:Rule
    var activityRule: ActivityTestRule<GLSurfaceRecyclerViewActivity> = ActivityTestRule(GLSurfaceRecyclerViewActivity::class.java)

    @Test
    @LargeTest
    fun scrollRecyclerView() {
        device.waitForIdle()
        device.scrollRecyclerViewTo("Twenty-one")
        device.waitForIdle()
        device.scrollRecyclerViewTo("One")
        device.waitForIdle()
    }
}
