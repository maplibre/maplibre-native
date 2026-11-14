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
    open var nativePtr: Long = 0
        protected set

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

    open fun setProperties(vararg properties: PropertyValue<*>) {
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

    open val id: String
        get() {
            checkThread()
            return nativeGetId()
        }

    /**
     * When the visibility was set as an expression, this returns the current evaluated value.
     */
    open val visibility: PropertyValue<String>
        get() {
            checkThread()
            return PaintPropertyValue("visibility", nativeGetVisibility() as String)
        }

    /**
     * The visibility of this layer as an expression, or null when the visibility is a constant value.
     */
    open val visibilityExpression: Expression?
        get() {
            checkThread()
            val jsonElement = nativeGetVisibilityExpression()
            return if (jsonElement != null) {
                Expression.Converter.convert(jsonElement)
            } else {
                null
            }
        }

    open var minZoom: Float
        get() {
            checkThread()
            return nativeGetMinZoom()
        }
        set(zoom) {
            checkThread()
            nativeSetMinZoom(zoom)
        }

    open var maxZoom: Float
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
    protected open external fun nativeGetId(): String

    @Keep
    protected open external fun nativeGetVisibility(): Any

    @Keep
    private external fun nativeGetVisibilityExpression(): JsonElement?

    @Keep
    protected open external fun nativeSetLayoutProperty(
        name: String?,
        value: Any?,
    )

    @Keep
    protected open external fun nativeSetPaintProperty(
        name: String?,
        value: Any?,
    )

    @Keep
    protected open external fun nativeSetFilter(filter: Array<Any?>?)

    @Keep
    protected open external fun nativeGetFilter(): JsonElement?

    @Keep
    protected open external fun nativeSetSourceLayer(sourceLayer: String?)

    @Keep
    protected open external fun nativeGetSourceLayer(): String

    @Keep
    protected open external fun nativeGetSourceId(): String

    @Keep
    protected open external fun nativeGetMinZoom(): Float

    @Keep
    protected open external fun nativeGetMaxZoom(): Float

    @Keep
    protected open external fun nativeSetMinZoom(zoom: Float)

    @Keep
    protected open external fun nativeSetMaxZoom(zoom: Float)

    private fun convertValue(value: Any?): Any? =
        when (value) {
            is Expression -> value.toArray()
            is Formatted -> value.toArray()
            else -> value
        }

    open fun setDetached() {
        detached = true
    }

    open fun isDetached(): Boolean = detached

    private companion object {
        const val TAG = "Mbgl-Layer"

        init {
            LibraryLoader.load()
        }
    }
}
