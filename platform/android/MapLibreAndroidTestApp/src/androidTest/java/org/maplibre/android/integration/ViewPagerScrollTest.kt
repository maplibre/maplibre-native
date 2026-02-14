package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import androidx.test.uiautomator.By
import androidx.test.uiautomator.Until
import org.maplibre.android.testapp.activity.fragment.ViewPagerActivity
import org.junit.Before
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

    @Before
    override fun beforeTest() {
        super.beforeTest()
        // Wait for the initial page tab to be visible before starting the test
        // This ensures the ViewPager and PagerTabStrip are fully laid out
        val initialTabVisible = device.wait(Until.hasObject(By.text("Page 0")), TIMEOUT_LONG)
        if (!initialTabVisible) {
            throw AssertionError("ViewPager tabs not visible after ${TIMEOUT_LONG}ms - Page 0 not found")
        }
    }

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
        val text = "Page $index"
        val found = device.wait(Until.hasObject(By.text(text)), TIMEOUT_UI_SEARCH_WAIT)
        if (!found) {
            throw AssertionError("Tab '$text' not found after ${TIMEOUT_UI_SEARCH_WAIT}ms")
        }
        val tab = device.findObject(By.text(text))
            ?: throw AssertionError("Tab '$text' was detected but findObject returned null")
        tab.click()
        // Small delay to allow ViewPager to settle after tab click
        device.waitForIdle(500)
    }

    companion object {
        private const val TIMEOUT_LONG = 15000L
    }
}
