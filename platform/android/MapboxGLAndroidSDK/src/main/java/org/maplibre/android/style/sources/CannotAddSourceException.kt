package org.maplibre.android.style.sources

import androidx.annotation.Keep

/**
 * Thrown when adding a source to a map twice
 */
@Keep
class CannotAddSourceException(message: String?) : RuntimeException(message)
