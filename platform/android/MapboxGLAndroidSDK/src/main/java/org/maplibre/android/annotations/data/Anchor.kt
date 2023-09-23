package org.maplibre.android.annotations.data

enum class Anchor {
    LEFT,
    RIGHT,
    CENTER,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT;

    override fun toString(): String =
        super.toString().lowercase().replace('_', '-')
}