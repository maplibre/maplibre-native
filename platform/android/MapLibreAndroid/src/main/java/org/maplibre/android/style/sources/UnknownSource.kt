package org.maplibre.android.style.sources

import androidx.annotation.Keep
import androidx.annotation.UiThread
/**
 * An unknown type of source
 */
@UiThread
@Keep
class UnknownSource
/**
 * Creates a UnknownSource.
 *
 * @param nativePtr pointer used by core
 */
internal constructor(nativePtr: Long) : Source(nativePtr) {
    protected external fun initialize()

    @Throws(Throwable::class)
    protected external fun finalize()
}
