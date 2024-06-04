package org.maplibre.android.testapp.activity.maplayout

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.*
import android.os.Bundle
import android.os.PersistableBundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.android.testapp.databinding.ActivityOverlayBinding
import org.maplibre.android.testapp.styles.TestStyles

/**
 * Test overlaying a Map with a View that uses a radial gradient shader.
 */
class OverlayMapActivity : AppCompatActivity() {

    private lateinit var binding: ActivityOverlayBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityOverlayBinding.inflate(layoutInflater)
        setContentView(binding.root)
        binding.mapView.onCreate(savedInstanceState)
        binding.parentView.addView(OverlayView(this))
        binding.mapView.getMapAsync {
            it.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets"))
        }
    }

    override fun onStart() {
        super.onStart()
        binding.mapView.onStart()
    }

    override fun onResume() {
        super.onResume()
        binding.mapView.onResume()
    }

    override fun onPause() {
        super.onPause()
        binding.mapView.onPause()
    }

    override fun onStop() {
        super.onStop()
        binding.mapView.onStop()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        binding.mapView.onLowMemory()
    }

    override fun onDestroy() {
        super.onDestroy()
        binding.mapView.onDestroy()
    }

    override fun onSaveInstanceState(outState: Bundle, outPersistentState: PersistableBundle) {
        super.onSaveInstanceState(outState, outPersistentState)
        outState.let {
            binding.mapView.onSaveInstanceState(it)
        }
    }

    class OverlayView(context: Context) : View(context) {

        private lateinit var paint: Paint

        @SuppressLint("DrawAllocation") // only happens once
        override fun onDraw(canvas: Canvas) {
            super.onDraw(canvas)

            canvas.let {
                if (!::paint.isInitialized) {
                    paint = Paint()
                    paint.color = Color.BLACK
                    paint.strokeWidth = 1.0f
                    paint.style = Paint.Style.FILL_AND_STROKE
                    paint.shader = RadialGradient(
                        width / 2.0f,
                        height / 2.0f,
                        height / 3.0f,
                        Color.TRANSPARENT,
                        Color.BLACK,
                        Shader.TileMode.CLAMP
                    )
                }
                it.drawRect(0.0f, 0.0f, width.toFloat(), height.toFloat(), paint)
            }
        }
    }
}
