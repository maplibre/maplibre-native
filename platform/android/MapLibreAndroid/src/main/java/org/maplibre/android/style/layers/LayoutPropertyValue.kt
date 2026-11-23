package org.maplibre.android.style.layers

open class LayoutPropertyValue<T>(
    name: String,
    value: T?,
) : PropertyValue<T>(name, value)
