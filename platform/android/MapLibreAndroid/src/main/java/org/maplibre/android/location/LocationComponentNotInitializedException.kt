package org.maplibre.android.location

internal class LocationComponentNotInitializedException :
    RuntimeException(
        "The LocationComponent has to be activated with one of the LocationComponent#activateLocationComponent" +
            " overloads before any other methods are invoked.",
    )
