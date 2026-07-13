package org.maplibre.android.annotations

import org.maplibre.android.maps.MapLibreMap
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.InjectMocks
import org.mockito.Mockito

class AnnotationTest : BaseTest() {
    @InjectMocks
    private val maplibreMap = Mockito.mock(MapLibreMap::class.java)
    private var annotation: Annotation? = null
    private val compare: Annotation = object : Annotation() {
        override fun getId(): Long {
            return 1
        }
    }

    @Before
    fun beforeTest() {
        annotation = object : Annotation() { // empty child
        }
    }

    @Test
    fun testSanity() {
        Assert.assertNotNull("markerOptions should not be null", annotation)
    }

    @Test
    fun testRemove() {
        annotation!!.id = 1
        annotation!!.setMapLibreMap(maplibreMap)
        annotation!!.remove()
        Mockito.verify(maplibreMap, Mockito.times(1)).removeAnnotation(
            annotation!!
        )
    }

    @Test
    fun testRemoveUnboundMapLibreMap() {
        annotation!!.id = 1
        annotation!!.remove()
        Mockito.verify(maplibreMap, Mockito.times(0)).removeAnnotation(
            annotation!!
        )
    }

    @Test
    fun testCompareToEqual() {
        annotation!!.id = 1
        Assert.assertEquals("conparable equal", 0, annotation!!.compareTo(compare).toLong())
    }

    @Test
    fun testCompareToHigher() {
        annotation!!.id = 3
        Assert.assertEquals("conparable higher", -1, annotation!!.compareTo(compare).toLong())
    }

    @Test
    fun testCompareTolower() {
        annotation!!.id = 0
        Assert.assertEquals("conparable lower", 1, annotation!!.compareTo(compare).toLong())
    }

    @Test
    fun testEquals() {
        var holder: Annotation? = null
        Assert.assertFalse(annotation == holder)
        holder = annotation
        Assert.assertTrue(annotation == holder)
        Assert.assertFalse(annotation == Any())
    }

    @Test
    fun testHashcode() {
        val id = 1
        annotation!!.id = id.toLong()
        Assert.assertSame("hashcode should match", annotation.hashCode(), id)
    }
}
