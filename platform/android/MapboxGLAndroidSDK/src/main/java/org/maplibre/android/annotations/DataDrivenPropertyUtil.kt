package org.maplibre.android.annotations

import android.graphics.Bitmap
import com.google.gson.JsonArray
import com.google.gson.JsonObject

typealias PairWithDefault = Triple<String, Any?, Any?>

internal infix fun Pair<String, Any?>.default(that: Any?) = PairWithDefault(
    this.first,
    this.second,
    that
)

internal fun List<PairWithDefault>.filterNonDefault(): List<Pair<String, Any>> =
    mapNotNull { triple ->
        val second = triple.second
        if (second != null && second != triple.third) {
            Pair(triple.first, second)
        } else {
            null
        }
    }

internal fun List<PairWithDefault>.nonDefaultMap(): Map<String, Any> = filterNonDefault().toMap()

internal fun Map<String, Any>.asJsonObject(): JsonObject = this.entries.fold(JsonObject()) { accumulator, pair ->
    accumulator.also { jsonElement ->
        when (val value = pair.value) {
            is String -> jsonElement.addProperty(pair.key, value)
            is Number -> jsonElement.addProperty(pair.key, value)
            is Boolean -> jsonElement.addProperty(pair.key, value)
            is Char -> jsonElement.addProperty(pair.key, value)
            is Bitmap -> jsonElement.addProperty(pair.key, value.toString())
            is Array<*> -> jsonElement.add(
                pair.key,
                JsonArray().apply {
                    value.forEach {
                        when (it) {
                            is String -> add(it)
                            is Number -> add(it)
                            is Boolean -> add(it)
                            is Char -> add(it)
                        }
                    }
                }
            )
            is List<*> -> jsonElement.add(
                pair.key,
                JsonArray().apply {
                    value.forEach {
                        when (it) {
                            is String -> add(it)
                            is Number -> add(it)
                            is Boolean -> add(it)
                            is Char -> add(it)
                        }
                    }
                }
            )
            else -> throw IllegalArgumentException(
                "There is a bug in one of the `flattenedValue` getters, " +
                    "providing a property that cannot be converted to JSON."
            )
        }
    }
}

internal fun Int.asColorString(): String =
    // see Android's implementation of Color.valueOf(Int)
    "rgba(${this shr 16 and 0xff}, ${this shr 8 and 0xff}, ${this and 0xff}, ${this shr 24 and 0xff})"
