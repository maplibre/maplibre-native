package org.maplibre.android.snapshotter.buildings

import org.junit.Assert.assertEquals
import org.junit.Test

class BuildingExtrusionSnapshotConstantsTest {

    @Test
    fun defaultIds_documentedForHostApps() {
        assertEquals("maplibre-snapshot-buildings", BuildingExtrusionSnapshot.DEFAULT_SOURCE_ID)
        assertEquals("maplibre-snapshot-buildings-extrusion", BuildingExtrusionSnapshot.DEFAULT_LAYER_ID)
    }
}
