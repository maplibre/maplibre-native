package org.maplibre.android.maps

/**
 * The subset of [MapLibreMapOptions] that is passed to the native map.
 *
 * The fields of this class are read from the native peer, they must not be renamed.
 */
open class NativeMapOptions {
    private val pixelRatio: Float
    private val crossSourceCollisions: Boolean

    private val actionJournalEnabled: Boolean
    private val actionJournalPath: String
    private val actionJournalLogFileSize: Long
    private val actionJournalLogFileCount: Long
    private val actionJournalRenderingReportInterval: Int
    private val asyncRendererCleanup: Boolean
    private val fastPFOREnabled: Boolean
    private val featureInfoEnabled: Boolean

    constructor(options: MapLibreMapOptions) {
        pixelRatio = options.pixelRatio
        crossSourceCollisions = options.crossSourceCollisions

        actionJournalEnabled = options.actionJournalEnabled
        actionJournalPath = options.actionJournalPath
        actionJournalLogFileSize = options.actionJournalLogFileSize
        actionJournalLogFileCount = options.actionJournalLogFileCount
        actionJournalRenderingReportInterval = options.actionJournalRenderingReportInterval
        asyncRendererCleanup = options.asyncRendererCleanup
        fastPFOREnabled = options.fastPFOREnabled
        featureInfoEnabled = options.featureInfoEnabled
    }

    constructor(pixelRatio: Float, crossSourceCollisions: Boolean) {
        this.pixelRatio = pixelRatio
        this.crossSourceCollisions = crossSourceCollisions

        actionJournalEnabled = false
        actionJournalPath = ""
        actionJournalLogFileSize = 0
        actionJournalLogFileCount = 0
        actionJournalRenderingReportInterval = 0

        asyncRendererCleanup = false
        fastPFOREnabled = false
        featureInfoEnabled = false
    }

    open fun pixelRatio(): Float = pixelRatio

    open fun crossSourceCollisions(): Boolean = crossSourceCollisions

    open fun actionJournalEnabled(): Boolean = actionJournalEnabled

    open fun actionJournalPath(): String = actionJournalPath

    open fun actionJournalLogFileSize(): Long = actionJournalLogFileSize

    open fun actionJournalLogFileCount(): Long = actionJournalLogFileCount

    open fun actionJournalRenderingReportInterval(): Int = actionJournalRenderingReportInterval

    open fun asyncRendererCleanup(): Boolean = asyncRendererCleanup

    open fun fastPFOREnabled(): Boolean = fastPFOREnabled

    open fun featureInfoEnabled(): Boolean = featureInfoEnabled
}
