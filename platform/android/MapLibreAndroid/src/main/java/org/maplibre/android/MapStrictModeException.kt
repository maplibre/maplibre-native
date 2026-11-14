package org.maplibre.android

class MapStrictModeException internal constructor(
    message: String?,
) : RuntimeException(
        String.format("Map detected an error that would fail silently otherwise: %s", message),
    )
