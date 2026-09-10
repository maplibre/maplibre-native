package org.maplibre.android.maps

import android.graphics.Bitmap
import org.maplibre.android.MapLibre
import org.maplibre.android.annotations.Icon
import org.maplibre.android.annotations.IconFactory
import org.maplibre.android.annotations.Marker

/**
 * Responsible for managing icons added to the Map.
 *
 * Maintains a [List] of [Icon] and  is responsible for initialising default markers.
 *
 * Keep track of icons added and the resulting average icon size. This is used internally by our
 * gestures detection to calculate the size of a touch target.
 */
internal class IconManager(
    private val nativeMap: NativeMap?,
) {
    private val iconMap: MutableMap<Icon, Int> = HashMap()

    var highestIconWidth = 0
        private set

    var highestIconHeight = 0
        private set

    fun loadIconForMarker(marker: Marker): Icon {
        var icon = marker.icon
        if (icon == null) {
            // TODO replace with anchor implementation,
            // we are faking an anchor by adding extra pixels and diving height by 2
            icon = loadDefaultIconForMarker(marker)
        } else {
            updateHighestIconSize(icon)
        }
        addIcon(icon)
        return icon
    }

    fun getTopOffsetPixelsForIcon(icon: Icon): Int =
        (
            nativeMap!!.getTopOffsetPixelsForAnnotationSymbol(icon.id!!) *
                nativeMap.getPixelRatio()
        ).toInt()

    private fun loadDefaultIconForMarker(marker: Marker): Icon {
        val icon = IconFactory.getInstance(MapLibre.getApplicationContext()).defaultMarker()
        val bitmap = icon.bitmap!!
        updateHighestIconSize(bitmap.width, bitmap.height / 2)
        marker.icon = icon
        return icon
    }

    private fun addIcon(
        icon: Icon,
        addIconToMap: Boolean = true,
    ) {
        val refCounter = iconMap[icon]
        if (refCounter == null) {
            iconMap[icon] = 1
            if (addIconToMap) {
                loadIcon(icon)
            }
        } else {
            iconMap[icon] = refCounter + 1
        }
    }

    private fun updateHighestIconSize(icon: Icon) {
        updateHighestIconSize(icon.bitmap!!)
    }

    private fun updateHighestIconSize(bitmap: Bitmap) {
        updateHighestIconSize(bitmap.width, bitmap.height)
    }

    private fun updateHighestIconSize(
        width: Int,
        height: Int,
    ) {
        if (width > highestIconWidth) {
            highestIconWidth = width
        }

        if (height > highestIconHeight) {
            highestIconHeight = height
        }
    }

    private fun loadIcon(icon: Icon) {
        val bitmap = icon.bitmap!!
        nativeMap!!.addAnnotationIcon(
            icon.id!!,
            bitmap.width,
            bitmap.height,
            icon.scale,
            icon.toBytes(),
        )
    }

    fun reloadIcons() {
        for (icon in iconMap.keys) {
            loadIcon(icon)
        }
    }

    fun ensureIconLoaded(
        marker: Marker,
        maplibreMap: MapLibreMap,
    ) {
        val icon = marker.icon ?: loadDefaultIconForMarker(marker)
        addIcon(icon)
        setTopOffsetPixels(marker, maplibreMap, icon)
    }

    private fun setTopOffsetPixels(
        marker: Marker,
        maplibreMap: MapLibreMap,
        icon: Icon,
    ) {
        // this seems to be a costly operation according to the profiler so I'm trying to save some calls
        val previousMarker = if (marker.id != -1L) maplibreMap.getAnnotation(marker.id) as Marker? else null
        if (previousMarker?.icon == null || previousMarker.icon != marker.icon) {
            marker.setTopOffsetPixels(getTopOffsetPixelsForIcon(icon))
        }
    }

    fun iconCleanup(icon: Icon) {
        val refCounter = iconMap[icon]
        if (refCounter != null) {
            val updated = refCounter - 1
            if (updated == 0) {
                remove(icon)
            } else {
                updateIconRefCounter(icon, updated)
            }
        }
    }

    private fun remove(icon: Icon) {
        nativeMap!!.removeAnnotationIcon(icon.id!!)
        iconMap.remove(icon)
    }

    private fun updateIconRefCounter(
        icon: Icon,
        refCounter: Int,
    ) {
        iconMap[icon] = refCounter
    }
}
