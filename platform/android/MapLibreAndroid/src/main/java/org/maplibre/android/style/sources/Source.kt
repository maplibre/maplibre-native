package org.maplibre.android.style.sources

import androidx.annotation.Keep
import com.google.gson.JsonObject
import org.maplibre.android.LibraryLoader
import org.maplibre.android.utils.ThreadUtils

/**
 * Base Peer class for sources. see source.hpp for the other half of the peer.
 */
abstract class Source {
    /**
     * Internal use
     *
     * @return the native peer pointer
     */
    @Keep
    var nativePtr: Long = 0
        private set

    protected var detached = false

    /**
     * Internal use
     *
     * @param nativePtr - pointer to native peer
     */
    @Keep
    protected constructor(nativePtr: Long) {
        checkThread()
        this.nativePtr = nativePtr
    }

    constructor() {
        checkThread()
    }

    /**
     * Validates if source interaction is happening on the UI thread
     */
    protected open fun checkThread() {
        ThreadUtils.checkThread(TAG)
    }

    /**
     * Retrieve the source id
     *
     * @return the source id
     */
    val id: String
        get() {
            checkThread()
            return nativeGetId()
        }

    /**
     * Retrieve the source attribution.
     *
     * Will return an empty String if no attribution is available.
     *
     * @return the string representation of the attribution in html format
     */
    val attribution: String
        get() {
            checkThread()
            return nativeGetAttribution()
        }

    /**
     * The tile pre-fetching zoom delta for current source. Pre-fetching makes sure that a low-resolution
     * tile at the (current_zoom_level - delta) is rendered as soon as possible at the
     * expense of a little bandwidth.
     * If delta has not been set or set to null, it will use the value in MapLibreMap instance.
     */
    var prefetchZoomDelta: Int?
        get() = nativeGetPrefetchZoomDelta()
        set(delta) {
            nativeSetPrefetchZoomDelta(delta)
        }

    /**
     * When a set of tiles for a current zoom level is being rendered and some of the
     * ideal tiles that cover the screen are not yet loaded, parent tile could be
     * used instead. This might introduce unwanted rendering side-effects, especially
     * for raster tiles that are overscaled multiple times. This is the maximum
     * limit for how much a parent tile can be overscaled.
     */
    var maxOverscaleFactorForParentTiles: Int?
        get() = nativeGetMaxOverscaleFactorForParentTiles()
        set(maxOverscaleFactor) {
            nativeSetMaxOverscaleFactorForParentTiles(maxOverscaleFactor)
        }

    /**
     * Whether or not the fetched tiles for the given source should be stored in the local cache.
     *
     * True if tiles are volatile, false if they will be stored in local cache. Default value is false.
     */
    var isVolatile: Boolean?
        get() = nativeIsVolatile()
        set(value) {
            nativeSetVolatile(value)
        }

    /**
     * The minimum tile update interval in milliseconds, which is used to throttle the tile update
     * network requests. The default value is 0.
     */
    var minimumTileUpdateInterval: Long?
        get() = nativeGetMinimumTileUpdateInterval()
        set(interval) {
            nativeSetMinimumTileUpdateInterval(interval)
        }

    @Keep
    protected external fun nativeGetId(): String

    @Keep
    protected external fun nativeGetAttribution(): String

    @Keep
    protected external fun nativeGetPrefetchZoomDelta(): Int?

    @Keep
    protected external fun nativeSetPrefetchZoomDelta(delta: Int?)

    @Keep
    protected external fun nativeSetMaxOverscaleFactorForParentTiles(maxOverscaleFactor: Int?)

    @Keep
    protected external fun nativeGetMaxOverscaleFactorForParentTiles(): Int?

    @Keep
    protected external fun nativeIsVolatile(): Boolean?

    @Keep
    protected external fun nativeSetVolatile(value: Boolean?)

    @Keep
    protected external fun nativeSetMinimumTileUpdateInterval(interval: Long?)

    @Keep
    protected external fun nativeGetMinimumTileUpdateInterval(): Long?

    @Keep
    protected external fun nativeSetFeatureState(
        sourceLayerId: String?,
        featureId: String,
        state: JsonObject,
    ): Boolean

    @Keep
    protected external fun nativeGetFeatureState(
        sourceLayerId: String?,
        featureId: String,
    ): JsonObject?

    @Keep
    protected external fun nativeRemoveFeatureState(
        sourceLayerId: String?,
        featureId: String?,
        stateKey: String?,
    ): Boolean

    fun setDetached() {
        detached = true
    }

    private companion object {
        const val TAG = "Mbgl-Source"

        init {
            LibraryLoader.load()
        }
    }
}
