package com.mapbox.mapboxsdk.testapp.activity.render

import android.content.Context
import com.mapbox.mapboxsdk.snapshotter.MapSnapshot
import com.mapbox.mapboxsdk.snapshotter.MapSnapshotter

class RenderTestSnapshotter internal constructor(context: Context, options: Options) :
    MapSnapshotter(context, options) {
    override fun addOverlay(mapSnapshot: MapSnapshot) {
        // don't add an overlay
    }
}
