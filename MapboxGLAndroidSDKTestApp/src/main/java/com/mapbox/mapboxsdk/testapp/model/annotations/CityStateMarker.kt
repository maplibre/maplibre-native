package com.mapbox.mapboxsdk.testapp.model.annotations

import com.mapbox.mapboxsdk.annotations.Marker

class CityStateMarker(
    cityStateOptions: CityStateMarkerOptions?,
    val infoWindowBackgroundColor: String
) : Marker(cityStateOptions)
