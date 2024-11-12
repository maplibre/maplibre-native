package org.maplibre.android.testapp.activity.render

import android.content.Context
import org.maplibre.android.snapshotter.MapSnapshot
import org.maplibre.android.snapshotter.MapSnapshotter

class RenderTestSnapshotter internal constructor(context: Context, options: Options) :
    MapSnapshotter(context, options) {
    override fun addOverlay(mapSnapshot: MapSnapshot) {
        // don't add an overlay
    }
}
