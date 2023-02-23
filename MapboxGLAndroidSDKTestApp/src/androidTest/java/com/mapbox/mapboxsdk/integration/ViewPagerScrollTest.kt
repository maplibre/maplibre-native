package com.mapbox.mapboxsdk.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import androidx.test.uiautomator.UiSelector
import com.mapbox.mapboxsdk.testapp.activity.fragment.ViewPagerActivity
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Regression test that validates MapFragment integration with a ViewPager
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class ViewPagerScrollTest : BaseIntegrationTest() {

    @get:Rule
    var activityRule: ActivityTestRule<ViewPagerActivity> = ActivityTestRule(ViewPagerActivity::class.java)

    @Test
    @LargeTest
    fun scrollViewPager() {
        for (i in 1..4) {
            clickTab(i)
        }

        for (i in 3 downTo 0) {
            clickTab(i)
        }
    }

    private fun clickTab(index: Int) {
        device.findObject(UiSelector().text("Page $index")).click()
    }
}
