package org.maplibre.plugins.android

import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.geometry.LatLng
import org.maplibre.plugins.ngonlayer.NgonLayerPlugin

/** A plugin demo: its style at a camera. To show another plugin, add an entry to [all]. */
class Demo(
    val id: String,
    val displayName: String,
    val style: String,
    val camera: CameraPosition,
    /** Registers the plugin's layer types. Returns false when the active renderer does not implement the plugin. */
    val register: () -> Boolean,
) {
    companion object {
        val all = listOf(
            Demo(
                id = "ngon-layer",
                displayName = "N-gons",
                // plugins/plugins/ngon-layer/examples/ngon.json, see app/build.gradle.kts.
                style = "asset://ngon.json",
                camera = CameraPosition.Builder().target(LatLng(0.0, 0.0)).zoom(11.3).build(),
                register = NgonLayerPlugin::register,
            ),
        )

        fun find(id: String): Demo? = all.firstOrNull { it.id == id }
    }
}
