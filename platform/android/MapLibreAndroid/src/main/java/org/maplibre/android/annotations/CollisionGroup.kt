package org.maplibre.android.annotations

import okhttp3.internal.toImmutableList
import org.maplibre.android.annotations.data.Anchor
import org.maplibre.android.annotations.data.Defaults
import kotlin.properties.Delegates

class CollisionGroup(
    symbols: List<Symbol> = emptyList(),
    val zLayer: Int = Defaults.Z_LAYER,
    symbolSpacing: Float = Defaults.COLLISION_GROUP_SYMBOL_SPACING,
    symbolAvoidEdges: Boolean = Defaults.COLLISION_GROUP_SYMBOL_AVOID_EDGES,
    iconAllowOverlap: Boolean = Defaults.COLLISION_GROUP_ICON_ALLOW_OVERLAP,
    iconIgnorePlacement: Boolean = Defaults.COLLISION_GROUP_ICON_IGNORE_PLACEMENT,
    iconOptional: Boolean = Defaults.COLLISION_GROUP_ICON_OPTIONAL,
    iconPadding: Float = Defaults.COLLISION_GROUP_ICON_PADDING,
    textPadding: Float = Defaults.COLLISION_GROUP_TEXT_PADDING,
    textAllowOverlap: Boolean = Defaults.COLLISION_GROUP_TEXT_ALLOW_OVERLAP,
    textIgnorePlacement: Boolean = Defaults.COLLISION_GROUP_TEXT_IGNORE_PLACEMENT,
    textOptional: Boolean = Defaults.COLLISION_GROUP_TEXT_OPTIONAL,
    textVariableAnchor: Array<Anchor>? = Defaults.COLLISION_GROUP_TEXT_VARIABLE_ANCHOR
) {

    var symbols: List<Symbol> by Delegates.observable(
        symbols.toImmutableList(),
        onChange = { _, _, new ->
            testDistinctSymbolKeys()
            manager?.apply {
                deleteAll()
                if (new.isNotEmpty()) {
                    addAll(new)
                    (new[0].key() as SymbolKey).applyProperties(this)
                }
            }
        }
    )

    var symbolSpacing: Float = symbolSpacing
        set(value) {
            field = value
            manager?.symbolSpacing = value
        }
    var symbolAvoidEdges: Boolean = symbolAvoidEdges
        set(value) {
            field = value
            manager?.symbolAvoidEdges = value
        }
    var iconAllowOverlap: Boolean = iconAllowOverlap
        set(value) {
            field = value
            manager?.iconAllowOverlap = value
        }
    var iconIgnorePlacement: Boolean = iconIgnorePlacement
        set(value) {
            field = value
            manager?.iconIgnorePlacement = value
        }
    var iconOptional: Boolean = iconOptional
        set(value) {
            field = value
            manager?.iconOptional = value
        }
    var iconPadding: Float = iconPadding
        set(value) {
            field = value
            manager?.iconPadding = value
        }
    var textPadding: Float = textPadding
        set(value) {
            field = value
            manager?.textPadding = value
        }
    var textAllowOverlap: Boolean = textAllowOverlap
        set(value) {
            field = value
            manager?.textAllowOverlap = value
        }
    var textIgnorePlacement: Boolean = textIgnorePlacement
        set(value) {
            field = value
            manager?.textIgnorePlacement = value
        }
    var textOptional: Boolean = textOptional
        set(value) {
            field = value
            manager?.textOptional = value
        }
    var textVariableAnchor: Array<Anchor>? = textVariableAnchor
        set(value) {
            field = value
            manager?.textVariableAnchor = value?.map { it.toString() }?.toTypedArray()
        }

    // Set by AnnotationContainerKeys
    internal var manager: SymbolManager? = null

    init {
        testDistinctSymbolKeys()

        if (textVariableAnchor?.isEmpty() == true) {
            throw IllegalArgumentException(
                "An empty array has been provided as a text variable anchor. Please use `null` " +
                        "instead of an empty array to indicate that no alternative anchors are provided."
            )
        }
    }

    private fun testDistinctSymbolKeys() {
        symbols.map {
            it.key()
        }.distinct().let {
            if (it.size > 1) {
                throw IllegalArgumentException(
                    "You have added symbols with conflicting Non-Data Driven (NDD) properties " +
                            "to this cluster group. Namely, the following sets of properties " +
                            "were found:\n${it.joinToString("; \n")}"
                )
            }
        }
    }
}