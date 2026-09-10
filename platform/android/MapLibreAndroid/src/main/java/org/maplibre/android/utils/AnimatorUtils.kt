package org.maplibre.android.utils

import android.animation.Animator
import android.animation.AnimatorInflater
import android.animation.AnimatorListenerAdapter
import android.animation.ObjectAnimator
import android.view.View
import androidx.annotation.AnimatorRes
import androidx.interpolator.view.animation.FastOutSlowInInterpolator

/**
 * Animator utility class.
 */
object AnimatorUtils {
    /**
     * Animate a view from an animator resource.
     *
     * @param view        the view to be animated
     * @param animatorRes the animator resource to be loaded
     * @param listener    the animator end listener
     */
    @JvmStatic
    fun animate(
        view: View,
        @AnimatorRes animatorRes: Int,
        listener: OnAnimationEndListener?,
    ) {
        animate(view, animatorRes, -1, listener)
    }

    /**
     * Animate a view from an animator resource.
     *
     * @param view        the view to be animated
     * @param animatorRes the animator resource to be loaded
     * @param duration    the duration of the animator
     * @param listener    the animator end listener
     */
    @JvmStatic
    fun animate(
        view: View?,
        @AnimatorRes animatorRes: Int,
        duration: Int,
        listener: OnAnimationEndListener?,
    ) {
        if (view == null) {
            return
        }

        view.setLayerType(View.LAYER_TYPE_HARDWARE, null)
        val animator = AnimatorInflater.loadAnimator(view.context, animatorRes)
        if (duration != -1) {
            animator.duration = duration.toLong()
        }

        animator.addListener(
            object : AnimatorListenerAdapter() {
                override fun onAnimationEnd(animation: Animator) {
                    super.onAnimationEnd(animation)
                    view.setLayerType(View.LAYER_TYPE_NONE, null)
                    listener?.onAnimationEnd()
                }
            },
        )
        animator.setTarget(view)
        animator.start()
    }

    /**
     * Animate a view from an animator resource.
     *
     * @param view        the view to be animated
     * @param animatorRes the animator resource to be loaded
     */
    @JvmStatic
    fun animate(
        view: View,
        @AnimatorRes animatorRes: Int,
    ) {
        animate(view, animatorRes, -1)
    }

    /**
     * Animate a view from an animator resource.
     *
     * @param view        the view to be animated
     * @param animatorRes the animator resource to be loaded
     * @param duration    the duration of the animator
     */
    @JvmStatic
    fun animate(
        view: View,
        @AnimatorRes animatorRes: Int,
        duration: Int,
    ) {
        animate(view, animatorRes, duration, null)
    }

    /**
     * Animate a view rotation property to a value.
     *
     * @param view     the view to be rotated
     * @param rotation the value to animate to
     */
    @JvmStatic
    fun rotate(
        view: View,
        rotation: Float,
    ) {
        view.setLayerType(View.LAYER_TYPE_HARDWARE, null)
        val rotateAnimator = ObjectAnimator.ofFloat(view, View.ROTATION, view.rotation, rotation)
        rotateAnimator.addListener(
            object : AnimatorListenerAdapter() {
                override fun onAnimationEnd(animation: Animator) {
                    super.onAnimationEnd(animation)
                    view.setLayerType(View.LAYER_TYPE_NONE, null)
                }
            },
        )
        rotateAnimator.start()
    }

    /**
     * Animate a view rotation property by a value.
     *
     * @param view       the view to be rotated
     * @param rotationBy the value to animate by
     */
    @JvmStatic
    fun rotateBy(
        view: View,
        rotationBy: Float,
    ) {
        view.setLayerType(View.LAYER_TYPE_HARDWARE, null)
        view.animate().rotationBy(rotationBy).setInterpolator(FastOutSlowInInterpolator()).setListener(
            object : AnimatorListenerAdapter() {
                override fun onAnimationEnd(animation: Animator) {
                    super.onAnimationEnd(animation)
                    view.setLayerType(View.LAYER_TYPE_NONE, null)
                }
            },
        )
    }

    /**
     * Animate a view alpha property to a value.
     *
     * @param convertView the view to be animated
     * @param alpha       the value to animate to
     * @param listener    the animator end listener
     */
    @JvmStatic
    fun alpha(
        convertView: View,
        alpha: Float,
        listener: OnAnimationEndListener?,
    ) {
        convertView.setLayerType(View.LAYER_TYPE_HARDWARE, null)
        val rotateAnimator = ObjectAnimator.ofFloat(convertView, View.ALPHA, convertView.alpha, alpha)
        rotateAnimator.addListener(
            object : AnimatorListenerAdapter() {
                override fun onAnimationStart(animation: Animator) {
                    super.onAnimationStart(animation)
                    convertView.visibility = View.VISIBLE
                }

                override fun onAnimationEnd(animation: Animator) {
                    super.onAnimationEnd(animation)
                    convertView.setLayerType(View.LAYER_TYPE_NONE, null)
                    listener?.onAnimationEnd()
                }
            },
        )
        rotateAnimator.start()
    }

    /**
     * Animate a view alpha property to a value.
     *
     * @param convertView the view to be animated
     * @param alpha       the value to animate to
     */
    @JvmStatic
    fun alpha(
        convertView: View,
        alpha: Float,
    ) {
        alpha(convertView, alpha, null)
    }

    /**
     * An interface definition that is invoked when an animation ends.
     */
    interface OnAnimationEndListener {
        fun onAnimationEnd()
    }
}
