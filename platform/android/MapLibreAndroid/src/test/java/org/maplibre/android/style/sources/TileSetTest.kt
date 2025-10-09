package org.maplibre.android.style.sources

import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.geometry.LatLngBounds
import org.robolectric.RobolectricTestRunner

/**
 * Test for TileSet class to validate bounds setting functionality
 */
@RunWith(RobolectricTestRunner::class)
class TileSetTest {

    @Test
    fun testSetBoundsWithFourFloats() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        
        // Test setting bounds with four float parameters
        tileSet.setBounds(-180f, -90f, 180f, 90f)
        
        assertNotNull("Bounds should be set", tileSet.bounds)
        assertEquals("Bounds should have 4 elements", 4, tileSet.bounds?.size)
        assertEquals("Left bound should be -180", -180f, tileSet.bounds?.get(0))
        assertEquals("Bottom bound should be -90", -90f, tileSet.bounds?.get(1))
        assertEquals("Right bound should be 180", 180f, tileSet.bounds?.get(2))
        assertEquals("Top bound should be 90", 90f, tileSet.bounds?.get(3))
    }

    @Test
    fun testSetBoundsWithVarargFloats() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        
        // Test setting bounds with vararg floats
        tileSet.setBounds(-10f, -20f, 30f, 40f)
        
        assertNotNull("Bounds should be set", tileSet.bounds)
        assertEquals("Bounds should have 4 elements", 4, tileSet.bounds?.size)
        assertEquals("Left bound should be -10", -10f, tileSet.bounds?.get(0))
        assertEquals("Bottom bound should be -20", -20f, tileSet.bounds?.get(1))
        assertEquals("Right bound should be 30", 30f, tileSet.bounds?.get(2))
        assertEquals("Top bound should be 40", 40f, tileSet.bounds?.get(3))
    }

    @Test
    fun testSetBoundsWithArray() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        
        // Test setting bounds with array
        val boundsArray = arrayOf(12f, 34f, 56f, 78f)
        tileSet.setBounds(boundsArray)
        
        assertNotNull("Bounds should be set", tileSet.bounds)
        assertEquals("Bounds should have 4 elements", 4, tileSet.bounds?.size)
        assertArrayEquals("Bounds should match input array", boundsArray, tileSet.bounds)
    }

    @Test
    fun testSetBoundsWithLatLngBounds() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        
        // Test setting bounds with LatLngBounds
        val latLngBounds = LatLngBounds.Builder()
            .include(org.maplibre.android.geometry.LatLng(12.0, 34.0))
            .include(org.maplibre.android.geometry.LatLng(56.0, 78.0))
            .build()
        
        tileSet.setBounds(latLngBounds)
        
        assertNotNull("Bounds should be set", tileSet.bounds)
        assertEquals("Bounds should have 4 elements", 4, tileSet.bounds?.size)
        assertEquals("Left bound should match", latLngBounds.longitudeWest.toFloat(), tileSet.bounds?.get(0))
        assertEquals("Bottom bound should match", latLngBounds.latitudeSouth.toFloat(), tileSet.bounds?.get(1))
        assertEquals("Right bound should match", latLngBounds.longitudeEast.toFloat(), tileSet.bounds?.get(2))
        assertEquals("Top bound should match", latLngBounds.latitudeNorth.toFloat(), tileSet.bounds?.get(3))
    }

    @Test
    fun testSetBoundsNoStackOverflow() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        
        // This test verifies the fix for the infinite recursion bug
        // If the bug still exists, this will cause a StackOverflowError
        tileSet.setBounds(1f, 2f, 3f, 4f)
        
        assertNotNull("Bounds should be set without stack overflow", tileSet.bounds)
        assertEquals("Should have 4 elements", 4, tileSet.bounds?.size)
    }

    @Test
    fun testMultipleSetBoundsCalls() {
        val tileSet = TileSet("2.1.0", "https://example.com/{z}/{x}/{y}.png")
        
        // Test multiple calls to setBounds
        tileSet.setBounds(-180f, -90f, 180f, 90f)
        assertNotNull("First bounds should be set", tileSet.bounds)
        
        tileSet.setBounds(-10f, -20f, 30f, 40f)
        assertNotNull("Second bounds should be set", tileSet.bounds)
        assertEquals("Left bound should be updated", -10f, tileSet.bounds?.get(0))
        assertEquals("Bottom bound should be updated", -20f, tileSet.bounds?.get(1))
        assertEquals("Right bound should be updated", 30f, tileSet.bounds?.get(2))
        assertEquals("Top bound should be updated", 40f, tileSet.bounds?.get(3))
    }
}
