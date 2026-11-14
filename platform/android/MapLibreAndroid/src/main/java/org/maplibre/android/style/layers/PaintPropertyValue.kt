package org.maplibre.android.style.layers

open class PaintPropertyValue<T>(
    name: String,
    value: T?,
) : PropertyValue<T>(name, value)
