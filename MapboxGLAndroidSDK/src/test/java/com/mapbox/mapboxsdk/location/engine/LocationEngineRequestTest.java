package com.mapbox.mapboxsdk.location.engine;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;

import static org.assertj.core.api.Assertions.assertThat;

@RunWith(MockitoJUnitRunner.class)
public class LocationEngineRequestTest {

  @Test
  public void checkDefaultValues() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(1000L).build();

    assertThat(request.getInterval()).isEqualTo(1000L);
    assertThat(request.getDisplacement()).isEqualTo(0.0f);
    assertThat(request.getMaxWaitTime()).isEqualTo(0L);
    assertThat(request.getFastestInterval()).isEqualTo(0L);
  }

  @Test
  public void checkBuilder() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(2000L)
            .setDisplacement(100.0f)
            .setMaxWaitTime(5000L)
            .setFastestInterval(500L)
            .build();

    assertThat(request.getInterval()).isEqualTo(2000L);
    assertThat(request.getDisplacement()).isEqualTo(100.0f);
    assertThat(request.getMaxWaitTime()).isEqualTo(5000L);
    assertThat(request.getFastestInterval()).isEqualTo(500L);
  }

  @Test
  public void checkRequestEqual() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(2000L).build();
    LocationEngineRequest otherRequest = new LocationEngineRequest.Builder(2000L).build();

    assertThat(request).isEqualTo(otherRequest);
    assertThat(request.hashCode()).isEqualTo(otherRequest.hashCode());
  }

  @Test
  public void checkRequestsNotEqual() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(2000L).build();
    LocationEngineRequest otherRequest = new LocationEngineRequest.Builder(2000L)
            .setFastestInterval(500L)
            .build();

    assertThat(request).isNotEqualTo(otherRequest);
  }

  @Test
  public void checkRequestsEqualHashCode() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(2000L).build();
    LocationEngineRequest otherRequest = new LocationEngineRequest.Builder(2000L).build();

    assertThat(request.hashCode()).isEqualTo(otherRequest.hashCode());
  }

  @Test
  public void checkRequestsNonEqualHashCode() {
    LocationEngineRequest request = new LocationEngineRequest.Builder(2000L).build();
    LocationEngineRequest otherRequest = new LocationEngineRequest.Builder(2000L)
            .setFastestInterval(500L)
            .build();

    assertThat(request.hashCode()).isNotEqualTo(otherRequest.hashCode());
  }
}
