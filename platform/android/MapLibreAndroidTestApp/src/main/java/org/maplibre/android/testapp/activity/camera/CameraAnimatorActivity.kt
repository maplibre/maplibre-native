package org.maplibre.android.testapp.activity.camera

import android.animation.Animator
import android.animation.AnimatorSet
import android.animation.TypeEvaluator
import android.animation.ValueAnimator
import android.os.Bundle
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.view.animation.AnticipateOvershootInterpolator
import android.view.animation.BounceInterpolator
import android.view.animation.Interpolator
import androidx.appcompat.app.AppCompatActivity
import androidx.collection.LongSparseArray
import androidx.core.view.animation.PathInterpolatorCompat
import androidx.interpolator.view.animation.FastOutLinearInInterpolator
import androidx.interpolator.view.animation.FastOutSlowInInterpolator
import org.maplibre.android.camera.CameraPosition
import org.maplibre.android.camera.CameraUpdateFactory
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.*
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles

/** Test activity showcasing using Android SDK animators to animate camera position changes. */
class CameraAnimatorActivity : AppCompatActivity(), OnMapReadyCallback {
    private val animators = LongSparseArray<Animator>()
    private lateinit var set: Animator
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_camera_animator)
        mapView = findViewById<View>(R.id.mapView) as MapView
        if (::mapView.isInitialized) {
            mapView.onCreate(savedInstanceState)
            mapView.getMapAsync(this)
        }
    }

    override fun onMapReady(map: MapLibreMap) {
        maplibreMap = map
        map.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
        initFab()
    }

    private fun initFab() {
        findViewById<View>(R.id.fab).setOnClickListener { view: View ->
            view.visibility = View.GONE
            val animatedPosition =
                CameraPosition.Builder()
                    .target(LatLng(37.789992, -122.402214))
                    .tilt(60.0)
                    .zoom(14.5)
                    .bearing(135.0)
                    .build()
            set = createExampleAnimator(maplibreMap.cameraPosition, animatedPosition)
            set.start()
        }
    }

    //
    // Animator API used for the animation on the FAB
    //
    private fun createExampleAnimator(
        currentPosition: CameraPosition,
        targetPosition: CameraPosition
    ): Animator {
        val animatorSet = AnimatorSet()
        animatorSet.play(createLatLngAnimator(currentPosition.target!!, targetPosition.target!!))
        animatorSet.play(createZoomAnimator(currentPosition.zoom, targetPosition.zoom))
        animatorSet.play(createBearingAnimator(currentPosition.bearing, targetPosition.bearing))
        animatorSet.play(createTiltAnimator(currentPosition.tilt, targetPosition.tilt))
        return animatorSet
    }

    private fun createLatLngAnimator(currentPosition: LatLng, targetPosition: LatLng): Animator {
        val latLngAnimator =
            ValueAnimator.ofObject(LatLngEvaluator(), currentPosition, targetPosition)
        latLngAnimator.duration = (1000 * ANIMATION_DELAY_FACTOR).toLong()
        latLngAnimator.interpolator = FastOutSlowInInterpolator()
        latLngAnimator.addUpdateListener { animation: ValueAnimator ->
            maplibreMap.moveCamera(
                CameraUpdateFactory.newLatLng((animation.animatedValue as LatLng))
            )
        }
        return latLngAnimator
    }

    private fun createZoomAnimator(currentZoom: Double, targetZoom: Double): Animator {
        val zoomAnimator = ValueAnimator.ofFloat(currentZoom.toFloat(), targetZoom.toFloat())
        zoomAnimator.duration = (2200 * ANIMATION_DELAY_FACTOR).toLong()
        zoomAnimator.startDelay = (600 * ANIMATION_DELAY_FACTOR).toLong()
        zoomAnimator.interpolator = AnticipateOvershootInterpolator()
        zoomAnimator.addUpdateListener { animation: ValueAnimator ->
            maplibreMap.moveCamera(
                CameraUpdateFactory.zoomTo((animation.animatedValue as Float).toDouble())
            )
        }
        return zoomAnimator
    }

    private fun createBearingAnimator(currentBearing: Double, targetBearing: Double): Animator {
        val bearingAnimator =
            ValueAnimator.ofFloat(currentBearing.toFloat(), targetBearing.toFloat())
        bearingAnimator.duration = (1000 * ANIMATION_DELAY_FACTOR).toLong()
        bearingAnimator.startDelay = (1000 * ANIMATION_DELAY_FACTOR).toLong()
        bearingAnimator.interpolator = FastOutLinearInInterpolator()
        bearingAnimator.addUpdateListener { animation: ValueAnimator ->
            maplibreMap.moveCamera(
                CameraUpdateFactory.bearingTo((animation.animatedValue as Float).toDouble())
            )
        }
        return bearingAnimator
    }

    private fun createTiltAnimator(currentTilt: Double, targetTilt: Double): Animator {
        val tiltAnimator = ValueAnimator.ofFloat(currentTilt.toFloat(), targetTilt.toFloat())
        tiltAnimator.duration = (1000 * ANIMATION_DELAY_FACTOR).toLong()
        tiltAnimator.startDelay = (1500 * ANIMATION_DELAY_FACTOR).toLong()
        tiltAnimator.addUpdateListener { animation: ValueAnimator ->
            maplibreMap.moveCamera(
                CameraUpdateFactory.tiltTo((animation.animatedValue as Float).toDouble())
            )
        }
        return tiltAnimator
    }

    //
    // Interpolator examples
    //
    private fun obtainExampleInterpolator(menuItemId: Int): Animator? {
        return animators[menuItemId.toLong()]
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        menuInflater.inflate(R.menu.menu_animator, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        if (!::maplibreMap.isInitialized) {
            return false
        }
        if (item.itemId != android.R.id.home) {
            findViewById<View>(R.id.fab).visibility = View.GONE
            resetCameraPosition()
            playAnimation(item.itemId)
        }
        return super.onOptionsItemSelected(item)
    }

    private fun resetCameraPosition() {
        maplibreMap.moveCamera(
            CameraUpdateFactory.newCameraPosition(
                CameraPosition.Builder()
                    .target(START_LAT_LNG)
                    .zoom(11.0)
                    .bearing(0.0)
                    .tilt(0.0)
                    .build()
            )
        )
    }

    private fun playAnimation(itemId: Int) {
        val animator = obtainExampleInterpolator(itemId)
        if (animator != null) {
            animator.cancel()
            animator.start()
        }
    }

    private fun obtainExampleInterpolator(interpolator: Interpolator, duration: Long): Animator {
        val zoomAnimator = ValueAnimator.ofFloat(11.0f, 16.0f)
        zoomAnimator.duration = (duration * ANIMATION_DELAY_FACTOR).toLong()
        zoomAnimator.interpolator = interpolator
        zoomAnimator.addUpdateListener { animation: ValueAnimator ->
            maplibreMap.moveCamera(
                CameraUpdateFactory.zoomTo((animation.animatedValue as Float).toDouble())
            )
        }
        return zoomAnimator
    }

    //
    // MapView lifecycle
    //
    override fun onStart() {
        super.onStart()
        mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        mapView.onStop()
        for (i in 0 until animators.size()) {
            animators[animators.keyAt(i)]!!.cancel()
        }
        if (set != null) {
            set.cancel()
        }
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        if (::mapView.isInitialized) {
            mapView.onDestroy()
        }
    }

    override fun onLowMemory() {
        super.onLowMemory()
        if (::mapView.isInitialized) {
            mapView.onLowMemory()
        }
    }

    /** Helper class to evaluate LatLng objects with a ValueAnimator */
    private class LatLngEvaluator : TypeEvaluator<LatLng> {
        private val latLng = LatLng()
        override fun evaluate(fraction: Float, startValue: LatLng, endValue: LatLng): LatLng {
            latLng.latitude = startValue.latitude + (endValue.latitude - startValue.latitude) * fraction
            latLng.longitude = startValue.longitude + (endValue.longitude - startValue.longitude) * fraction
            return latLng
        }
    }

    companion object {
        private const val ANIMATION_DELAY_FACTOR = 1.5
        private val START_LAT_LNG = LatLng(37.787947, -122.407432)
    }

    init {
        val accelerateDecelerateAnimatorSet = AnimatorSet()
        accelerateDecelerateAnimatorSet.playTogether(
            createLatLngAnimator(START_LAT_LNG, LatLng(37.826715, -122.422795)),
            obtainExampleInterpolator(FastOutSlowInInterpolator(), 2500)
        )
        animators.put(
            R.id.menu_action_accelerate_decelerate_interpolator.toLong(),
            accelerateDecelerateAnimatorSet
        )
        val bounceAnimatorSet = AnimatorSet()
        bounceAnimatorSet.playTogether(
            createLatLngAnimator(START_LAT_LNG, LatLng(37.787947, -122.407432)),
            obtainExampleInterpolator(BounceInterpolator(), 3750)
        )
        animators.put(R.id.menu_action_bounce_interpolator.toLong(), bounceAnimatorSet)
        animators.put(
            R.id.menu_action_anticipate_overshoot_interpolator.toLong(),
            obtainExampleInterpolator(AnticipateOvershootInterpolator(), 2500)
        )
        animators.put(
            R.id.menu_action_path_interpolator.toLong(),
            obtainExampleInterpolator(
                PathInterpolatorCompat.create(.22f, .68f, 0f, 1.71f),
                2500
            )
        )
    }
}
