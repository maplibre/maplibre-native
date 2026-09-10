package org.maplibre.android.style.layers

import androidx.annotation.Keep

/**
 * Thrown when adding a layer to a map twice
 */
@Keep
class CannotAddLayerException(
    message: String?,
) : RuntimeException(message)
