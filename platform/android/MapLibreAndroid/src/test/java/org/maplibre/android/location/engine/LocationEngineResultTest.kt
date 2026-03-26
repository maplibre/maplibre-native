package org.maplibre.android.location.engine

import android.content.Intent
import android.location.Location
import android.location.LocationManager
import android.os.Bundle
import org.assertj.core.api.Assertions.*
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.Mockito.*
import org.mockito.junit.MockitoJUnitRunner
import java.util.Collections

@RunWith(MockitoJUnitRunner::class)
class LocationEngineResultTest : BaseTest(){
    @Test
    fun checkNullIntent() {
        val result = LocationEngineResult.extractResult(null)
        assertThat(result).isNull()
    }

    @Test
    fun passInvalidIntent() {
        val intent = mock(Intent::class.java)
        val result = LocationEngineResult.extractResult(intent)
        assertThat(result).isNull()
    }

    @Test
    fun passValidIntent() {
        val location = mock(Location::class.java)
        val result = LocationEngineResult.extractResult(getValidIntent(location))
        assertThat(result).isNotNull()
        assertThat(result!!.lastLocation).isSameAs(location)
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
        assertThat(result != null)
        assertThat(result.locations.size == 0)
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
                val location = mock(Location::class.java)
                `when`(location.latitude).thenReturn(TEST_LAT_LNG)
                `when`(location.longitude).thenReturn(TEST_LAT_LNG)
                return location
            }

        private fun assertForNullInput(result: LocationEngineResult) {
            assertThat(result).isNotNull()
            assertThat(result.locations).isEmpty()
        }

        private fun assertForValidInput(result: LocationEngineResult) {
            assertThat(result.locations).isNotNull()
            assertThat(result.locations.size).isEqualTo(1)
            assertThat(result.locations[0].latitude).isEqualTo(TEST_LAT_LNG)
            assertThat(result.locations[0].longitude).isEqualTo(TEST_LAT_LNG)
        }

        private fun getValidIntent(location: Location): Intent {
            val intent = mock(Intent::class.java)
            `when`(intent.hasExtra(LocationManager.KEY_LOCATION_CHANGED)).thenReturn(true)
            val bundle = mock(Bundle::class.java)
            `when`<Any?>(bundle.getParcelable(LocationManager.KEY_LOCATION_CHANGED)).thenReturn(location)
            `when`(intent.extras).thenReturn(bundle)
            return intent
        }
    }
}
