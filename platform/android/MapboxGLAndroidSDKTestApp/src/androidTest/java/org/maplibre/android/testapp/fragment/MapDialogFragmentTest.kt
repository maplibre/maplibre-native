package org.maplibre.android.testapp.fragment

import androidx.test.espresso.Espresso
import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.action.ViewActions.click
import androidx.test.espresso.matcher.ViewMatchers.withId
import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.AppCenter
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.activity.maplayout.MapInDialogActivity
import org.junit.Ignore
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Regression test that validates that a map inside a DialogFragment can be opened and closed.
 */
@RunWith(AndroidJUnit4ClassRunner::class)
@LargeTest
class MapDialogFragmentTest : AppCenter() {

    @get:Rule
    var activityRule: ActivityTestRule<MapInDialogActivity> = ActivityTestRule(MapInDialogActivity::class.java)

    @Test
    @Ignore("Probably does not work")
    fun openCloseDialog() {
        onView(withId(R.id.button_open_dialog)).perform(click())
        Thread.sleep(2500)
        Espresso.pressBack()
    }
}
