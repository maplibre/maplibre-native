package org.maplibre.android.testapp.activity.style

import android.animation.Animator
import android.animation.AnimatorListenerAdapter
import android.animation.TypeEvaluator
import android.animation.ValueAnimator
import android.animation.ValueAnimator.AnimatorUpdateListener
import android.graphics.drawable.BitmapDrawable
import android.os.Bundle
import android.view.animation.AccelerateDecelerateInterpolator
import android.view.animation.LinearInterpolator
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.res.ResourcesCompat
import com.google.gson.JsonObject
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.android.geometry.LatLng
import org.maplibre.android.maps.MapView
import org.maplibre.android.maps.MapLibreMap
import org.maplibre.android.maps.OnMapReadyCallback
import org.maplibre.android.maps.Style
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.SymbolLayer
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.turf.TurfMeasurement
import java.util.*

/**
 * Test activity showcasing animating a SymbolLayer.
 */
class AnimatedSymbolLayerActivity : AppCompatActivity() {
    private val random = Random()
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap
    private var style: Style? = null
    private val randomCars: MutableList<Car> = ArrayList()
    private var randomCarSource: GeoJsonSource? = null
    private var taxi: Car? = null
    private var taxiSource: GeoJsonSource? = null
    private var passenger: LatLng? = null
    private val animators: MutableList<Animator> = ArrayList()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_animated_marker)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(
            OnMapReadyCallback { map: MapLibreMap ->
                maplibreMap = map
                map.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style? ->
                    this.style = style
                    setupCars()
                    animateRandomRoutes()
                    animateTaxi()
                }
            }
        )
    }

    private fun setupCars() {
        addRandomCars()
        addPassenger()
        addMainCar()
    }

    private fun animateRandomRoutes() {
        val longestDrive = longestDrive
        val random = Random()
        for (car in randomCars) {
            val isLongestDrive = longestDrive == car
            val valueAnimator = ValueAnimator.ofObject(LatLngEvaluator(), car.current, car.next)
            valueAnimator.addUpdateListener(object : AnimatorUpdateListener {
                private var latLng: LatLng? = null
                override fun onAnimationUpdate(animation: ValueAnimator) {
                    latLng = animation.animatedValue as LatLng
                    car.current = latLng
                    if (isLongestDrive) {
                        updateRandomCarSource()
                    }
                }
            })
            if (isLongestDrive) {
                valueAnimator.addListener(object : AnimatorListenerAdapter() {
                    override fun onAnimationEnd(animation: Animator) {
                        super.onAnimationEnd(animation)
                        updateRandomDestinations()
                        animateRandomRoutes()
                    }
                })
            }
            valueAnimator.addListener(object : AnimatorListenerAdapter() {
                override fun onAnimationStart(animation: Animator) {
                    super.onAnimationStart(animation)
                    car.feature.properties()!!
                        .addProperty("bearing", Car.getBearing(car.current, car.next))
                }
            })
            val offset = if (random.nextInt(2) == 0) 0 else random.nextInt(1000) + 250
            valueAnimator.startDelay = offset.toLong()
            valueAnimator.duration = car.duration - offset
            valueAnimator.interpolator = LinearInterpolator()
            valueAnimator.start()
            animators.add(valueAnimator)
        }
    }

    private fun animateTaxi() {
        val valueAnimator = ValueAnimator.ofObject(LatLngEvaluator(), taxi!!.current, taxi!!.next)
        valueAnimator.addUpdateListener(object : AnimatorUpdateListener {
            private var latLng: LatLng? = null
            override fun onAnimationUpdate(animation: ValueAnimator) {
                latLng = animation.animatedValue as LatLng
                taxi!!.current = latLng
                updateTaxiSource()
            }
        })
        valueAnimator.addListener(object : AnimatorListenerAdapter() {
            override fun onAnimationEnd(animation: Animator) {
                super.onAnimationEnd(animation)
                updatePassenger()
                animateTaxi()
            }
        })
        valueAnimator.addListener(object : AnimatorListenerAdapter() {
            override fun onAnimationStart(animation: Animator) {
                super.onAnimationStart(animation)
                taxi!!.feature.properties()!!
                    .addProperty("bearing", Car.getBearing(taxi!!.current, taxi!!.next))
            }
        })
        valueAnimator.duration = (7 * taxi!!.current!!.distanceTo(taxi!!.next!!)).toLong()
        valueAnimator.interpolator = AccelerateDecelerateInterpolator()
        valueAnimator.start()
        animators.add(valueAnimator)
    }

    private fun updatePassenger() {
        passenger = latLngInBounds
        updatePassengerSource()
        taxi!!.setNext(passenger)
    }

    private fun updatePassengerSource() {
        val source = style!!.getSourceAs<GeoJsonSource>(PASSENGER_SOURCE)
        val featureCollection = FeatureCollection.fromFeatures(
            arrayOf(
                Feature.fromGeometry(
                    Point.fromLngLat(
                        passenger!!.longitude,
                        passenger!!.latitude
                    )
                )
            )
        )
        source!!.setGeoJson(featureCollection)
    }

    private fun updateTaxiSource() {
        taxi!!.updateFeature()
        taxiSource!!.setGeoJson(taxi!!.feature)
    }

    private fun updateRandomDestinations() {
        for (randomCar in randomCars) {
            randomCar.setNext(latLngInBounds)
        }
    }

    private val longestDrive: Car?
        private get() {
            var longestDrive: Car? = null
            for (randomCar in randomCars) {
                if (longestDrive == null) {
                    longestDrive = randomCar
                } else if (longestDrive.duration < randomCar.duration) {
                    longestDrive = randomCar
                }
            }
            return longestDrive
        }

    private fun updateRandomCarSource() {
        for (randomCarsRoute in randomCars) {
            randomCarsRoute.updateFeature()
        }
        randomCarSource!!.setGeoJson(featuresFromRoutes())
    }

    private fun featuresFromRoutes(): FeatureCollection {
        val features: MutableList<Feature> = ArrayList()
        for (randomCarsRoute in randomCars) {
            features.add(randomCarsRoute.feature)
        }
        return FeatureCollection.fromFeatures(features)
    }

    private val duration: Long
        private get() = (random.nextInt(DURATION_RANDOM_MAX) + DURATION_BASE).toLong()

    private fun addRandomCars() {
        var latLng: LatLng
        var next: LatLng
        for (i in 0..9) {
            latLng = latLngInBounds
            next = latLngInBounds
            val properties = JsonObject()
            properties.addProperty(PROPERTY_BEARING, Car.getBearing(latLng, next))
            val feature = Feature.fromGeometry(
                Point.fromLngLat(
                    latLng.longitude,
                    latLng.latitude
                ),
                properties
            )
            randomCars.add(
                Car(feature, next, duration)
            )
        }
        randomCarSource = GeoJsonSource(RANDOM_CAR_SOURCE, featuresFromRoutes())
        style!!.addSource(randomCarSource!!)
        style!!.addImage(
            RANDOM_CAR_IMAGE_ID,
            (ResourcesCompat.getDrawable(resources, R.drawable.ic_car_top, null) as BitmapDrawable).bitmap
        )
        val symbolLayer = SymbolLayer(RANDOM_CAR_LAYER, RANDOM_CAR_SOURCE)
        symbolLayer.withProperties(
            PropertyFactory.iconImage(RANDOM_CAR_IMAGE_ID),
            PropertyFactory.iconAllowOverlap(true),
            PropertyFactory.iconRotate(Expression.get(PROPERTY_BEARING)),
            PropertyFactory.iconIgnorePlacement(true)
        )
        style!!.addLayerBelow(symbolLayer, WATERWAY_LAYER_ID)
    }

    private fun addPassenger() {
        passenger = latLngInBounds
        val featureCollection = FeatureCollection.fromFeatures(
            arrayOf(
                Feature.fromGeometry(
                    Point.fromLngLat(
                        passenger!!.longitude,
                        passenger!!.latitude
                    )
                )
            )
        )
        style!!.addImage(
            PASSENGER,
            (ResourcesCompat.getDrawable(resources, R.drawable.icon_burned, null) as BitmapDrawable).bitmap
        )
        val geoJsonSource = GeoJsonSource(PASSENGER_SOURCE, featureCollection)
        style!!.addSource(geoJsonSource)
        val symbolLayer = SymbolLayer(PASSENGER_LAYER, PASSENGER_SOURCE)
        symbolLayer.withProperties(
            PropertyFactory.iconImage(PASSENGER),
            PropertyFactory.iconIgnorePlacement(true),
            PropertyFactory.iconAllowOverlap(true)
        )
        style!!.addLayerBelow(symbolLayer, RANDOM_CAR_LAYER)
    }

    private fun addMainCar() {
        val latLng = latLngInBounds
        val properties = JsonObject()
        properties.addProperty(PROPERTY_BEARING, Car.getBearing(latLng, passenger))
        val feature = Feature.fromGeometry(
            Point.fromLngLat(
                latLng.longitude,
                latLng.latitude
            ),
            properties
        )
        val featureCollection = FeatureCollection.fromFeatures(arrayOf(feature))
        taxi = Car(feature, passenger, duration)
        style!!.addImage(
            TAXI,
            (ResourcesCompat.getDrawable(resources, R.drawable.ic_taxi_top, null) as BitmapDrawable).bitmap
        )
        taxiSource = GeoJsonSource(TAXI_SOURCE, featureCollection)
        style!!.addSource(taxiSource!!)
        val symbolLayer = SymbolLayer(TAXI_LAYER, TAXI_SOURCE)
        symbolLayer.withProperties(
            PropertyFactory.iconImage(TAXI),
            PropertyFactory.iconRotate(Expression.get(PROPERTY_BEARING)),
            PropertyFactory.iconAllowOverlap(true),
            PropertyFactory.iconIgnorePlacement(true)
        )
        style!!.addLayer(symbolLayer)
    }

    private val latLngInBounds: LatLng
        get() {
            val bounds = maplibreMap.projection.visibleRegion.latLngBounds
            val generator = Random()
            val randomLat = bounds.latitudeSouth + generator.nextDouble() * bounds.latitudeNorth - bounds.latitudeSouth
            val randomLon = bounds.longitudeWest + generator.nextDouble() * bounds.longitudeEast - bounds.longitudeWest
            return LatLng(randomLat, randomLon)
        }

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
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onDestroy() {
        super.onDestroy()
        for (animator in animators) {
            if (animator != null) {
                animator.removeAllListeners()
                animator.cancel()
            }
        }
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    /**
     * Evaluator for LatLng pairs
     */
    private class LatLngEvaluator : TypeEvaluator<LatLng> {
        private val latLng = LatLng()
        override fun evaluate(fraction: Float, startValue: LatLng, endValue: LatLng): LatLng {
            latLng.latitude = startValue.latitude + (endValue.latitude - startValue.latitude) * fraction
            latLng.longitude = startValue.longitude + (endValue.longitude - startValue.longitude) * fraction
            return latLng
        }
    }

    private class Car internal constructor(var feature: Feature, next: LatLng?, duration: Long) {
        var next: LatLng?
        var current: LatLng?
        val duration: Long

        @JvmName("setNext1")
        fun setNext(next: LatLng?) {
            this.next = next
        }

        fun updateFeature() {
            feature = Feature.fromGeometry(
                Point.fromLngLat(
                    current!!.longitude,
                    current!!.latitude
                )
            )
            feature.properties()!!.addProperty("bearing", getBearing(current, next))
        }

        companion object {
            fun getBearing(from: LatLng?, to: LatLng?): Float {
                return TurfMeasurement.bearing(
                    Point.fromLngLat(from!!.longitude, from.latitude),
                    Point.fromLngLat(to!!.longitude, to.latitude)
                ).toFloat()
            }
        }

        init {
            val point = feature.geometry() as Point?
            current = LatLng(point!!.latitude(), point.longitude())
            this.duration = duration
            this.next = next
        }
    }

    companion object {
        private const val PASSENGER = "passenger"
        private const val PASSENGER_LAYER = "passenger-layer"
        private const val PASSENGER_SOURCE = "passenger-source"
        private const val TAXI = "taxi"
        private const val TAXI_LAYER = "taxi-layer"
        private const val TAXI_SOURCE = "taxi-source"
        private const val RANDOM_CAR_LAYER = "random-car-layer"
        private const val RANDOM_CAR_SOURCE = "random-car-source"
        private const val RANDOM_CAR_IMAGE_ID = "random-car"
        private const val PROPERTY_BEARING = "bearing"
        private const val WATERWAY_LAYER_ID = "water_intermittent"
        private const val DURATION_RANDOM_MAX = 1500
        private const val DURATION_BASE = 3000
    }
}
