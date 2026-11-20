package org.maplibre.android.exceptions

open class CalledFromWorkerThreadException(
    message: String?,
) : RuntimeException(message)
