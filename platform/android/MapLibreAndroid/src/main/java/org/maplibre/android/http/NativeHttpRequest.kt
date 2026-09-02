package org.maplibre.android.http

import androidx.annotation.Keep
import org.maplibre.android.MapLibre
import java.util.concurrent.locks.ReentrantLock

@Keep
class NativeHttpRequest
    @Keep
    private constructor(
        @field:Keep private var nativePtr: Long,
        resourceUrl: String,
        dataRange: String,
        etag: String,
        modified: String,
        offlineUsage: Boolean,
    ) : HttpResponder {
        private val httpRequest: HttpRequest = MapLibre.getModuleProvider().createHttpRequest()

        // Reentrancy is not needed, but "Lock" is an abstract class.
        private val lock = ReentrantLock()

        init {
            if (resourceUrl.startsWith("local://")) {
                // used by render test to serve files from assets
                executeLocalRequest(resourceUrl)
            } else {
                httpRequest.executeRequest(this, nativePtr, resourceUrl, dataRange, etag, modified, offlineUsage)
            }
        }

        fun cancel() {
            httpRequest.cancelRequest()

            // TODO: We need a lock here because we can try
            // to cancel at the same time the request is getting
            // answered on the OkHTTP thread. We could get rid of
            // this lock by using Runnable when we move Android
            // implementation of mln::RunLoop to Looper.
            lock.lock()
            nativePtr = 0
            lock.unlock()
        }

        override fun onResponse(
            responseCode: Int,
            eTag: String?,
            lastModified: String?,
            cacheControl: String?,
            expires: String?,
            retryAfter: String?,
            xRateLimitReset: String?,
            body: ByteArray,
        ) {
            lock.lock()
            if (nativePtr != 0L) {
                nativeOnResponse(
                    responseCode,
                    eTag,
                    lastModified,
                    cacheControl,
                    expires,
                    retryAfter,
                    xRateLimitReset,
                    body,
                )
            }
            lock.unlock()
        }

        private fun executeLocalRequest(resourceUrl: String) {
            LocalRequestTask(
                MapLibre.getApplicationContext().assets,
                object : LocalRequestTask.OnLocalRequestResponse {
                    override fun onResponse(bytes: ByteArray?) {
                        if (bytes != null) {
                            lock.lock()
                            if (nativePtr != 0L) {
                                nativeOnResponse(200, null, null, null, null, null, null, bytes)
                            }
                            lock.unlock()
                        }
                    }
                },
            ).execute(resourceUrl)
        }

        override fun handleFailure(
            type: Int,
            errorMessage: String,
        ) {
            lock.lock()
            if (nativePtr != 0L) {
                nativeOnFailure(type, errorMessage)
            }
            lock.unlock()
        }

        @Keep
        private external fun nativeOnFailure(
            type: Int,
            message: String,
        )

        @Keep
        private external fun nativeOnResponse(
            code: Int,
            etag: String?,
            modified: String?,
            cacheControl: String?,
            expires: String?,
            retryAfter: String?,
            xRateLimitReset: String?,
            body: ByteArray,
        )
    }
