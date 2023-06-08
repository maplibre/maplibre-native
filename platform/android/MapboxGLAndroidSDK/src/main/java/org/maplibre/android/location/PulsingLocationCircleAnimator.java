package org.maplibre.android.location;

/**
 * Manages the logic of the interpolated animation which is applied to the LocationComponent's pulsing circle
 */
public class PulsingLocationCircleAnimator extends MapLibreFloatAnimator {

  /**
   *
   * @param updateListener  the {@link AnimationsValueChangeListener} associated with this animator.
   * @param maxAnimationFps the maximum frames per second that the animator should use. Default
   *                        is the {@link LocationAnimatorCoordinator#maxAnimationFps} variable.
   */
  public PulsingLocationCircleAnimator(AnimationsValueChangeListener updateListener,
                                       int maxAnimationFps,
                                       float circleMaxRadius) {
    super(new Float[]{0f, circleMaxRadius}, updateListener, maxAnimationFps);
  }
}
