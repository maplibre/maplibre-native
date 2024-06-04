package org.maplibre.android.testapp.activity.style

import android.os.AsyncTask
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import org.maplibre.geojson.Feature
import org.maplibre.geojson.FeatureCollection
import org.maplibre.geojson.Point
import org.maplibre.android.maps.*
import org.maplibre.android.style.expressions.Expression
import org.maplibre.android.style.layers.*
import org.maplibre.android.style.sources.GeoJsonSource
import org.maplibre.android.testapp.R
import org.maplibre.android.testapp.styles.TestStyles
import org.maplibre.android.testapp.utils.GeoParseUtil
import org.maplibre.android.utils.BitmapUtils
import timber.log.Timber
import java.io.IOException
import java.lang.ref.WeakReference
import java.util.ArrayList

/**
 * Test stretchable image as a background for text..
 */
class StretchableImageActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var mapView: MapView
    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_stretchable_image)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Streets")) { style: Style ->
            val popup = BitmapUtils.getBitmapFromDrawable(
                resources.getDrawable(R.drawable.popup)
            )
            val popupDebug =
                BitmapUtils.getBitmapFromDrawable(resources.getDrawable(R.drawable.popup_debug))

            // The two (blue) columns of pixels that can be stretched horizontally:
            //   - the pixels between x: 25 and x: 55 can be stretched
            //   - the pixels between x: 85 and x: 115 can be stretched.
            val stretchX: MutableList<ImageStretches> = ArrayList()
            stretchX.add(ImageStretches(25F, 55F))
            stretchX.add(ImageStretches(85F, 115F))

            // The one (red) row of pixels that can be stretched vertically:
            //   - the pixels between y: 25 and y: 100 can be stretched
            val stretchY: MutableList<ImageStretches> = ArrayList()
            stretchY.add(ImageStretches(25F, 100F))

            // This part of the image that can contain text ([x1, y1, x2, y2]):
            val content = ImageContent(25F, 25F, 115F, 100F)
            style.addImage(NAME_POPUP, popup!!, stretchX, stretchY, content)
            style.addImage(NAME_POPUP_DEBUG, popupDebug!!, stretchX, stretchY, content)
            LoadFeatureTask(this@StretchableImageActivity).execute()
        }
    }

    private fun onFeatureLoaded(json: String?) {
        if (json == null) {
            Timber.e("json is null.")
            return
        }
        val style = maplibreMap.style
        if (style != null) {
            val featureCollection = FeatureCollection.fromJson(json)
            val stretchSource = GeoJsonSource(STRETCH_SOURCE, featureCollection)
            val stretchLayer = SymbolLayer(STRETCH_LAYER, STRETCH_SOURCE)
                .withProperties(
                    PropertyFactory.textField(Expression.get("name")),
                    PropertyFactory.iconImage(Expression.get("image-name")),
                    PropertyFactory.iconAllowOverlap(true),
                    PropertyFactory.textAllowOverlap(true),
                    PropertyFactory.iconTextFit(Property.ICON_TEXT_FIT_BOTH)
                )

            // the original, unstretched image for comparison
            val point = Point.fromLngLat(-70.0, 0.0)
            val feature = Feature.fromGeometry(point)
            val originalCollection = FeatureCollection.fromFeature(feature)
            val originalSource = GeoJsonSource(ORIGINAL_SOURCE, originalCollection)
            val originalLayer = SymbolLayer(ORIGINAL_LAYER, ORIGINAL_SOURCE)
            style.addSource(stretchSource)
            style.addSource(originalSource)
            style.addLayer(stretchLayer)
            style.addLayer(originalLayer)
        }
    }

    private class LoadFeatureTask(activity: StretchableImageActivity) :
        AsyncTask<Void?, Int?, String?>() {
        private val activity: WeakReference<StretchableImageActivity>
        protected override fun doInBackground(vararg p0: Void?): String? {
            val activity = activity.get()
            if (activity != null) {
                var json: String? = null
                try {
                    json = GeoParseUtil.loadStringFromAssets(
                        activity.applicationContext,
                        "stretchable_image.geojson"
                    )
                } catch (exception: IOException) {
                    Timber.e(exception, "Could not read feature")
                }
                return json
            }
            return null
        }

        override fun onPostExecute(json: String?) {
            super.onPostExecute(json)
            val activity = activity.get()
            activity?.onFeatureLoaded(json)
        }

        init {
            this.activity = WeakReference(activity)
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
        private const val NAME_POPUP = "popup"
        private const val NAME_POPUP_DEBUG = "popup-debug"
        private const val STRETCH_SOURCE = "STRETCH_SOURCE"
        private const val STRETCH_LAYER = "STRETCH_LAYER"
        private const val ORIGINAL_SOURCE = "ORIGINAL_SOURCE"
        private const val ORIGINAL_LAYER = "ORIGINAL_LAYER"
    }
}
