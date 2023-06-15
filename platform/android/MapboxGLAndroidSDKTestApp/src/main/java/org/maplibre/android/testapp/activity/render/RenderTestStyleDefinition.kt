package org.maplibre.android.testapp.activity.render

import java.util.HashMap

class RenderTestStyleDefinition {
    var version: Int? = null
    var metadata: Metadata? = null
    private val additionalProperties: MutableMap<String, Any> = HashMap()
    fun getAdditionalProperties(): Map<String, Any> {
        return additionalProperties
    }

    fun setAdditionalProperty(name: String, value: Any) {
        additionalProperties[name] = value
    }

    inner class Metadata {
        var test: Test? = null
        private val additionalProperties: MutableMap<String, Any> = HashMap()
        fun getAdditionalProperties(): Map<String, Any> {
            return this.additionalProperties
        }

        fun setAdditionalProperty(name: String, value: Any) {
            this.additionalProperties[name] = value
        }
    }

    inner class Test {
        var width: Int? = null
        var height: Int? = null
        val pixelRatio: Float? = null
        var center: List<Int>? = null
        var zoom: Int? = null
        var diff: Double? = null
        var operations: List<List<String>>? = null
        private val additionalProperties: MutableMap<String, Any> = HashMap()
        fun getAdditionalProperties(): Map<String, Any> {
            return this.additionalProperties
        }

        fun setAdditionalProperty(name: String, value: Any) {
            this.additionalProperties[name] = value
        }
    }
}
