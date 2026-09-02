package org.maplibre.android.style.layers

import androidx.annotation.Keep
import com.google.gson.JsonElement
import org.maplibre.android.LibraryLoader
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.types.Formatted
import org.maplibre.android.utils.ThreadUtils

/**
 * Base class for the different Layer types
 */
abstract class Layer {
    /**
     * Internal use
     *
     * @return the native peer pointer
     */
    @Keep
    var nativePtr: Long = 0
        private set

    @Keep
    @Suppress("unused")
    private var invalidated = false

    private var detached = false

    @Keep
    protected constructor(nativePtr: Long) {
        checkThread()
        this.nativePtr = nativePtr
    }

    constructor() {
        checkThread()
    }

    /**
     * Validates if layer interaction is happening on the UI thread
     */
    protected open fun checkThread() {
        ThreadUtils.checkThread(TAG)
    }

    fun setProperties(vararg properties: PropertyValue<*>) {
        if (detached) {
            return
        }

        checkThread()
        if (properties.isEmpty()) {
            return
        }

        for (property in properties) {
            val converted = convertValue(property.value)
            if (property is PaintPropertyValue<*>) {
                nativeSetPaintProperty(property.name, converted)
            } else {
                nativeSetLayoutProperty(property.name, converted)
            }
        }
    }

    val id: String
        get() {
            checkThread()
            return nativeGetId()
        }

    val visibility: PropertyValue<String>
        get() {
            checkThread()
            return PaintPropertyValue("visibility", nativeGetVisibility() as String)
        }

    var minZoom: Float
        get() {
            checkThread()
            return nativeGetMinZoom()
        }
        set(zoom) {
            checkThread()
            nativeSetMinZoom(zoom)
        }

    var maxZoom: Float
        get() {
            checkThread()
            return nativeGetMaxZoom()
        }
        set(zoom) {
            checkThread()
            nativeSetMaxZoom(zoom)
        }

    @Keep
    @Throws(Throwable::class)
    protected open external fun finalize()

    @Keep
    protected external fun nativeGetId(): String

    @Keep
    protected external fun nativeGetVisibility(): Any

    @Keep
    protected external fun nativeSetLayoutProperty(
        name: String?,
        value: Any?,
    )

    @Keep
    protected external fun nativeSetPaintProperty(
        name: String?,
        value: Any?,
    )

    @Keep
    protected external fun nativeSetFilter(filter: Array<Any?>?)

    @Keep
    protected external fun nativeGetFilter(): JsonElement?

    @Keep
    protected external fun nativeSetSourceLayer(sourceLayer: String?)

    @Keep
    protected external fun nativeGetSourceLayer(): String

    @Keep
    protected external fun nativeGetSourceId(): String

    @Keep
    protected external fun nativeGetMinZoom(): Float

    @Keep
    protected external fun nativeGetMaxZoom(): Float

    @Keep
    protected external fun nativeSetMinZoom(zoom: Float)

    @Keep
    protected external fun nativeSetMaxZoom(zoom: Float)

    private fun convertValue(value: Any?): Any? =
        when (value) {
            is Expression -> value.toArray()
            is Formatted -> value.toArray()
            else -> value
        }

    fun setDetached() {
        detached = true
    }

    fun isDetached(): Boolean = detached

    private companion object {
        const val TAG = "Mbgl-Layer"

        init {
            LibraryLoader.load()
        }
    }
}
