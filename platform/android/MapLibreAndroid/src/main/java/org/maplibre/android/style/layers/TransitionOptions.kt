package org.maplibre.android.style.layers

import androidx.annotation.Keep

/**
 * Resembles transition property from the style specification.
 *
 * @see [Transition documentation](https://maplibre.org/maplibre-style-spec/transition/)
 *
 * @param duration the duration of the transition
 * @param delay the delay to start the transition
 * @param enablePlacementTransitions the flag that describes whether the fade in/out symbol placement transition
 * should be enabled. Defaults to true.
 */
class TransitionOptions(
    /**
     * The transition duration.
     */
    @field:Keep val duration: Long,
    /**
     * The transition delay.
     */
    @field:Keep val delay: Long,
    @field:Keep private val enablePlacementTransitions: Boolean,
) {
    /**
     * Create a transition property based on duration and a delay.
     *
     * @param duration the duration of the transition
     * @param delay    the delay to start the transition
     */
    constructor(duration: Long, delay: Long) : this(duration, delay, true)

    /**
     * The flag that describes whether the fade in/out symbol placement transition should be enabled.
     *
     * True if the fade in/out symbol placement transition should be enabled, false otherwise.
     */
    val isEnablePlacementTransitions: Boolean
        get() = enablePlacementTransitions

    override fun equals(other: Any?): Boolean {
        if (this === other) {
            return true
        }
        if (other == null || javaClass != other.javaClass) {
            return false
        }

        val that = other as TransitionOptions

        if (duration != that.duration) {
            return false
        }
        if (delay != that.delay) {
            return false
        }
        return enablePlacementTransitions == that.enablePlacementTransitions
    }

    override fun hashCode(): Int {
        var result = (duration xor (duration ushr 32)).toInt()
        result = 31 * result + (delay xor (delay ushr 32)).toInt()
        result = 31 * result + if (enablePlacementTransitions) 1 else 0
        return result
    }

    override fun toString(): String =
        (
            "TransitionOptions{" +
                "duration=" + duration +
                ", delay=" + delay +
                ", enablePlacementTransitions=" + enablePlacementTransitions +
                '}'
        )

    companion object {
        /**
         * Create a transition property based on duration and a delay.
         *
         * @param duration the duration of the transition
         * @param delay    the delay to start the transition
         * @return a new transition property object
         */
        @Keep
        @JvmStatic
        @Deprecated("use fromTransitionOptions(long, long, boolean) instead")
        fun fromTransitionOptions(
            duration: Long,
            delay: Long,
        ): TransitionOptions {
            // Invoked from JNI only
            return TransitionOptions(duration, delay)
        }

        /**
         * Create a transition property.
         *
         * @param duration                   the duration of the transition
         * @param delay                      the delay to start the transition
         * @param enablePlacementTransitions the flag that describes whether the fade in/out symbol placement
         *                                   transition should be enabled. Defaults to true.
         * @return a new transition property object
         */
        @Keep
        @JvmStatic
        @JvmName("fromTransitionOptions")
        internal fun fromTransitionOptions(
            duration: Long,
            delay: Long,
            enablePlacementTransitions: Boolean,
        ): TransitionOptions {
            // Invoked from JNI only
            return TransitionOptions(duration, delay, enablePlacementTransitions)
        }
    }
}
