package com.mapbox.mapboxsdk.style.sources

import androidx.annotation.Keep
import com.mapbox.mapboxsdk.LibraryLoader
import com.mapbox.mapboxsdk.utils.ThreadUtils
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
    protected fun checkThread() {
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
     *
     * Will return an empty String if no attribution is available.
     *
     *
     * @return the string representation of the attribution in html format
     */
    val attribution: String
        get() {
            checkThread()
            return nativeGetAttribution()
        }
    /**
     * Retrieve current pre-fetching zoom delta.
     *
     * @return current zoom delta or null if not set.
     */
    /**
     * Set the tile pre-fetching zoom delta for current source. Pre-fetching makes sure that a low-resolution
     * tile at the (current_zoom_level - delta) is rendered as soon as possible at the
     * expense of a little bandwidth.
     * If delta has not been set or set to null, it will use the value in MapboxMap instance.
     *
     * @param delta zoom delta
     */
    var prefetchZoomDelta: Int?
        get() = nativeGetPrefetchZoomDelta()
        set(delta) = nativeSetPrefetchZoomDelta(delta)
    /**
     * Retrieve current maximum overscale factor for parent tiles.
     *
     * @return current maximum overscale factor or null if not set.
     */
    /**
     * When a set of tiles for a current zoom level is being rendered and some of the
     * ideal tiles that cover the screen are not yet loaded, parent tile could be
     * used instead. This might introduce unwanted rendering side-effects, especially
     * for raster tiles that are overscaled multiple times. This method sets the maximum
     * limit for how much a parent tile can be overscaled.
     *
     * @param maxOverscaleFactor maximum overscale factor
     */
    var maxOverscaleFactorForParentTiles: Int?
        get() = nativeGetMaxOverscaleFactorForParentTiles()
        set(maxOverscaleFactor) {
            nativeSetMaxOverscaleFactorForParentTiles(maxOverscaleFactor)
        }
    /**
     * Retrieve whether or not the fetched tiles for the given source should be stored in the local cache
     *
     * @return true if tiles are volatile, false if they will be stored in local cache. Default value is false.
     */
    /**
     * Set a flag defining whether or not the fetched tiles for the given source should be stored in the local cache
     *
     * @param value current setting for volatile.
     */
    var isVolatile: Boolean?
        get() = nativeIsVolatile()
        set(value) {
            nativeSetVolatile(value)
        }
    /**
     * Retrieve the minimum tile update interval, which is used to throttle the tile update network requests.
     *
     * @return the update interval in milliseconds, default valuse is 0.
     */
    /**
     * Sets the minimum tile update interval, which is used to throttle the tile update network requests.
     *
     * @param interval the update interval in milliseconds.
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
    protected external fun nativeGetPrefetchZoomDelta(): Int

    @Keep
    protected external fun nativeSetPrefetchZoomDelta(delta: Int?)

    @Keep
    protected external fun nativeSetMaxOverscaleFactorForParentTiles(maxOverscaleFactor: Int?)

    @Keep
    protected external fun nativeGetMaxOverscaleFactorForParentTiles(): Int

    @Keep
    protected external fun nativeIsVolatile(): Boolean

    @Keep
    protected external fun nativeSetVolatile(value: Boolean?)

    @Keep
    protected external fun nativeSetMinimumTileUpdateInterval(interval: Long?)

    @Keep
    protected external fun nativeGetMinimumTileUpdateInterval(): Long

    fun setDetached() {
        detached = true
    }

    companion object {
        private const val TAG = "Mbgl-Source"

        init {
            LibraryLoader.load()
        }
    }
}
