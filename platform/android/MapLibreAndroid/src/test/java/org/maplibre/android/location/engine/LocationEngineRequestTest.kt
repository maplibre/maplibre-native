package org.maplibre.android.location.engine

import org.assertj.core.api.Assertions.*
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.mockito.junit.MockitoJUnitRunner

@RunWith(MockitoJUnitRunner::class)
class LocationEngineRequestTest : BaseTest() {
    @Test
    fun checkDefaultValues() {
        val request = LocationEngineRequest.Builder(1000L).build()
        assertThat(request.interval).isEqualTo(1000L)
        assertThat(request.displacement).isEqualTo(0.0f)
        assertThat(request.maxWaitTime).isEqualTo(0L)
        assertThat(request.fastestInterval).isEqualTo(0L)
    }

    @Test
    fun checkBuilder() {
        val request = LocationEngineRequest.Builder(2000L)
                .setDisplacement(100.0f)
                .setMaxWaitTime(5000L)
                .setFastestInterval(500L)
                .build()
        assertThat(request.interval).isEqualTo(2000L)
        assertThat(request.displacement).isEqualTo(100.0f)
        assertThat(request.maxWaitTime).isEqualTo(5000L)
        assertThat(request.fastestInterval).isEqualTo(500L)
    }

    @Test
    fun checkRequestEqual() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L).build()
        assertThat(request).isEqualTo(otherRequest)
        assertThat(request.hashCode()).isEqualTo(otherRequest.hashCode())
    }

    @Test
    fun checkRequestsNotEqual() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L)
                .setFastestInterval(500L)
                .build()
        assertThat(request).isNotEqualTo(otherRequest)
    }

    @Test
    fun checkRequestsEqualHashCode() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L).build()
        assertThat(request.hashCode()).isEqualTo(otherRequest.hashCode())
    }

    @Test
    fun checkRequestsNonEqualHashCode() {
        val request = LocationEngineRequest.Builder(2000L).build()
        val otherRequest = LocationEngineRequest.Builder(2000L)
                .setFastestInterval(500L)
                .build()
        assertThat(request.hashCode()).isNotEqualTo(otherRequest.hashCode())
    }
}
