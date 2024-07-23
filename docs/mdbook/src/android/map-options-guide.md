# MapLibreMapOptions
This guide will explain different ways to create a map

When working with maps there are chances of providing default values for MapView to render.
There are several ways to build MapView:
1. Providing existing XML namespace tags of MapView in layout
2. Creating `MapLibreMapOptions` and providing builder function values into MapView
3. Creating `SupportMapFragment` with the help of `MapLibreMapOptions`

Before explaining MapView configurations we need to know what we can do with both XML namespaces and `MapLibreMaptions`


Common configurations can be set:
- Map base uri
- Camera
- Zoom
- Pitch
- Gestures
- Compass
- Logo
- Attribution
- Placement of above elements on the Map and more

We will see how to achieve these configurations in XML layout and programmatically in Activity code step by step

1. Creating MapView namespace and provide some data in layout file
  ```xml
<?xml version="1.0" encoding="utf-8"?>
<androidx.constraintlayout.widget.ConstraintLayout xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:app="http://schemas.android.com/apk/res-auto"
    xmlns:tools="http://schemas.android.com/tools"
    android:id="@+id/main"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    tools:context=".activity.options.MapOptionsXmlActivity">

    <org.maplibre.android.maps.MapView
        android:id="@+id/mapView"
        android:layout_width="match_parent"
        android:layout_height="match_parent"
        app:maplibre_apiBaseUri="https://api.maplibre.org"
        app:maplibre_cameraBearing="34.0"
        app:maplibre_cameraPitchMax="90.0"
        app:maplibre_cameraPitchMin="0.0"
        app:maplibre_cameraTargetLat="52.519003"
        app:maplibre_cameraTargetLng="13.400972"
        app:maplibre_cameraTilt="25.0"
        app:maplibre_cameraZoom="16"
        app:maplibre_cameraZoomMax="34.0"
        app:maplibre_cameraZoomMin="15.0"
        app:maplibre_localIdeographFontFamilies="@array/array_local_ideograph_family_test"
        app:maplibre_localIdeographFontFamily="Droid Sans"
        app:maplibre_uiCompass="true"
        app:maplibre_uiCompassFadeFacingNorth="true"
        app:maplibre_uiCompassGravity="top|end"
        app:maplibre_uiDoubleTapGestures="true"
        app:maplibre_uiHorizontalScrollGestures="true"
        app:maplibre_uiRotateGestures="true"
        app:maplibre_uiScrollGestures="true"
        app:maplibre_uiTiltGestures="true"
        app:maplibre_uiZoomGestures="true" />

</androidx.constraintlayout.widget.ConstraintLayout>
```
We can give any other existing values to `maplibre` tags and  only need to create Mapview and MapLibreMap objects with simple setup in Activity
```kotlin
class MapOptionsXmlActivity : AppCompatActivity(), OnMapReadyCallback {
    private lateinit var mapView: MapView
    private lateinit var maplibreMap: MapLibreMap

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_options_xml)
        mapView = findViewById(R.id.mapView)
        mapView.onCreate(savedInstanceState)
        mapView.getMapAsync(this)
    }

    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        this.maplibreMap.setStyle(
            Style.Builder().fromUri(TestStyles.getPredefinedStyleWithFallback("Streets"))
        )
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
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
```
2. Creating `MapLibreMapOptions` and with MapView constructor. Here we don't have to create MapView from xml since we want to create it programmatically
```xml
<?xml version="1.0" encoding="utf-8"?>
<FrameLayout
    xmlns:android="http://schemas.android.com/apk/res/android"
    android:id="@+id/container"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical"/>
```

And all setup happens in Activity code:
```kotlin
class MapOptionsRuntimeActivity : AppCompatActivity(), OnMapReadyCallback {

    private lateinit var maplibreMap: MapLibreMap
    private lateinit var mapView: MapView
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_options_runtime)

        // Create map configuration
        val maplibreMapOptions = MapLibreMapOptions.createFromAttributes(this)
        maplibreMapOptions.apply {
            apiBaseUri("https://api.maplibre.org")
            camera(CameraPosition.Builder()
                .bearing(34.0)
                .target(LatLng(52.519003, 13.400972))
                .zoom(16.0)
                .tilt(25.0)
                .build()
            )
            maxPitchPreference(90.0)
            minPitchPreference(0.0)
            maxZoomPreference(34.0)
            minZoomPreference(15.0)
            localIdeographFontFamily("Droid Sans")
            zoomGesturesEnabled(true)
            compassEnabled(true)
            compassFadesWhenFacingNorth(true)
            scrollGesturesEnabled(true)
            rotateGesturesEnabled(true)
            tiltGesturesEnabled(true)
        }

        // Create map programmatically, add to view hierarchy
        mapView = MapView(this, maplibreMapOptions)
        mapView.getMapAsync(this)
        mapView.onCreate(savedInstanceState)
        (findViewById<View>(R.id.container) as ViewGroup).addView(mapView)
    }

    override fun onMapReady(maplibreMap: MapLibreMap) {
        this.maplibreMap = maplibreMap
        this.maplibreMap.setStyle(
            Style.Builder().fromUri(TestStyles.getPredefinedStyleWithFallback("Streets"))
        )
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
        mapView.onDestroy()
    }

    override fun onLowMemory() {
        super.onLowMemory()
        mapView.onLowMemory()
    }
}
```
Finally we will see a result similar to this :
<div style="align: center">
  <img src="https://github.com/user-attachments/assets/1749e7aa-e836-4ccd-8777-bd6994395e25" alt="Screenshot with the MapLibreMapOptions">
</div>

For full content of `MapOptionsRuntimeActivity` and `MapOptionsXmlActivity`, please visit source code of [Test APP](https://github.com/jDilshodbek/maplibre-gl-native/tree/feature/documentation-maplibre-map-options/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/options)

You can read more about `MapLibreMapOptions` in the [documentation](https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.maps/-map-libre-map-options/index.html?query=open%20class%20MapLibreMapOptions%20:%20Parcelable)

3. Creating `SupportMapFragment` with the help of `MapLibreMapOptions`

 If you are using MapFragment in your project, it is also easy to give initial values to the `newInstance()` static method of `SupportMapFragment` which requires passing `MapLibreMapOptions` parameter

Let's see how we can do in a sample activity:

```kotlin
class SupportMapFragmentActivity :
    AppCompatActivity(),
    OnMapViewReadyCallback,
    OnMapReadyCallback,
    OnDidFinishRenderingFrameListener {
    private lateinit var maplibreMap: MapLibreMap
    private lateinit var mapView: MapView
    private var initialCameraAnimation = true
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_map_fragment)
        val mapFragment: SupportMapFragment?
        if (savedInstanceState == null) {
            mapFragment = SupportMapFragment.newInstance(createFragmentOptions())
            supportFragmentManager
                .beginTransaction()
                .add(R.id.fragment_container, mapFragment, TAG)
                .commit()
        } else {
            mapFragment = supportFragmentManager.findFragmentByTag(TAG) as SupportMapFragment?
        }
        mapFragment!!.getMapAsync(this)
    }

    private fun createFragmentOptions(): MapLibreMapOptions {
        val options = MapLibreMapOptions.createFromAttributes(this, null)
        options.scrollGesturesEnabled(false)
        options.zoomGesturesEnabled(false)
        options.tiltGesturesEnabled(false)
        options.rotateGesturesEnabled(false)
        options.debugActive(false)
        val dc = LatLng(38.90252, -77.02291)
        options.minZoomPreference(9.0)
        options.maxZoomPreference(11.0)
        options.camera(
            CameraPosition.Builder()
                .target(dc)
                .zoom(11.0)
                .build()
        )
        return options
    }

    override fun onMapViewReady(map: MapView) {
        mapView = map
        mapView.addOnDidFinishRenderingFrameListener(this)
    }

    override fun onMapReady(map: MapLibreMap) {
        maplibreMap = map
        maplibreMap.setStyle(TestStyles.getPredefinedStyleWithFallback("Satellite Hybrid"))
    }

    override fun onDestroy() {
        super.onDestroy()
        mapView.removeOnDidFinishRenderingFrameListener(this)
    }

    override fun onDidFinishRenderingFrame(fully: Boolean, frameEncodingTime: Double, frameRenderingTime: Double) {
        if (initialCameraAnimation && fully && maplibreMap != null) {
            maplibreMap.animateCamera(
                CameraUpdateFactory.newCameraPosition(CameraPosition.Builder().tilt(45.0).build()),
                5000
            )
            initialCameraAnimation = false
        }
    }

    companion object {
        private const val TAG = "com.mapbox.map"
    }
}
```
You can also find full contents of `SupportMapFragmentActivity` in the [Test App](https://github.com/jDilshodbek/maplibre-gl-native/blob/feature/documentation-maplibre-map-options/platform/android/MapLibreAndroidTestApp/src/main/java/org/maplibre/android/testapp/activity/fragment/SupportMapFragmentActivity.kt)

To learn more about `SupportFragment` , please visit the [documentation](https://maplibre.org/maplibre-native/android/api/-map-libre%20-native%20-android/org.maplibre.android.maps/-support-map-fragment/index.html?query=open%20class%20SupportMapFragment%20:%20Fragment,%20OnMapReadyCallback)
