package org.maplibre.android.testapp.model.annotations

import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker

class CountryMarker(
    baseMarkerOptions: BaseMarkerOptions<*, *>?,
    val abbrevName: String,
    val flagRes: Int
) : Marker(baseMarkerOptions)
