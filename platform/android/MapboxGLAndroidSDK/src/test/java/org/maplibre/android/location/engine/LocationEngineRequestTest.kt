package org.maplibre.android.location.engine

import org.assertj.core.api.Assertions
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.junit.MockitoJUnitRunner

@RunWith(MockitoJUnitRunner::class)
class LocationEngineRequestTest {
    @Test
    fun checkDefaultValues() {
        val request = LocationEngineRequest.Builder(1000L).build()
        Assertions.assertThat(request.interval).isEqualTo(1000L)
        Assertions.assertThat(request.displacement).isEqualTo(0.0f)
        Assertions.assertThat(request.maxWaitTime).isEqualTo(0L)
        Assertions.assertThat(request.fastestInterval).isEqualTo(0L)
    }

    @Test
    fun checkBuilder() {
        val request = LocationEngineRequest.Builder(2000L)
                .setDisplacement(100.0f)
                .setMaxWaitTime(5000L)
                .setFastestInterval(500L)
                .build()
        Assertions.assertThat(request.interval).isEqualTo(2000L)
        Assertions.assertThat(request.displacement).isEqualTo(100.0f)
        Assertions.assertThat(request.maxWaitTime).isEqualTo(5000L)
        Assertions.assertThat(request.fastestInterval).isEqualTo(500L)
    }

    @Test
    fun checkRequestEqual() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L).build()
        Assertions.assertThat(request).isEqualTo(otherRequest)
        Assertions.assertThat(request.hashCode()).isEqualTo(otherRequest.hashCode())
    }

    @Test
    fun checkRequestsNotEqual() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L)
                .setFastestInterval(500L)
                .build()
        Assertions.assertThat(request).isNotEqualTo(otherRequest)
    }

    @Test
    fun checkRequestsEqualHashCode() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L).build()
        Assertions.assertThat(request.hashCode()).isEqualTo(otherRequest.hashCode())
    }

    @Test
    fun checkRequestsNonEqualHashCode() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L)
                .setFastestInterval(500L)
                .build()
        Assertions.assertThat(request.hashCode()).isNotEqualTo(otherRequest.hashCode())
    }
}
