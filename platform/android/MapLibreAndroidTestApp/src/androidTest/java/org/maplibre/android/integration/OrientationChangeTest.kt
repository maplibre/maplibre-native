package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.testapp.activity.maplayout.SimpleMapActivity
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4ClassRunner::class)
class OrientationChangeTest : BaseIntegrationTest() {

    @get:Rule
    var activityRule: ActivityTestRule<SimpleMapActivity> = ActivityTestRule(SimpleMapActivity::class.java)

    @Test
    @LargeTest
    fun rotateSimpleMap() {
        device.setOrientationLeft()
        device.waitForIdle()
        device.setOrientationNatural()
        device.waitForIdle()
        device.setOrientationRight()
        device.waitForIdle()
        device.setOrientationNatural()
        device.setOrientationLeft()
        device.setOrientationNatural()
        device.setOrientationRight()
        device.setOrientationNatural()
    }
}
