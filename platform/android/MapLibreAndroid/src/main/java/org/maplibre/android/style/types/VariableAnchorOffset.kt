package org.maplibre.android.style.types

import org.json.JSONArray
import org.json.JSONObject

/**
 * Represents a collection of anchor offsets.
 */
class VariableAnchorOffset(values: Array<AnchorOffset>) {
    private val anchorOffsets: Array<AnchorOffset>

    init {
        values.copyOf().also { anchorOffsets = it }
    }

    /**
     * Converts the collection to a JSON string representation.
     *
     * @return The JSON string representation of the collection.
     */
    override fun toString(): String {
        val jsonArray = JSONArray()
        for (entry in anchorOffsets) {
            val entryArray = JSONArray()
            entryArray.put(entry.offset[0])
            entryArray.put(entry.offset[1])

            val entryObject = JSONObject()
            entryObject.put(entry.anchorType, entryArray)

            jsonArray.put(entryObject)
        }

        return jsonArray.toString()
    }

    /**
     * Converts the collection to an array.
     *
     * @return An array containing the anchor types and their offsets.
     */
    fun toArray(): Array<Any?> {
        val anchorOffsetArray = arrayOfNulls<Any>(anchorOffsets.size * 2)
        for (i in anchorOffsets.indices) {
            anchorOffsetArray[i] = anchorOffsets[i].anchorType;
            anchorOffsetArray[i + 1] = anchorOffsets[i].offset;
        }

        return anchorOffsetArray
    }

    fun isEmpty(): Boolean {
        return anchorOffsets.isEmpty()
    }

    fun size(): Int {
        return anchorOffsets.size
    }

    fun begin(): Iterator<AnchorOffset> {
        return anchorOffsets.iterator()
    }

    fun end(): Iterator<AnchorOffset> {
        return anchorOffsets.iterator()
    }

    operator fun get(index: Int): AnchorOffset {
        return anchorOffsets[index]
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is VariableAnchorOffset) return false
        return anchorOffsets.contentEquals(other.anchorOffsets)
    }

    override fun hashCode(): Int {
        return anchorOffsets.contentHashCode()
    }
}