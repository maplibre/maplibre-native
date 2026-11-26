package org.maplibre.android.testapp.utils

import androidx.test.espresso.IdlingResource
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.RenderingStats

typealias FeatureCheck = (layerId: String, sourceId: String, featureInfo: RenderingStats.FeatureInfo) -> Boolean

/// An IdlingResource that waits for a specific feature/layer/source to be rendered
class FeatureMatchIdleResource(private val mapView: MapView) :
    IdlingResource {
    private val listener: MapView.OnDidFinishRenderingFrameWithStatsListener =
        MapView.OnDidFinishRenderingFrameWithStatsListener { fully, stats ->
            if (fully && !matched) {
                val anyMatched = anyOf.isEmpty() || anyOf.any { checkFeature(it, stats) }
                if (anyMatched && allOf.all { checkFeature(it, stats) }) {
                    callback?.onTransitionToIdle()
                    matched = true
                }
            }
        }

    init {
        mapView.addOnDidFinishRenderingFrameListener(listener)
    }

    /// Add the feature/layer/source to check for, and the minimum count of features to wait for.
    /// If one or more are added, at least one of them must appear in a frame, in addition to those
    /// in the allOf set, for a frame to be considered "idle".
    fun addAnyMatch(
        featureId: String?,
        layerId: String?,
        sourceId: String?,
        count: Int = 1,
        check: FeatureCheck? = null
    ) {
        anyOf = anyOf + FeatureMatch(featureId, layerId, sourceId, count, check)
        matched = false
    }

    /// Add the feature/layer/source to check for, and the minimum count of features to wait for.
    /// All features added with this method must appear in a frame for it to be considered "idle".
    fun addAllMatch(
        featureId: String?,
        layerId: String?,
        sourceId: String?,
        count: Int = 1,
        check: FeatureCheck? = null
    ) {
        allOf = allOf + FeatureMatch(featureId, layerId, sourceId, count, check)
        matched = false
    }

    /// Reset the conditions, the resource will be considered idle until new conditions are added.
    fun clear() {
        anyOf = emptyList()
        allOf = emptyList()
        matched = true
    }

    /// Remove the view listener
    fun unregister() {
        mapView.removeOnDidFinishRenderingFrameListener(listener)
    }

    override fun getName(): String {
        return javaClass.simpleName
    }

    override fun isIdleNow(): Boolean {
        return matched
    }

    override fun registerIdleTransitionCallback(callback: IdlingResource.ResourceCallback?) {
        this.callback = callback
    }

    private fun checkFeature(match: FeatureMatch, stats: RenderingStats): Boolean {
        return match.minCount < 1 || stats.renderedFeatures.any {
            (match.sourceId == null || it.key.sourceID == match.sourceId) && (match.layerId == null || it.key.layerID == match.layerId) && checkFeature(
                match,
                it.key.layerID,
                it.key.sourceID,
                it.value
            )
        }
    }

    private fun checkFeature(
        match: FeatureMatch,
        layerId: String,
        sourceId: String,
        featureInfo: List<RenderingStats.FeatureInfo>
    ): Boolean {
        return (match.featureId == null && match.check == null) || featureInfo.any {
            (match.featureId == null || it.featureID == match.featureId) && (match.check == null || match.check.invoke(
                layerId,
                sourceId,
                it
            ))
        }
    }

    private data class FeatureMatch(
        val featureId: String?,
        val layerId: String?,
        val sourceId: String?,
        val minCount: Int,
        val check: FeatureCheck?
    )

    private var anyOf: List<FeatureMatch> = emptyList()
    private var allOf: List<FeatureMatch> = emptyList()
    private var matched = true
    private var callback: IdlingResource.ResourceCallback? = null
}
