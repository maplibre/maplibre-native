package org.maplibre.android.maps

/**
 * The subset of [MapLibreMapOptions] that is passed to the native map.
 *
 * The fields of this class are read from the native peer, they must not be renamed.
 */
class NativeMapOptions {
    private val pixelRatio: Float
    private val crossSourceCollisions: Boolean

    private val actionJournalEnabled: Boolean
    private val actionJournalPath: String
    private val actionJournalLogFileSize: Long
    private val actionJournalLogFileCount: Long
    private val actionJournalRenderingReportInterval: Int
    private val asyncRendererCleanup: Boolean

    private val fastPFOREnabled: Boolean

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
    }

    fun pixelRatio(): Float = pixelRatio

    fun crossSourceCollisions(): Boolean = crossSourceCollisions

    fun actionJournalEnabled(): Boolean = actionJournalEnabled

    fun actionJournalPath(): String = actionJournalPath

    fun actionJournalLogFileSize(): Long = actionJournalLogFileSize

    fun actionJournalLogFileCount(): Long = actionJournalLogFileCount

    fun actionJournalRenderingReportInterval(): Int = actionJournalRenderingReportInterval

    fun asyncRendererCleanup(): Boolean = asyncRendererCleanup

    fun fastPFOREnabled(): Boolean = fastPFOREnabled
}
