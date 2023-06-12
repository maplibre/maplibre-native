package org.maplibre.android.integration

import androidx.test.filters.LargeTest
import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import androidx.test.rule.ActivityTestRule
import org.maplibre.android.testapp.activity.feature.QueryRenderedFeaturesBoxCountActivity
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Regression test that validates reopening an Activity and querying the map before surface recreation #14394
 */
@RunWith(AndroidJUnit4ClassRunner::class)
class QueryRenderedFeaturesBoxCountTest : BaseIntegrationTest() {

    @get:Rule
    var activityRule: ActivityTestRule<QueryRenderedFeaturesBoxCountActivity> =
        ActivityTestRule(QueryRenderedFeaturesBoxCountActivity::class.java)

    @Test
    @LargeTest
    fun reopenQueryRendererFeaturesActivity() {
        device.waitForIdle()
        device.pressHome()
        device.waitForIdle()
        device.launchActivity(activityRule.activity, QueryRenderedFeaturesBoxCountActivity::class.java)
        device.waitForIdle()
    }
}
