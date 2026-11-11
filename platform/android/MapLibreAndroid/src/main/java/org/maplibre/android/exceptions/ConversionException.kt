package org.maplibre.android.exceptions

/**
 * A ConversionException is thrown when a conversion failed to execute.
 */
open class ConversionException(
    detailMessage: String?,
) : RuntimeException(detailMessage)
