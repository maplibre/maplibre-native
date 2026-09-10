package org.maplibre.android.location.engine

/**
 * Invoked for asynchronous notifications when new data
 * from engine becomes available.
 *
 * @param T Successful updated data type
 */
interface LocationEngineCallback<T> {
    /**
     * Invoked when new data available.
     *
     * @param result updated data.
     */
    fun onSuccess(result: T)

    /**
     * Invoked when engine exception occurs.
     *
     * @param exception [Exception]
     */
    fun onFailure(exception: Exception)
}
