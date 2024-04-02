package org.maplibre.android.offline

import androidx.annotation.Keep
import androidx.annotation.StringDef

/**
 * An Offline Region error
 */
class OfflineRegionError // For JNI use only // Constructors
@Keep private constructor( // Getters
    @field:ErrorReason
    @get:ErrorReason
    val reason: String,
    /**
     * / * An error message from the request handler, e.g. a server message or a system message
     * / * informing the user about the reason for the failure.
     */
    val message: String
) {
    /**
     * Error code, as a string, self-explanatory.
     */
    @StringDef(*[REASON_SUCCESS, REASON_NOT_FOUND, REASON_SERVER, REASON_CONNECTION, REASON_OTHER])
    @kotlin.annotation.Retention(AnnotationRetention.SOURCE)
    annotation class ErrorReason

    override fun equals(o: Any?): Boolean {
        if (this === o) {
            return true
        }
        if (o == null || javaClass != o.javaClass) {
            return false
        }
        val that = o as OfflineRegionError
        return if (reason != that.reason) {
            false
        } else {
            message == that.message
        }
    }

    override fun hashCode(): Int {
        var result = reason.hashCode()
        result = 31 * result + message.hashCode()
        return result
    }

    override fun toString(): String {
        return ("OfflineRegionError{" + "reason='" + reason + '\'' + ", message='" + message + '\'' + '}')
    }

    companion object {
        const val REASON_SUCCESS = "REASON_SUCCESS"
        const val REASON_NOT_FOUND = "REASON_NOT_FOUND"
        const val REASON_SERVER = "REASON_SERVER"
        const val REASON_CONNECTION = "REASON_CONNECTION"
        const val REASON_OTHER = "REASON_OTHER"
    }
}
