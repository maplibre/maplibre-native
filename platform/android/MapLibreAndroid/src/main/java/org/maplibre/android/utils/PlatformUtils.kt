package org.maplibre.android.utils

object PlatformUtils {
    @JvmStatic
    fun isTest(): Boolean = isJUnit() || isRobolectric() || !isAndroid()

    @JvmStatic
    fun isJUnit(): Boolean =
        try {
            Class.forName("org.junit.Test")
            true
        } catch (ignored: ClassNotFoundException) {
            false
        }

    @JvmStatic
    fun isRobolectric(): Boolean =
        try {
            Class.forName("org.robolectric.Robolectric")
            true
        } catch (ignored: ClassNotFoundException) {
            false
        }

    @JvmStatic
    fun isAndroid(): Boolean =
        try {
            Class.forName("android.os.Build")
            true
        } catch (ignored: ClassNotFoundException) {
            false
        }
}
