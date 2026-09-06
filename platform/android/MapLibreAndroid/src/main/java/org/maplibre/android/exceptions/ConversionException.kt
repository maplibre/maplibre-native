package org.maplibre.android.exceptions

/**
 * A ConversionException is thrown when a conversion failed to execute.
 */
class ConversionException(
    detailMessage: String?,
) : RuntimeException(detailMessage)
