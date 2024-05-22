package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import androidx.test.uiautomator.UiSelector
import org.junit.Ignore
import org.maplibre.android.testapp.activity.fragment.ViewPagerActivity
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Regression test that validates MapFragment integration with a ViewPager
 */
@Ignore("https://github.com/maplibre/maplibre-native/issues/2316")
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
