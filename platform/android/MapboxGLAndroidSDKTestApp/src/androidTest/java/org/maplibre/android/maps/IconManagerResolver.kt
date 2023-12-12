package org.maplibre.android.maps

import org.maplibre.android.annotations.Icon
import timber.log.Timber

class IconManagerResolver(maplibreMap: MapLibreMap?) {
    private var iconManager: IconManager? = null

    init {
        try {
            val annotationManagerField = MapLibreMap::class.java.getDeclaredField("annotationManager")
            annotationManagerField.isAccessible = true
            val annotationManager = annotationManagerField[maplibreMap] as AnnotationManager
            val iconManagerField = AnnotationManager::class.java.getDeclaredField("iconManager")
            iconManagerField.isAccessible = true
            iconManager = iconManagerField[annotationManager] as IconManager
        } catch (exception: Exception) {
            Timber.e(exception, "Could not create IconManagerResolver, unable to reflect.")
        }
    }

    val iconMap: Map<Icon, Int>
        get() {
            try {
                val field = IconManager::class.java.getDeclaredField("iconMap")
                field.isAccessible = true
                return field[iconManager] as Map<Icon, Int>
            } catch (exception: NoSuchFieldException) {
                Timber.e(exception, "Could not getIconMap, unable to reflect.")
            } catch (exception: IllegalAccessException) {
                Timber.e(exception, "Could not getIconMap, unable to reflect.")
            }
            return HashMap()
        }
}
