package com.mapbox.mapboxsdk.testapp.activity.style

import android.graphics.Color
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.mapbox.geojson.Feature
import com.mapbox.geojson.FeatureCollection
import com.mapbox.geojson.Point
import com.mapbox.mapboxsdk.maps.MapView
import com.mapbox.mapboxsdk.maps.MapboxMap
import com.mapbox.mapboxsdk.maps.OnMapReadyCallback
import com.mapbox.mapboxsdk.maps.Style
import com.mapbox.mapboxsdk.style.expressions.Expression
import com.mapbox.mapboxsdk.style.expressions.Expression.FormatOption
import com.mapbox.mapboxsdk.style.layers.PropertyFactory
import com.mapbox.mapboxsdk.style.layers.SymbolLayer
import com.mapbox.mapboxsdk.style.sources.GeoJsonSource
import com.mapbox.mapboxsdk.testapp.R
import com.mapbox.mapboxsdk.utils.BitmapUtils

/**
 * Test image in label.
 */
class ImageInLabelActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapView: MapView
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_stretchable_image)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(mapboxMap: MapboxMap) {
        mapboxMap.setStyle(Style.getPredefinedStyle("Streets")) { style: Style ->
            val us = BitmapUtils.getBitmapFromDrawable(
                resources.getDrawable(R.drawable.ic_us)
            )
            val android =
                BitmapUtils.getBitmapFromDrawable(resources.getDrawable(R.drawable.ic_android))
            style.addImage("us", us!!)
            style.addImage("android", android!!)
            val point = Point.fromLngLat(-10.0, 0.0)
            val feature = Feature.fromGeometry(point)
            val originalCollection = FeatureCollection.fromFeature(feature)
            val originalSource = GeoJsonSource(ORIGINAL_SOURCE, originalCollection)
            val originalLayer = SymbolLayer(ORIGINAL_LAYER, ORIGINAL_SOURCE)
                .withProperties(
                    PropertyFactory.textAllowOverlap(true),
                    PropertyFactory.textField(
                        Expression
                            .format(
                                Expression.formatEntry(
                                    Expression.literal("Android: "),
                                    FormatOption.formatFontScale(1.0),
                                    FormatOption.formatTextColor(Color.BLUE),
                                    FormatOption.formatTextFont(
                                        arrayOf(
                                            "Ubuntu Medium",
                                            "Arial Unicode MS Regular"
                                        )
                                    )
                                ),
                                Expression.formatEntry(Expression.image(Expression.literal("android"))),
                                Expression.formatEntry(
                                    Expression.literal("Us: "),
                                    FormatOption.formatFontScale(1.5),
                                    FormatOption.formatTextColor(Color.YELLOW)
                                ),
                                Expression.formatEntry(Expression.image(Expression.literal("us"))),
                                Expression.formatEntry(
                                    Expression.literal("suffix"),
                                    FormatOption.formatFontScale(2.0),
                                    FormatOption.formatTextColor(Color.CYAN)
                                )
                            )
                    )
                )
            style.addSource(originalSource)
            style.addLayer(originalLayer)
        }
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

    public override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        mapView.onSaveInstanceState(outState)
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }

    public override fun onDestroy() {
        super.onDestroy()
        mapView.onDestroy()
    }

    companion object {
        private const val ORIGINAL_SOURCE = "ORIGINAL_SOURCE"
        private const val ORIGINAL_LAYER = "ORIGINAL_LAYER"
    }
}
