package com.mapbox.mapboxsdk;

import android.text.format.DateUtils;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

@RunWith(RobolectricTestRunner.class)
public class AccountsManagerTest {
  @Test
  public void testIsExpired() {
    long now = AccountsManager.getNow();

    long defaultValue = 0L;
    long tooOld = now - DateUtils.HOUR_IN_MILLIS - 1;
    long futureValue = now + 1;
    long immediatePast = now - 1;

    assertTrue(AccountsManager.isExpired(now, defaultValue));
    assertTrue(AccountsManager.isExpired(now, tooOld));

    assertFalse(AccountsManager.isExpired(now, futureValue));
    assertFalse(AccountsManager.isExpired(now, immediatePast));
  }
}
