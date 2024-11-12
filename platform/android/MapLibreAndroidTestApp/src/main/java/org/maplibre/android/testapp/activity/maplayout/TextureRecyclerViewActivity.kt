package org.maplibre.android.testapp.activity.maplayout

import android.annotation.SuppressLint
import org.maplibre.android.testapp.R

/**
 * TestActivity showcasing how to integrate multiple TexureView MapViews in a RecyclerView.
 */
@SuppressLint("ClickableViewAccessibility")
class TextureRecyclerViewActivity : GLSurfaceRecyclerViewActivity() {

    override fun getMapItemLayoutId(): Int {
        return R.layout.item_map_texture
    }
}
