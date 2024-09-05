package org.maplibre.androidautoapp

import androidx.car.app.CarContext
import androidx.car.app.Screen
import androidx.car.app.SurfaceCallback
import androidx.car.app.SurfaceContainer
import androidx.car.app.model.Action
import androidx.car.app.model.ActionStrip
import androidx.car.app.model.CarColor
import androidx.car.app.model.CarIcon
import androidx.car.app.model.Template
import androidx.car.app.navigation.model.NavigationTemplate
import androidx.core.graphics.drawable.IconCompat
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style

class MapLibreCarAppScreen(
    carContext: CarContext,
    carSurface: MapLibreAndroidAutoSurface,
) : Screen(carContext) {

    private var mapLibreMap: MapLibreMap? = null

    private val mSurfaceCallback: SurfaceCallback = object : SurfaceCallback {
        override fun onSurfaceAvailable(container: SurfaceContainer) {
            super.onSurfaceAvailable(container)
        }

        override fun onClick(x: Float, y: Float) {
            super.onClick(x, y)
        }

        override fun onFling(velocityX: Float, velocityY: Float) {
            super.onFling(velocityX, velocityY)
        }

        override fun onScroll(distanceX: Float, distanceY: Float) {
            super.onScroll(distanceX, distanceY)
        }

        override fun onScale(focusX: Float, focusY: Float, scaleFactor: Float) {
            super.onScale(focusX, focusY, scaleFactor)
        }
    }

    init {
        carSurface.apply {
            addOnSurfaceCallbackListener(mSurfaceCallback)
            init(
                Style.Builder()
                    .fromUri("YOUR_STYLE_URI"),
                OnMapReadyCallback {
                    mapLibreMap = it
                }
            )
        }
    }

    override fun onGetTemplate(): Template {
        val builder = NavigationTemplate.Builder()
        builder.setBackgroundColor(CarColor.SECONDARY)

        val actionStripBuilder = ActionStrip.Builder()
        actionStripBuilder.addAction(
            Action.Builder()
                .setTitle("Back")
                .setOnClickListener {
                    // TODO: Handle back action.
                }
                .build()
        )

        builder.setActionStrip(actionStripBuilder.build())
        val panIconBuilder = CarIcon.Builder(
            IconCompat.createWithResource(
                carContext,
                R.drawable.maplibre_mylocation_bg_shape
            )
        )
        builder.setMapActionStrip(
            ActionStrip.Builder()
                .addAction(
                    Action.Builder(Action.PAN)
                        .setIcon(panIconBuilder.build())
                        .build()
                )
                .addAction(
                    Action.Builder()
                        .setIcon(
                            CarIcon.Builder(
                                IconCompat.createWithResource(
                                    carContext,
                                    R.drawable.maplibre_mylocation_bg_shape
                                )
                            )
                                .build()
                        )
                        .setOnClickListener {
                            // TODO: Handle onClick action.
                        }
                        .build())
                .addAction(
                    Action.Builder()
                        .setIcon(
                            CarIcon.Builder(
                                IconCompat.createWithResource(
                                    carContext,
                                    R.drawable.maplibre_mylocation_bg_shape
                                )
                            )
                                .build()
                        )
                        .setOnClickListener {
                            // TODO: Handle onClick action.
                        }
                        .build())
                .build())
        return builder.build()
    }
}