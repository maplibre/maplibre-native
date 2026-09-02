package org.maplibre.android.location.engine

/**
 * Data model that contains parameters for location
 * engine requests.
 *
 * @since 1.0.0
 */
class LocationEngineRequest private constructor(
    builder: Builder,
) {
    /**
     * Desired interval between location updates in milliseconds.
     *
     * @since 1.0.0
     */
    val interval: Long = builder.interval

    /**
     * Desired quality of the request, one of the priority constants.
     *
     * @since 1.0.0
     */
    val priority: Int = builder.priority

    /**
     * Distance between location updates in meters.
     *
     * @since 1.0.0
     */
    val displacement: Float = builder.displacement

    /**
     * Maximum wait time in milliseconds for location updates.
     *
     * @since 1.0.0
     */
    val maxWaitTime: Long = builder.maxWaitTime

    /**
     * Fastest interval in milliseconds for location updates.
     *
     * @since 1.0.0
     */
    val fastestInterval: Long = builder.fastestInterval

    /**
     * Compares this LocationEngineRequest to the specified object.
     *
     * @param other locationEngineRequest to compare to.
     * @return true when the type and values are equal, false otherwise.
     * @since 3.1.1
     */
    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        val that = other as LocationEngineRequest

        if (interval != that.interval) {
            return false
        }
        if (priority != that.priority) {
            return false
        }
        if (displacement.compareTo(that.displacement) != 0) {
            return false
        }
        if (maxWaitTime != that.maxWaitTime) {
            return false
        }
        return fastestInterval == that.fastestInterval
    }

    /**
     * Returns a hash code for this object.
     *
     * @return integer hash of the values.
     * @since 3.1.1
     */
    override fun hashCode(): Int {
        var result = (interval xor (interval ushr 32)).toInt()
        result = 31 * result + priority
        result = 31 * result + (if (displacement != +0.0f) displacement.toBits() else 0)
        result = 31 * result + (maxWaitTime xor (maxWaitTime ushr 32)).toInt()
        result = 31 * result + (fastestInterval xor (fastestInterval ushr 32)).toInt()
        return result
    }

    /**
     * Builds a [LocationEngineRequest].
     *
     * @param interval default interval between location updates
     * @since 1.0.0
     */
    class Builder(
        internal val interval: Long,
    ) {
        internal var priority: Int = PRIORITY_HIGH_ACCURACY
            private set
        internal var displacement: Float = 0.0f
            private set
        internal var maxWaitTime: Long = 0L
            private set
        internal var fastestInterval: Long = 0L
            private set

        /**
         * Set priority for request.
         * Use priority constant: [PRIORITY_HIGH_ACCURACY]
         *
         * @param priority constant
         * @return reference to builder
         * @since 1.0.0
         */
        fun setPriority(priority: Int): Builder {
            this.priority = priority
            return this
        }

        /**
         * Set distance between location updates.
         *
         * @param displacement distance between locations in meters.
         * @return reference to builder
         * @since 1.0.0
         */
        fun setDisplacement(displacement: Float): Builder {
            this.displacement = displacement
            return this
        }

        /**
         * Sets the maximum wait time in milliseconds for location updates.
         *
         * Locations determined at intervals but delivered in batch based on
         * wait time. Batching is not supported by all engines.
         *
         * @param maxWaitTime wait time in milliseconds.
         * @return reference to builder
         * @since 1.0.0
         */
        fun setMaxWaitTime(maxWaitTime: Long): Builder {
            this.maxWaitTime = maxWaitTime
            return this
        }

        /**
         * Sets the fastest interval in milliseconds for location updates.
         *
         * @param interval fastest interval in milliseconds.
         * @return reference to builder
         * @since 1.0.0
         */
        fun setFastestInterval(interval: Long): Builder {
            this.fastestInterval = interval
            return this
        }

        /**
         * Builds request object.
         *
         * @return instance of location request.
         * @since 1.0.0
         */
        fun build(): LocationEngineRequest = LocationEngineRequest(this)
    }

    companion object {
        /**
         * Used with [Builder.setPriority] to request the most accurate location.
         *
         * @since 1.0.0
         */
        const val PRIORITY_HIGH_ACCURACY = 0

        /**
         * Used with [Builder.setPriority] to request coarse location that is battery optimized.
         *
         * @since 1.0.0
         */
        const val PRIORITY_BALANCED_POWER_ACCURACY = 1

        /**
         * Used with [Builder.setPriority] to request coarse ~ 10 km accuracy location.
         *
         * @since 1.0.0
         */
        const val PRIORITY_LOW_POWER = 2

        /**
         * Used with [Builder.setPriority] to request passive location: no locations will be returned
         * unless a different client has requested location updates.
         *
         * @since 1.0.0
         */
        const val PRIORITY_NO_POWER = 3
    }
}
