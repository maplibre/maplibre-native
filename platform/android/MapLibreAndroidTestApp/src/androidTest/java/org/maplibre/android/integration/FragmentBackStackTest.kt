package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import androidx.test.uiautomator.UiSelector
import org.junit.Ignore
import org.maplibre.android.testapp.activity.fragment.FragmentBackStackActivity
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Regression test that validates MapFragment integration on the backstack
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class FragmentBackStackTest : BaseIntegrationTest() {

    @get:Rule
    var activityRule: ActivityTestRule<FragmentBackStackActivity> = ActivityTestRule(FragmentBackStackActivity::class.java)

    @Test
    @Ignore("https://github.com/maplibre/maplibre-native/issues/2469")
    @LargeTest
    fun backPressedOnBackStackResumed() {
        device.waitForIdle()
        clickReplaceFragmentButton()
        device.pressHome()
        device.waitForIdle()
        device.launchActivity(activityRule.activity.applicationContext, FragmentBackStackActivity::class.java)
        backPressBackStack()
        device.waitForIdle()
    }

    private fun clickReplaceFragmentButton() {
        device.findObject(UiSelector().description(textDescription)).click()
    }

    private fun backPressBackStack() {
        device.pressBack() // pops fragment, showing map
        device.pressBack() // finish activity
    }

    private companion object {
        const val textDescription = "btn_change_fragment"
    }
}
