package org.maplibre.android.location

import org.maplibre.android.maps.Style
import org.maplibre.android.style.layers.Layer

internal class LocationComponentPositionManager(
    private val style: Style,
    private var layerAbove: String?,
    private var layerBelow: String?,
    var bearingOnTop: Boolean,
) {
    /**
     * Returns true whenever layer above/below configuration has changed and requires re-layout.
     */
    fun update(
        layerAbove: String?,
        layerBelow: String?,
        bearingOnTop: Boolean,
    ): Boolean {
        val requiresUpdate =
            this.layerAbove != layerAbove ||
                this.layerBelow != layerBelow ||
                this.bearingOnTop != bearingOnTop

        this.layerAbove = layerAbove
        this.layerBelow = layerBelow
        this.bearingOnTop = bearingOnTop
        return requiresUpdate
    }

    fun addLayerToMap(layer: Layer) {
        val layerAbove = this.layerAbove
        val layerBelow = this.layerBelow
        if (layerAbove != null) {
            style.addLayerAbove(layer, layerAbove)
        } else if (layerBelow != null) {
            style.addLayerBelow(layer, layerBelow)
        } else {
            style.addLayer(layer)
        }
    }
}
