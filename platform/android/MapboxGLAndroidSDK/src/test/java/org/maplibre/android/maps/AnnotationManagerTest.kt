package org.maplibre.android.maps

import androidx.collection.LongSparseArray
import org.maplibre.android.annotations.Annotation
import org.maplibre.android.annotations.BaseMarkerOptions
import org.maplibre.android.annotations.Marker
import org.maplibre.android.annotations.MarkerOptions
import org.maplibre.android.geometry.LatLng
import org.junit.Assert
import org.junit.Test
import org.mockito.ArgumentMatchers
import org.mockito.Mockito

class AnnotationManagerTest {
    @Test
    @Throws(Exception::class)
    fun checksAddAMarker() {
        val aNativeMapView: NativeMap = Mockito.mock(NativeMapView::class.java)
        val aMapView = Mockito.mock(MapView::class.java)
        val annotationsArray = LongSparseArray<Annotation>()
        val aIconManager = Mockito.mock(
            IconManager::class.java
        )
        val annotations: Annotations = AnnotationContainer(aNativeMapView, annotationsArray)
        val markers: Markers = MarkerContainer(aNativeMapView, annotationsArray, aIconManager)
        val polygons: Polygons = PolygonContainer(aNativeMapView, annotationsArray)
        val polylines: Polylines = PolylineContainer(aNativeMapView, annotationsArray)
        val shapeAnnotations: ShapeAnnotations =
            ShapeAnnotationContainer(aNativeMapView, annotationsArray)
        val annotationManager = AnnotationManager(
            aMapView,
            annotationsArray,
            aIconManager,
            annotations,
            markers,
            polygons,
            polylines,
            shapeAnnotations
        )
        val aMarker = Mockito.mock(
            Marker::class.java
        )
        val aId = 5L
        Mockito.`when`(aNativeMapView.addMarker(aMarker)).thenReturn(aId)
        val aMarkerOptions = Mockito.mock(
            BaseMarkerOptions::class.java
        )
        val aMapLibreMap = Mockito.mock(MapLibreMap::class.java)
        Mockito.`when`(aMarkerOptions.marker).thenReturn(aMarker)
        annotationManager.addMarker(aMarkerOptions, aMapLibreMap)
        Assert.assertEquals(aMarker, annotationManager.annotations[0])
        Assert.assertEquals(aMarker, annotationManager.getAnnotation(aId))
    }

    @Test
    @Throws(Exception::class)
    fun checksAddMarkers() {
        val aNativeMapView = Mockito.mock(NativeMapView::class.java)
        val aMapView = Mockito.mock(MapView::class.java)
        val annotationsArray = LongSparseArray<Annotation>()
        val aIconManager = Mockito.mock(
            IconManager::class.java
        )
        val annotations: Annotations = AnnotationContainer(aNativeMapView, annotationsArray)
        val markers: Markers = MarkerContainer(aNativeMapView, annotationsArray, aIconManager)
        val polygons: Polygons = PolygonContainer(aNativeMapView, annotationsArray)
        val polylines: Polylines = PolylineContainer(aNativeMapView, annotationsArray)
        val shapeAnnotations: ShapeAnnotations =
            ShapeAnnotationContainer(aNativeMapView, annotationsArray)
        val annotationManager = AnnotationManager(
            aMapView,
            annotationsArray,
            aIconManager,
            annotations,
            markers,
            polygons,
            polylines,
            shapeAnnotations
        )
        val firstId = 1L
        val secondId = 2L
        val markerList: MutableList<BaseMarkerOptions<*, *>> = ArrayList()
        val firstMarkerOption = MarkerOptions().position(LatLng()).title("first")
        val secondMarkerOption = MarkerOptions().position(LatLng()).title("second")
        markerList.add(firstMarkerOption)
        markerList.add(secondMarkerOption)
        val aMapLibreMap = Mockito.mock(MapLibreMap::class.java)
        Mockito.`when`(
            aNativeMapView.addMarker(
                ArgumentMatchers.any(
                    Marker::class.java
                )
            )
        ).thenReturn(firstId, secondId)
        Mockito.`when`(aNativeMapView.addMarkers(ArgumentMatchers.anyList()))
            .thenReturn(longArrayOf(firstId, secondId))
        annotationManager.addMarkers(markerList, aMapLibreMap)
        Assert.assertEquals(2, annotationManager.annotations.size)
        Assert.assertEquals("first", (annotationManager.annotations[0] as Marker).title)
        Assert.assertEquals("second", (annotationManager.annotations[1] as Marker).title)
        Assert.assertEquals("first", (annotationManager.getAnnotation(firstId) as Marker).title)
        Assert.assertEquals("second", (annotationManager.getAnnotation(secondId) as Marker).title)
    }
}
