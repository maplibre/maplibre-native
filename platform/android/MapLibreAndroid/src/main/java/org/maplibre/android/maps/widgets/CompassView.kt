package org.maplibre.android.maps.widgets

import android.content.Context
import android.graphics.drawable.Drawable
import android.util.AttributeSet
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import androidx.core.view.ViewCompat
import androidx.core.view.ViewPropertyAnimatorCompat
import androidx.core.view.ViewPropertyAnimatorListenerAdapter
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.MapLibreMapOptions
import kotlin.math.abs

/**
 * UI element overlaid on a map to show the map's bearing when it isn't true north (0.0). Tapping
 * the compass resets the bearing to true north and hides the compass.
 *
 * You can change the behaviour of this View during initialisation with
 * [MapLibreMapOptions], and xml attributes. While running you can
 * use [org.maplibre.android.maps.UiSettings].
 */
class CompassView :
    ImageView,
    Runnable {
    private var compassRotation = 0.0f
    private var fadeCompassViewFacingNorth = true
    private var fadeAnimator: ViewPropertyAnimatorCompat? = null
    private lateinit var compassAnimationListener: MapLibreMap.OnCompassAnimationListener
    private var animating = false

    constructor(context: Context) : super(context) {
        initialize(context)
    }

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs) {
        initialize(context)
    }

    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr) {
        initialize(context)
    }

    private fun initialize(context: Context) {
        isEnabled = false

        // Layout params
        val screenDensity = context.resources.displayMetrics.density
        layoutParams = ViewGroup.LayoutParams((48 * screenDensity).toInt(), (48 * screenDensity).toInt())
    }

    fun injectCompassAnimationListener(compassAnimationListener: MapLibreMap.OnCompassAnimationListener) {
        this.compassAnimationListener = compassAnimationListener
    }

    fun isAnimating(isAnimating: Boolean) {
        this.animating = isAnimating
    }

    fun resetAnimation() {
        fadeAnimator?.cancel()
        fadeAnimator = null
    }

    val isHidden: Boolean
        get() = fadeCompassViewFacingNorth && isFacingNorth()

    fun isFacingNorth(): Boolean =
        // increase range of facing north to more than only 0.0
        abs(compassRotation) >= 359.0 || abs(compassRotation) <= 1.0

    override fun setEnabled(enabled: Boolean) {
        super.setEnabled(enabled)
        if (enabled && !isHidden) {
            resetAnimation()
            alpha = 1.0f
            visibility = View.VISIBLE
            update(compassRotation.toDouble())
        } else {
            resetAnimation()
            alpha = 0.0f
            visibility = View.INVISIBLE
        }
    }

    /**
     * Updates the direction of the compass.
     *
     * @param bearing the direction value of the map
     */
    fun update(bearing: Double) {
        compassRotation = bearing.toFloat()

        if (!isEnabled) {
            return
        }

        if (isHidden) {
            if (visibility == View.INVISIBLE || fadeAnimator != null) {
                return
            }
            postDelayed(this, TIME_WAIT_IDLE)
            return
        } else {
            resetAnimation()
            alpha = 1.0f
            visibility = View.VISIBLE
        }

        notifyCompassAnimationListenerWhenAnimating()
        rotation = compassRotation
    }

    fun fadeCompassViewFacingNorth(compassFadeFacingNorth: Boolean) {
        fadeCompassViewFacingNorth = compassFadeFacingNorth
    }

    val isFadeCompassViewFacingNorth: Boolean
        get() = fadeCompassViewFacingNorth

    /**
     * Set the CompassView image.
     *
     * @param compass the drawable to use as compass image
     */
    fun setCompassImage(compass: Drawable?) {
        setImageDrawable(compass)
    }

    /**
     * Get the current configured CompassView image.
     *
     * @return the drawable used as compass image
     */
    fun getCompassImage(): Drawable? = drawable

    override fun run() {
        if (isHidden) {
            compassAnimationListener.onCompassAnimationFinished()
            resetAnimation()
            setLayerType(View.LAYER_TYPE_HARDWARE, null)
            fadeAnimator =
                ViewCompat.animate(this).alpha(0.0f).setDuration(TIME_FADE_ANIMATION).apply {
                    setListener(
                        object : ViewPropertyAnimatorListenerAdapter() {
                            override fun onAnimationEnd(view: View) {
                                setLayerType(LAYER_TYPE_NONE, null)
                                visibility = View.INVISIBLE
                                resetAnimation()
                            }
                        },
                    )
                }
        }
    }

    private fun notifyCompassAnimationListenerWhenAnimating() {
        if (animating) {
            compassAnimationListener.onCompassAnimation()
        }
    }

    companion object {
        const val TIME_WAIT_IDLE = 500L
        const val TIME_MAP_NORTH_ANIMATION = 150L
        private const val TIME_FADE_ANIMATION = TIME_WAIT_IDLE
    }
}
