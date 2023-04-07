package com.mapbox.mapboxsdk.testapp.utils

import android.app.Activity
import android.content.Intent
import com.mapbox.mapboxsdk.testapp.activity.FeatureOverviewActivity

object NavUtils {
    fun navigateHome(context: Activity) {
        context.startActivity(Intent(context, FeatureOverviewActivity::class.java))
        context.finish()
    }
}
