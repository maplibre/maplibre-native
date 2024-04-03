package org.maplibre.android.testapp.model.annotations

import org.maplibre.android.annotations.Marker

class CityStateMarker(
    cityStateOptions: CityStateMarkerOptions?,
    val infoWindowBackgroundColor: String
) : Marker(cityStateOptions)
