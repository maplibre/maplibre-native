package org.maplibre.android.offline

import androidx.annotation.Keep
import org.maplibre.android.offline.OfflineRegion.DownloadState

/**
 * A region's status includes its active/inactive state as well as counts
 * of the number of resources that have completed downloading, their total
 * size in bytes, and the total number of resources that are required.
 *
 *
 * Note that the total required size in bytes is not currently available. A
 * future API release may provide an estimate of this number.
 *
 */
class OfflineRegionStatus
/**
 * Returns when the value of requiredResourceCount is a precise
 * count of the number of required resources, and false when it is merely a lower
 * bound.
 *
 *
 * Specifically, it is false during early phases of an offline download. Once
 * style and tile sources have been downloaded, it is possible to calculate the
 * precise number of required resources, at which point it is set to true.
 *
 *
 * @return True if the required resource count is precise, false if not
 */

/*
* Use setObserver(OfflineRegionObserver observer) to obtain a OfflineRegionStatus object.
*
* For JNI use only
*/ @Keep private constructor(
    /**
     * Returns the download state.
     *
     *
     * State is defined as
     *
     *
     *  * [OfflineRegion.STATE_ACTIVE]
     *  * [OfflineRegion.STATE_INACTIVE]
     *
     *
     * @return the download state.
     */
    @field:DownloadState
    @get:DownloadState
    val downloadState: Int,
    /**
     * The number of resources (inclusive of tiles) that have been fully downloaded
     * and are ready for offline access.
     */
    val completedResourceCount: Long,
    /**
     * The cumulative size, in bytes, of all resources (inclusive of tiles) that have
     * been fully downloaded.
     */
    val completedResourceSize: Long,
    /**
     * The number of tiles that have been fully downloaded and are ready for
     * offline access.
     */
    val completedTileCount: Long,
    /**
     * The cumulative size, in bytes, of all tiles that have been fully downloaded.
     */
    val completedTileSize: Long,
    /**
     * The number of resources that are known to be required for this region. See the
     * documentation for `requiredResourceCountIsPrecise` for an important caveat
     * about this number.
     */
    val requiredResourceCount: Long,
    /**
     * This property is true when the value of requiredResourceCount is a precise
     * count of the number of required resources, and false when it is merely a lower
     * bound.
     *
     *
     * Specifically, it is false during early phases of an offline download. Once
     * style and tile sources have been downloaded, it is possible to calculate the
     * precise number of required resources, at which point it is set to true.
     *
     */
    val isRequiredResourceCountPrecise: Boolean
) {
    /**
     * Get the number of resources (inclusive of tiles) that have been fully downloaded
     * and are ready for offline access.
     *
     * @return the amount of resources that have finished downloading.
     */
    /**
     * The cumulative size, in bytes, of all resources (inclusive of tiles) that have
     * been fully downloaded.
     *
     * @return the size of the resources that have finished downloading
     */
    /**
     * Get the number of tiles that have been fully downloaded and are ready for
     * offline access.
     *
     * @return the completed tile count
     */
    /**
     * Get the cumulative size, in bytes, of all tiles that have been fully downloaded.
     *
     * @return the completed tile size
     */
    /**
     * Get the number of resources that are known to be required for this region. See the
     * documentation for `requiredResourceCountIsPrecise` for an important caveat
     * about this number.
     *
     * @return the amount of resources that are required
     */
    /**
     * Validates if the region download has completed
     *
     * @return true if download is complete, false if not
     */
    val isComplete: Boolean
        get() = completedResourceCount >= requiredResourceCount
}
