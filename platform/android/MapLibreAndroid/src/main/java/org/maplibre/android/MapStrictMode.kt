package org.maplibre.android

/**
 * Using this class you can enable a strict mode that will throw the [MapStrictModeException]
 * whenever the map would fail silently otherwise.
 */
object MapStrictMode {
    @Volatile
    private var strictModeEnabled = false

    /**
     * Set the strict mode that will throw the [MapStrictModeException]
     * whenever the map would fail silently otherwise.
     *
     * @param strictModeEnabled true to enable the strict mode, false otherwise
     */
    @JvmStatic
    @Synchronized
    fun setStrictModeEnabled(strictModeEnabled: Boolean) {
        MapStrictMode.strictModeEnabled = strictModeEnabled
    }

    /**
     * Internal use. Called whenever the strict mode violation occurs.
     */
    @JvmStatic
    fun strictModeViolation(message: String?) {
        if (strictModeEnabled) {
            throw MapStrictModeException(message)
        }
    }

    /**
     * Internal use. Called whenever the strict mode violation occurs.
     */
    @JvmStatic
    fun strictModeViolation(
        message: String?,
        throwable: Throwable?,
    ) {
        if (strictModeEnabled) {
            throw MapStrictModeException(String.format("%s - %s", message, throwable))
        }
    }

    /**
     * Internal use. Called whenever the strict mode violation occurs.
     */
    @JvmStatic
    fun strictModeViolation(throwable: Throwable?) {
        if (strictModeEnabled) {
            throw MapStrictModeException(String.format("%s", throwable))
        }
    }
}
