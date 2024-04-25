package org.maplibre.android.testapp.offline

import androidx.test.internal.runner.junit4.AndroidJUnit4ClassRunner
import org.maplibre.android.AppCenter
import org.maplibre.android.testapp.activity.offline.OfflineActivity
import org.maplibre.android.testapp.utils.OfflineUtils.convertRegionName
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import java.io.UnsupportedEncodingException
import java.util.*

@RunWith(AndroidJUnit4ClassRunner::class)
class OfflineUtilsTest : AppCenter() {
    @Test
    @Throws(UnsupportedEncodingException::class)
    fun testOfflineUtilsConvertToBytes() {
        val expected = CONVERTED_REGION_NAME.toByteArray(OfflineActivity.JSON_CHARSET)
        val actual = convertRegionName(REGION_NAME)
        Assert.assertTrue("Bytes arrays should match", Arrays.equals(expected, actual))
    }

    @Test
    @Throws(UnsupportedEncodingException::class)
    fun testOfflineUtilsConvertToString() {
        val actual =
            convertRegionName(CONVERTED_REGION_NAME.toByteArray(OfflineActivity.JSON_CHARSET))
        Assert.assertEquals("Strings should match", REGION_NAME, actual)
    }

    @Test
    fun testOfflineUtilsConvertNoOp() {
        val convertNoOp = convertRegionName(convertRegionName(REGION_NAME)!!)
        Assert.assertEquals("Strings should match", REGION_NAME, convertNoOp)
    }

    companion object {
        private const val REGION_NAME = "hello world"
        private const val CONVERTED_REGION_NAME = "{\"FIELD_REGION_NAME\":\"hello world\"}"
    }
}
