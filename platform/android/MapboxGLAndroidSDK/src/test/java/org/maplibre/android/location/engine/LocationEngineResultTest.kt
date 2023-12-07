package org.maplibre.android.location.engine

import android.content.Intent
import android.location.Location
import android.location.LocationManager
import android.os.Bundle
import org.assertj.core.api.Assertions
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mockito
import org.mockito.junit.MockitoJUnitRunner
import java.util.Collections

@RunWith(MockitoJUnitRunner::class)
class LocationEngineResultTest {
    @Test
    fun checkNullIntent() {
        val result = LocationEngineResult.extractResult(null)
        Assertions.assertThat(result).isNull()
    }

    @Test
    fun passInvalidIntent() {
        val intent = Mockito.mock(Intent::class.java)
        val result = LocationEngineResult.extractResult(intent)
        Assertions.assertThat(result).isNull()
    }

    @Test
    fun passValidIntent() {
        val location = Mockito.mock(Location::class.java)
        val result = LocationEngineResult.extractResult(getValidIntent(location))
        Assertions.assertThat(result).isNotNull()
        Assertions.assertThat(result!!.lastLocation).isSameAs(location)
    }

    @Test
    fun passNullLocation() {
        val location: Location? = null
        val result = LocationEngineResult.create(location)
        assertForNullInput(result)
    }

    @Test
    fun passNullLocationList() {
        val locations: List<Location>? = null
        val result = LocationEngineResult.create(locations)
        assertForNullInput(result)
    }

    @Test
    fun passValidLocation() {
        val location = validLocation
        val result = LocationEngineResult.create(location)
        assertForValidInput(result)
    }

    @Test
    fun passValidLocationList() {
        val locations = listOf(validLocation)
        val result = LocationEngineResult.create(locations)
        assertForValidInput(result)
    }

    @Test
    fun passMutableLocationListWithNulls() {
        val locations = locationsWithNulls
        val result = LocationEngineResult.create(locations)
        assertForValidInput(result)
    }

    @Test
    fun passImmutableLocationListWithNulls() {
        val locations = Collections.unmodifiableList(locationsWithNulls)
        val result = LocationEngineResult.create(locations)
        assertForValidInput(result)
    }

    @Test
    fun passImmutableListWithNullLocation() {
        val locations = listOf<Location?>(null)
        val result = LocationEngineResult.create(locations)
        Assertions.assertThat(result != null)
        Assertions.assertThat(result.locations.size == 0)
    }

    companion object {
        private const val TEST_LAT_LNG = 1.0
        // J2K: IDE suggestion to use ArrayList<Location?>
        private val locationsWithNulls: ArrayList<Location?>
            private get() = object : ArrayList<Location?>() {
                init {
                    add(null)
                    add(validLocation)
                    add(null)
                }
            }
        private val validLocation: Location
            private get() {
                val location = Mockito.mock(Location::class.java)
                Mockito.`when`(location.latitude).thenReturn(TEST_LAT_LNG)
                Mockito.`when`(location.longitude).thenReturn(TEST_LAT_LNG)
                return location
            }

        private fun assertForNullInput(result: LocationEngineResult) {
            Assertions.assertThat(result).isNotNull()
            Assertions.assertThat(result.locations).isEmpty()
        }

        private fun assertForValidInput(result: LocationEngineResult) {
            Assertions.assertThat(result.locations).isNotNull()
            Assertions.assertThat(result.locations.size).isEqualTo(1)
            Assertions.assertThat(result.locations[0].latitude).isEqualTo(TEST_LAT_LNG)
            Assertions.assertThat(result.locations[0].longitude).isEqualTo(TEST_LAT_LNG)
        }

        private fun getValidIntent(location: Location): Intent {
            val intent = Mockito.mock(Intent::class.java)
            Mockito.`when`(intent.hasExtra(LocationManager.KEY_LOCATION_CHANGED)).thenReturn(true)
            val bundle = Mockito.mock(Bundle::class.java)
            Mockito.`when`<Any?>(bundle.getParcelable(LocationManager.KEY_LOCATION_CHANGED)).thenReturn(location)
            Mockito.`when`(intent.extras).thenReturn(bundle)
            return intent
        }
    }
}
