package org.maplibre.android.style.expressions

import android.graphics.Color
import org.maplibre.geojson.Point
import org.maplibre.geojson.Polygon
import org.maplibre.android.style.expressions.Expression.FormatOption
import org.maplibre.android.style.layers.PropertyFactory
import org.maplibre.android.style.layers.PropertyValue
import org.maplibre.android.utils.ColorUtils
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith
import org.maplibre.android.BaseTest
import org.robolectric.RobolectricTestRunner
import java.util.*

/**
 * Expression unit tests that validate the expression output with the expected Object[]array representation.
 */
@RunWith(RobolectricTestRunner::class)
class ExpressionTest : BaseTest() {
    @Test
    fun testPropertyValueIsExpression() {
        val property: PropertyValue<*> = PropertyFactory.lineWidth(Expression.get("width"))
        Assert.assertTrue(property.isExpression)
    }

    @Test
    fun testPropertyValueEqualsExpression() {
        val property: PropertyValue<*> = PropertyFactory.lineWidth(Expression.get("width"))
        org.junit.Assert.assertEquals(Expression.get("width"), property.expression)
    }

    @Test
    @Throws(Exception::class)
    fun testRgb() {
        val expected = arrayOf<Any>("rgb", 0f, 0f, 0f)
        val actual =
            Expression.rgb(Expression.literal(0), Expression.literal(0), Expression.literal(0))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testRgbLiteral() {
        val expected = arrayOf<Any>("rgb", 0f, 0f, 0f)
        val actual = Expression.rgb(0, 0, 0).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testRgba() {
        val expected = arrayOf<Any>("rgba", 0f, 0f, 0f, 1f)
        val actual = Expression.rgba(
            Expression.literal(0),
            Expression.literal(0),
            Expression.literal(0),
            Expression.literal(1)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testRgbaLiteral() {
        val expected = arrayOf<Any>("rgba", 0f, 0f, 0f, 1f)
        val actual = Expression.rgba(0, 0, 0, 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testToRgba() {
        val expected = arrayOf<Any>("to-rgba", arrayOf<Any>("to-color", "rgba(255, 0, 0, 1)"))
        val actual = Expression.toRgba(
            Expression.toColor(
                Expression.literal(
                    ColorUtils.colorToRgbaString(
                        Color.RED
                    )
                )
            )
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testEq() {
        val expected = arrayOf<Any>("==", 1f, 1f)
        val actual = Expression.eq(Expression.literal(1), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testEqLiteral() {
        val expected = arrayOf<Any>("==", 1f, 1f)
        val actual = Expression.eq(Expression.literal(1), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testEqExpression() {
        val expected = arrayOf<Any>("==", arrayOf<Any>("get", "hello"), 1f)
        val actual = Expression.eq(Expression.get("hello"), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testNeq() {
        val expected = arrayOf<Any>("!=", 0f, 1f)
        val actual = Expression.neq(Expression.literal(0), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testNeqLiteral() {
        val expected = arrayOf<Any>("!=", 0f, 1f)
        val actual = Expression.neq(Expression.literal(0), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testNeqExpression() {
        val expected = arrayOf<Any>("!=", arrayOf<Any>("get", "hello"), 1f)
        val actual = Expression.neq(Expression.get("hello"), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGt() {
        val expected = arrayOf<Any>(">", 0f, 1f)
        val actual = Expression.gt(Expression.literal(0), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGtLiteral() {
        val expected = arrayOf<Any>(">", 0f, 1f)
        val actual = Expression.gt(Expression.literal(0), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGtExpression() {
        val expected = arrayOf<Any>(">", arrayOf<Any>("get", "hello"), 1f)
        val actual = Expression.gt(Expression.get("hello"), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLt() {
        val expected = arrayOf<Any>("<", 1f, 0f)
        val actual = Expression.lt(Expression.literal(1), Expression.literal(0)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLtLiteral() {
        val expected = arrayOf<Any>("<", 1f, 0f)
        val actual = Expression.lt(Expression.literal(1), 0).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLtExpression() {
        val expected = arrayOf<Any>("<", arrayOf<Any>("get", "hello"), 1f)
        val actual = Expression.lt(Expression.get("hello"), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGte() {
        val expected = arrayOf<Any>(">=", 1f, 1f)
        val actual = Expression.gte(Expression.literal(1), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGteLiteral() {
        val expected = arrayOf<Any>(">=", 1f, 1f)
        val actual = Expression.gte(Expression.literal(1), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGteExpression() {
        val expected = arrayOf<Any>(">=", arrayOf<Any>("get", "hello"), 1f)
        val actual = Expression.gte(Expression.get("hello"), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLte() {
        val expected = arrayOf<Any>("<=", 1f, 1f)
        val actual = Expression.lte(Expression.literal(1), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLteExpression() {
        val expected = arrayOf<Any>("<=", arrayOf<Any>("get", "hello"), 1f)
        val actual = Expression.lte(Expression.get("hello"), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLteLiteral() {
        val expected = arrayOf<Any>("<=", 1f, 1f)
        val actual = Expression.lte(Expression.literal(1), 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAll() {
        val expected = arrayOf<Any>("all", true, true, true)
        val actual = Expression.all(
            Expression.literal(true),
            Expression.literal(true),
            Expression.literal(true)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAny() {
        val expected = arrayOf<Any>("any", true, false, false)
        val actual = Expression.any(
            Expression.literal(true),
            Expression.literal(false),
            Expression.literal(false)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testNot() {
        val expected = arrayOf<Any>("!", false)
        val actual = Expression.not(Expression.literal(false)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testNotLiteral() {
        val expected = arrayOf<Any>("!", false)
        val actual = Expression.not(false).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSwitchCase() {
        val expectedCaseOneGet = arrayOf<Any>("get", "key1")
        val expectedCaseOne = arrayOf<Any>("==", expectedCaseOneGet, "value1")
        val expectedCaseTwoGet = arrayOf<Any>("get", "key2")
        val expectedCaseTwo = arrayOf<Any>("!=", expectedCaseTwoGet, "value2")
        val expected = arrayOf<Any>("case", expectedCaseOne, expectedCaseTwo)
        val actual = Expression.switchCase(
            Expression.eq(Expression.get(Expression.literal("key1")), Expression.literal("value1")),
            Expression.neq(Expression.get(Expression.literal("key2")), Expression.literal("value2"))
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSwitchCaseLiteral() {
        val expectedCaseOneGet = arrayOf<Any>("get", "key1")
        val expectedCaseOne = arrayOf<Any>("==", expectedCaseOneGet, "value1")
        val expectedCaseTwoGet = arrayOf<Any>("get", "key2")
        val expectedCaseTwo = arrayOf<Any>("!=", expectedCaseTwoGet, "value2")
        val expected = arrayOf<Any>("case", expectedCaseOne, expectedCaseTwo)
        val actual = Expression.switchCase(
            Expression.eq(Expression.get("key1"), Expression.literal("value1")),
            Expression.neq(Expression.get("key2"), Expression.literal("value2"))
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMatch() {
        val input = "input"
        val labels = arrayOf("a", "b", "c")
        val outputs = arrayOf("1", "2", "3")
        val defaultOutput = "0"
        val expected = arrayOf<Any>(
            "match", input,
            labels[0], outputs[0],
            labels[1], outputs[1],
            labels[2], outputs[2],
            defaultOutput
        )
        val actual = Expression.match(
            Expression.literal(input),
            Expression.literal(labels[0]),
            Expression.literal(
                outputs[0]
            ),
            Expression.literal(labels[1]),
            Expression.literal(
                outputs[1]
            ),
            Expression.literal(labels[2]),
            Expression.literal(
                outputs[2]
            ),
            Expression.literal(defaultOutput)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMatchWithStops() {
        val input = "input"
        val labels = arrayOf("a", "b", "c")
        val outputs = arrayOf("1", "2", "3")
        val defaultOutput = "0"
        val expected = arrayOf<Any>(
            "match", input,
            labels[0], outputs[0],
            labels[1], outputs[1],
            labels[2], outputs[2],
            defaultOutput
        )
        val actual = Expression.match(
            Expression.literal(input),
            Expression.literal(defaultOutput),
            Expression.stop(labels[0], outputs[0]),
            Expression.stop(labels[1], outputs[1]),
            Expression.stop(labels[2], outputs[2])
        )
            .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCoalesce() {
        val expectedGetOne = arrayOf<Any>("get", "invalidKey")
        val expectedGetTwo = arrayOf<Any>("get", "validKey")
        val expected = arrayOf<Any>("coalesce", expectedGetOne, expectedGetTwo)
        val actual = Expression.coalesce(
            Expression.get(Expression.literal("invalidKey")),
            Expression.get(Expression.literal("validKey"))
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCoalesceLiteral() {
        val expectedGetOne = arrayOf<Any>("get", "invalidKey")
        val expectedGetTwo = arrayOf<Any>("get", "validKey")
        val expected = arrayOf<Any>("coalesce", expectedGetOne, expectedGetTwo)
        val actual = Expression.coalesce(
            Expression.get("invalidKey"),
            Expression.get("validKey")
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testProperties() {
        val expected = arrayOf<Any>("properties")
        val actual = Expression.properties().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGeometryType() {
        val expected = arrayOf<Any>("geometry-type")
        val actual = Expression.geometryType().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testId() {
        val expected = arrayOf<Any>("id")
        val actual = Expression.id().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHeatmapDensity() {
        val expected = arrayOf<Any>("heatmap-density")
        val actual = Expression.heatmapDensity().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAt() {
        val expected = arrayOf<Any>("at", 3f, arrayOf<Any>("literal", arrayOf<Any>("one", "two")))
        val actual =
            Expression.at(Expression.literal(3), Expression.literal(arrayOf<Any>("one", "two")))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testInString() {
        val expected = arrayOf<Any>("in", "one", "onetwo")
        val actual =
            Expression.`in`(Expression.literal("one"), Expression.literal("onetwo")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testWithIn() {
        val lngLats = listOf(
            Arrays.asList(
                Point.fromLngLat(0.0, 0.0),
                Point.fromLngLat(0.0, 5.0),
                Point.fromLngLat(5.0, 5.0),
                Point.fromLngLat(5.0, 0.0),
                Point.fromLngLat(0.0, 0.0)
            )
        )
        val polygon = Polygon.fromLngLats(lngLats)
        val map = HashMap<String, String>()
        map["type"] = "Polygon"
        map["json"] = polygon.toJson()
        val expected = arrayOf<Any>("within", map)
        val actual = Expression.within(polygon).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testIndexOf() {
        val expected = arrayOf<Any>("index-of", 2f, arrayOf<Any>("literal", arrayOf<Any>(1f, 2f, 3f)))
        val actual =
            Expression.indexOf(Expression.literal(2f), Expression.literal(arrayOf<Any>(1f, 2f, 3f)))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }


    @Test
    @Throws(Exception::class)
    fun testIndexOfWithFromIndex() {
        val expected = arrayOf<Any>("index-of", 2f, arrayOf<Any>("literal", arrayOf<Any>(1f, 2f, 3f)), 1f)
        val actual =
            Expression.indexOf(Expression.literal(2f), Expression.literal(arrayOf<Any>(1f, 2f, 3f)), Expression.literal(1f))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSlice() {
        val expected = arrayOf<Any>("slice", arrayOf<Any>("literal", arrayOf<Any>(1f, 2f, 3f)), 1f)
        val actual =
            Expression.slice(Expression.literal(arrayOf<Any>(1f, 2f, 3f)), Expression.literal(1f))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSliceWithToIndex() {
        val expected = arrayOf<Any>("slice", arrayOf<Any>("literal", arrayOf<Any>(1f, 2f, 3f)), 1f, 3f)
        val actual =
            Expression.slice(Expression.literal(arrayOf<Any>(1f, 2f, 3f)), Expression.literal(1f), Expression.literal(3f))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testInNumber() {
        val expected = arrayOf<Any>("in", 1f, arrayOf<Any>("literal", arrayOf<Any>(1f, 2f)))
        val actual =
            Expression.`in`(Expression.literal(1f), Expression.literal(arrayOf<Any>(1f, 2f)))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testDistance() {
        val point = Point.fromLngLat(1.0, 2.0)
        val map = HashMap<String, String>()
        map["json"] = point.toJson()
        val expected = arrayOf<Any>("distance", map)
        val actual = Expression.distance(point).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testInArray() {
        val expected =
            arrayOf<Any>("in", "one", arrayOf<Any>("literal", arrayOf<Any>("one", "two")))
        val actual = Expression.`in`(
            Expression.literal("one"),
            Expression.literal(arrayOf<Any>("one", "two"))
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAtLiteral() {
        val expected = arrayOf<Any>("at", 3f, arrayOf<Any>("literal", arrayOf<Any>("one", "two")))
        val actual = Expression.at(3, Expression.literal(arrayOf<Any>("one", "two"))).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAtExpression() {
        val expected = arrayOf<Any>("at", 3f, arrayOf<Any>("properties"))
        val actual = Expression.at(Expression.literal(3), Expression.properties()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGet() {
        val expected = arrayOf<Any>("get", "key")
        val actual = Expression.get(Expression.literal("key")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGetLiteral() {
        val expected = arrayOf<Any>("get", "key")
        val actual = Expression.get("key").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGetObject() {
        val expected = arrayOf<Any>("get", "key", arrayOf<Any>("properties"))
        val actual = Expression.get(Expression.literal("key"), Expression.properties()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testGetObjectLiteral() {
        val expected = arrayOf<Any>("get", "key", arrayOf<Any>("properties"))
        val actual = Expression.get("key", Expression.properties()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHas() {
        val expected = arrayOf<Any>("has", "key")
        val actual = Expression.has(Expression.literal("key")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHasLiteral() {
        val expected = arrayOf<Any>("has", "key")
        val actual = Expression.has("key").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHasObject() {
        val expected = arrayOf<Any>("has", "key", arrayOf<Any>("properties"))
        val actual = Expression.has(Expression.literal("key"), Expression.properties()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHasObjectLiteral() {
        val expected = arrayOf<Any>("has", "key", arrayOf<Any>("properties"))
        val actual = Expression.has("key", Expression.properties()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHasExpression() {
        val expected = arrayOf<Any>("has", arrayOf<Any>("get", "key"), arrayOf<Any>("properties"))
        val actual =
            Expression.has(Expression.get(Expression.literal("key")), Expression.properties())
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testHasExpressionLiteral() {
        val expected = arrayOf<Any>("has", arrayOf<Any>("get", "key"), arrayOf<Any>("properties"))
        val actual = Expression.has(Expression.get("key"), Expression.properties()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLength() {
        val expected = arrayOf<Any>("length", "key")
        val actual = Expression.length(Expression.literal("key")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLengthLiteral() {
        val expected = arrayOf<Any>("length", "key")
        val actual = Expression.length("key").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLengthExpression() {
        val expected = arrayOf<Any>("length", arrayOf<Any>("get", "key"))
        val actual = Expression.length(Expression.get(Expression.literal("key"))).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLn2() {
        val expected = arrayOf<Any>("ln2")
        val actual = Expression.ln2().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testPi() {
        val expected = arrayOf<Any>("pi")
        val actual = Expression.pi().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testE() {
        val expected = arrayOf<Any>("e")
        val actual = Expression.e().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSum() {
        val expected = arrayOf<Any>("+", 1f, 2f)
        val actual = Expression.sum(Expression.literal(1), Expression.literal(2)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSumLiteral() {
        val expected = arrayOf<Any>("+", 1f, 2f)
        val actual = Expression.sum(1, 2).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testProduct() {
        val expected = arrayOf<Any>("*", 1f, 2f)
        val actual = Expression.product(Expression.literal(1), Expression.literal(2)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testProductLiteral() {
        val expected = arrayOf<Any>("*", 1f, 2f)
        val actual = Expression.product(1, 2).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSubtract() {
        val expected = arrayOf<Any>("-", 2f, 1f)
        val actual = Expression.subtract(Expression.literal(2), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSubtractLiteral() {
        val expected = arrayOf<Any>("-", 2f, 1f)
        val actual = Expression.subtract(2, 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testDivision() {
        val expected = arrayOf<Any>("/", 2f, 1f)
        val actual = Expression.division(Expression.literal(2), Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testDivisionLiteral() {
        val expected = arrayOf<Any>("/", 2f, 1f)
        val actual = Expression.division(2, 1).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testDivisionWithNestedGet() {
        val nestedGet: Any = arrayOf<Any>("get", "key")
        val expected = arrayOf("/", 2f, nestedGet)
        val actual =
            Expression.division(Expression.literal(2), Expression.get(Expression.literal("key")))
                .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMod() {
        val expected = arrayOf<Any>("%", 1f, 3f)
        val actual = Expression.mod(Expression.literal(1), Expression.literal(3)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testModLiteral() {
        val expected = arrayOf<Any>("%", 1f, 3f)
        val actual = Expression.mod(1, 3).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testPow() {
        val expected = arrayOf<Any>("^", 2f, 3f)
        val actual = Expression.pow(Expression.literal(2), Expression.literal(3)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testPowLiteral() {
        val expected = arrayOf<Any>("^", 2f, 3f)
        val actual = Expression.pow(2, 3).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSqrt() {
        val expected = arrayOf<Any>("sqrt", 4f)
        val actual = Expression.sqrt(Expression.literal(4)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSqrtLiteral() {
        val expected = arrayOf<Any>("sqrt", 4f)
        val actual = Expression.sqrt(4).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLog10() {
        val expected = arrayOf<Any>("log10", 10f)
        val actual = Expression.log10(Expression.literal(10f)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLog10Literal() {
        val expected = arrayOf<Any>("log10", 10f)
        val actual = Expression.log10(10).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLn() {
        val expected = arrayOf<Any>("ln", 2f)
        val actual = Expression.ln(Expression.literal(2)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLnLiteral() {
        val expected = arrayOf<Any>("ln", 2f)
        val actual = Expression.ln(2).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLog2() {
        val expected = arrayOf<Any>("log2", 16f)
        val actual = Expression.log2(Expression.literal(16)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLog2Literal() {
        val expected = arrayOf<Any>("log2", 16f)
        val actual = Expression.log2(16).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSin() {
        val expected = arrayOf<Any>("sin", 45f)
        val actual = Expression.sin(Expression.literal(45)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testSinLiteral() {
        val expected = arrayOf<Any>("sin", 45f)
        val actual = Expression.sin(45).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCos() {
        val expected = arrayOf<Any>("cos", 45f)
        val actual = Expression.cos(Expression.literal(45)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCosLiteral() {
        val expected = arrayOf<Any>("cos", 45f)
        val actual = Expression.cos(45).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testTan() {
        val expected = arrayOf<Any>("tan", 45f)
        val actual = Expression.tan(Expression.literal(45)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testTanLiteral() {
        val expected = arrayOf<Any>("tan", 45f)
        val actual = Expression.tan(45).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAsin() {
        val expected = arrayOf<Any>("asin", 45f)
        val actual = Expression.asin(Expression.literal(45)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAsinLiteral() {
        val expected = arrayOf<Any>("asin", 45f)
        val actual = Expression.asin(45).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAcos() {
        val expected = arrayOf<Any>("acos", 45f)
        val actual = Expression.acos(Expression.literal(45)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAcosLiteral() {
        val expected = arrayOf<Any>("acos", 45f)
        val actual = Expression.acos(45).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAtan() {
        val expected = arrayOf<Any>("atan", 45f)
        val actual = Expression.atan(Expression.literal(45)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testAtanLiteral() {
        val expected = arrayOf<Any>("atan", 45f)
        val actual = Expression.atan(45).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMin() {
        val expected = arrayOf<Any>("min", 0f, 1f, 2f, 3f)
        val actual = Expression.min(
            Expression.literal(0),
            Expression.literal(1),
            Expression.literal(2),
            Expression.literal(3)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMinLiteral() {
        val expected = arrayOf<Any>("min", 0f, 1f, 2f, 3f)
        val actual = Expression.min(0, 1, 2, 3).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMax() {
        val expected = arrayOf<Any>("max", 0f, 1f, 2f, 3f)
        val actual = Expression.max(
            Expression.literal(0),
            Expression.literal(1),
            Expression.literal(2),
            Expression.literal(3)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testMaxLiteral() {
        val expected = arrayOf<Any>("max", 0f, 1f, 2f, 3f)
        val actual = Expression.max(0, 1, 2, 3).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testUpcase() {
        val expected = arrayOf<Any>("upcase", "string")
        val actual = Expression.upcase(Expression.literal("string")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testUpcaseLiteral() {
        val expected = arrayOf<Any>("upcase", "string")
        val actual = Expression.upcase("string").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testDowncase() {
        val expected = arrayOf<Any>("downcase", "string")
        val actual = Expression.downcase(Expression.literal("string")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testDowncaseLiteral() {
        val expected = arrayOf<Any>("downcase", "string")
        val actual = Expression.downcase("string").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testConcat() {
        val expected = arrayOf<Any>("concat", "foo", "bar")
        val actual =
            Expression.concat(Expression.literal("foo"), Expression.literal("bar")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testConcatLiteral() {
        val expected = arrayOf<Any>("concat", "foo", "bar")
        val actual = Expression.concat("foo", "bar").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testArray() {
        val get = arrayOf<Any>("get", "keyToArray")
        val expected = arrayOf<Any>("array", get)
        val actual = Expression.array(Expression.get(Expression.literal("keyToArray"))).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testArrayLiteral() {
        val get = arrayOf<Any>("get", "keyToArray")
        val expected = arrayOf<Any>("array", get)
        val actual = Expression.array(Expression.get("keyToArray")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testTypeOf() {
        val expected = arrayOf<Any>("typeof", "value")
        val actual = Expression.typeOf(Expression.literal("value")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testString() {
        val expected = arrayOf<Any>("string", "value")
        val actual = Expression.string(Expression.literal("value")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testNumber() {
        val expected = arrayOf<Any>("number", 1f)
        val actual = Expression.number(Expression.literal(1)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testBool() {
        val expected = arrayOf<Any>("boolean", true)
        val actual = Expression.bool(Expression.literal(true)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testObject() {
        val `object` = Any()
        val expected = arrayOf("object", `object`)
        val actual = Expression.`object`(Expression.literal(`object`)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testToString() {
        val expected = arrayOf<Any>("to-string", 3f)
        val actual = Expression.toString(Expression.literal(3)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testToNumber() {
        val expected = arrayOf<Any>("to-number", "3")
        val actual = Expression.toNumber(Expression.literal("3")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testToBool() {
        val expected = arrayOf<Any>("to-boolean", "true")
        val actual = Expression.toBool(Expression.literal("true")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testToColor() {
        val expected = arrayOf<Any>("to-color", "value")
        val actual = Expression.toColor(Expression.literal("value")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testToPadding() {
        val expected = arrayOf<Any>("to-padding", "value")
        val actual = Expression.toPadding(Expression.literal("value")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLet() {
        val expected = arrayOf<Any>("let", "letName", "value")
        val actual =
            Expression.let(Expression.literal("letName"), Expression.literal("value")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testVar() {
        val expected = arrayOf<Any>("var", "letName")
        val actual = Expression.`var`(Expression.literal("letName")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testVarLiteral() {
        val expected = arrayOf<Any>("var", "letName")
        val actual = Expression.`var`("letName").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testVarExpression() {
        val expected = arrayOf<Any>("var", arrayOf<Any>("get", "letName"))
        val actual = Expression.`var`(Expression.get(Expression.literal("letName"))).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testVarExpressionLiteral() {
        val expected = arrayOf<Any>("var", arrayOf<Any>("get", "letName"))
        val actual = Expression.`var`(Expression.get("letName")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testZoom() {
        val expected = arrayOf<Any>("zoom")
        val actual = Expression.zoom().toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testStepBasic() {
        val expected = arrayOf<Any>("step", 12f, 11f, 0f, 111f, 1f, 1111f)
        val actual = Expression.step(
            Expression.literal(12),
            Expression.literal(11),
            Expression.literal(0),
            Expression.literal(111),
            Expression.literal(1),
            Expression.literal(1111)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testStepBasicLiteral() {
        val expected =
            arrayOf<Any>("step", arrayOf<Any>("get", "line-width"), 11f, 0f, 111f, 1f, 1111f)
        val actual = Expression.step(
            Expression.get("line-width"),
            Expression.literal(11),
            Expression.stop(0, 111),
            Expression.stop(1, 1111)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testStepExpression() {
        val input = arrayOf<Any>("get", "key")
        val number = arrayOf<Any>("to-number", input)
        val expected = arrayOf<Any>("step", number, 11f, 0f, 111f, 1f, 1111f)
        val actual = Expression.step(
            Expression.toNumber(Expression.get(Expression.literal("key"))),
            Expression.literal(11),
            Expression.literal(0),
            Expression.literal(111),
            Expression.literal(1),
            Expression.literal(1111)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testStepExpressionLiteral() {
        val input = arrayOf<Any>("get", "key")
        val number = arrayOf<Any>("to-number", input)
        val expected = arrayOf<Any>("step", number, 11f, 0f, 111f, 1f, 1111f)
        val actual = Expression.step(
            Expression.toNumber(Expression.get("key")),
            Expression.literal(11),
            Expression.stop(0, 111),
            Expression.stop(1, 1111)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLinear() {
        val expected =
            arrayOf<Any>("interpolate", arrayOf<Any>("linear"), 12f, 0f, 1f, 1f, 2f, 2f, 3f)
        val actual = Expression.interpolate(
            Expression.linear(),
            Expression.literal(12),
            Expression.literal(0),
            Expression.literal(1),
            Expression.literal(1),
            Expression.literal(2),
            Expression.literal(2),
            Expression.literal(3)
        )
            .toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLinearStops() {
        val expected =
            arrayOf<Any>("interpolate", arrayOf<Any>("linear"), 12f, 0f, 1f, 1f, 2f, 2f, 3f)
        val actual = Expression.interpolate(
            Expression.linear(),
            Expression.literal(12),
            Expression.stop(0, 1),
            Expression.stop(1, 2),
            Expression.stop(2, 3)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testExponential() {
        val exponential = arrayOf<Any>("exponential", 12f)
        val get = arrayOf<Any>("get", "x")
        val expected = arrayOf<Any>("interpolate", exponential, get, 0f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.exponential(Expression.literal(12)),
            Expression.get(Expression.literal("x")),
            Expression.literal(0),
            Expression.literal(100),
            Expression.literal(200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testExponentialLiteral() {
        val exponential = arrayOf<Any>("exponential", 12f)
        val get = arrayOf<Any>("get", "x")
        val expected = arrayOf<Any>("interpolate", exponential, get, 0f, 100f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.exponential(12),
            Expression.get("x"),
            Expression.stop(0, 100),
            Expression.stop(100, 200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testExponentialExpressionLiteral() {
        val getX = arrayOf<Any>("get", "x")
        val exponential = arrayOf<Any>("exponential", getX)
        val getY = arrayOf<Any>("get", "y")
        val expected = arrayOf<Any>("interpolate", exponential, getY, 0f, 100f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.exponential(Expression.get("x")),
            Expression.get("y"),
            Expression.stop(0, 100),
            Expression.stop(100, 200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCubicBezier() {
        val cubicBezier = arrayOf<Any>("cubic-bezier", 1f, 1f, 1f, 1f)
        val get = arrayOf<Any>("get", "x")
        val expected = arrayOf<Any>("interpolate", cubicBezier, get, 0f, 100f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.cubicBezier(
                Expression.literal(1),
                Expression.literal(1),
                Expression.literal(1),
                Expression.literal(1)
            ),
            Expression.get(Expression.literal("x")),
            Expression.literal(0),
            Expression.literal(100),
            Expression.literal(100),
            Expression.literal(200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCubicBezierLiteral() {
        val cubicBezier = arrayOf<Any>("cubic-bezier", 1f, 1f, 1f, 1f)
        val get = arrayOf<Any>("get", "x")
        val expected = arrayOf<Any>("interpolate", cubicBezier, get, 0f, 100f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.cubicBezier(1, 1, 1, 1),
            Expression.get("x"),
            Expression.stop(0, 100),
            Expression.stop(100, 200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCubicBezierExpression() {
        val getX = arrayOf<Any>("get", "x")
        val getY = arrayOf<Any>("get", "y")
        val getZ = arrayOf<Any>("get", "z")
        val cubicBezier = arrayOf<Any>("cubic-bezier", getZ, 1f, getY, 1f)
        val expected = arrayOf<Any>("interpolate", cubicBezier, getX, 0f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.cubicBezier(
                Expression.get(Expression.literal("z")),
                Expression.literal(1),
                Expression.get(Expression.literal("y")),
                Expression.literal(1)
            ),
            Expression.get(Expression.literal("x")),
            Expression.literal(0),
            Expression.literal(100),
            Expression.literal(200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testCubicBezierExpressionLiteral() {
        val getX = arrayOf<Any>("get", "x")
        val getY = arrayOf<Any>("get", "y")
        val getZ = arrayOf<Any>("get", "z")
        val cubicBezier = arrayOf<Any>("cubic-bezier", getZ, 1f, getY, 1f)
        val expected = arrayOf<Any>("interpolate", cubicBezier, getX, 0f, 100f, 100f, 200f)
        val actual = Expression.interpolate(
            Expression.cubicBezier(
                Expression.get("z"),
                Expression.literal(1),
                Expression.get("y"),
                Expression.literal(1)
            ),
            Expression.get("x"),
            Expression.stop(0, 100),
            Expression.stop(100, 200)
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testExpressionConcatToString() {
        val expected = "[\"concat\", \"foo\", \"bar\"]"
        val actual =
            Expression.concat(Expression.literal("foo"), Expression.literal("bar")).toString()
        org.junit.Assert.assertEquals("toString should match", expected, actual)
    }

    @Test
    @Throws(Exception::class)
    fun testExpressionMinToString() {
        val expected = "[\"min\", 0.0, 1.0, 2.0, 3.0]"
        val actual = Expression.min(0, 1, 2, 3).toString()
        org.junit.Assert.assertEquals("toString should match", expected, actual)
    }

    @Test
    @Throws(Exception::class)
    fun testExpressionExponentialToString() {
        val expected = (
            "[\"interpolate\", [\"cubic-bezier\", 1.0, 1.0, 1.0, 1.0]," +
                " [\"get\", \"x\"], 0.0, 100.0, 100.0, 200.0]"
            )
        val actual = Expression.interpolate(
            Expression.cubicBezier(
                Expression.literal(1),
                Expression.literal(1),
                Expression.literal(1),
                Expression.literal(1)
            ),
            Expression.get(Expression.literal("x")),
            Expression.literal(0),
            Expression.literal(100),
            Expression.literal(100),
            Expression.literal(200)
        ).toString()
        org.junit.Assert.assertEquals("toString should match", expected, actual)
    }

    @Test
    @Throws(Exception::class)
    fun testLiteralArray() {
        val array = arrayOf<Any>(1, "text")
        val expected = arrayOf<Any>("literal", array)
        val actual = Expression.literal(array).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    @Throws(Exception::class)
    fun testLiteralArrayString() {
        val array = arrayOf<Any>(1, "text")
        val expected = "[\"literal\", [1, \"text\"]]"
        val actual = Expression.literal(array).toString()
        org.junit.Assert.assertEquals("literal array should match", expected, actual)
    }

    @Test
    @Throws(Exception::class)
    fun testLiteralPrimitiveArrayConversion() {
        val array = floatArrayOf(0.2f, 0.5f)
        val expected = arrayOf<Any>("literal", arrayOf<Any>(0.2f, 0.5f))
        val actual = Expression.literal(array).toArray()
        org.junit.Assert.assertEquals("primitive array should be converted", expected, actual)
    }

    @Test
    fun testColorConversion() {
        val greenColor = Expression.color(-0xff0100)
        val expected = arrayOf<Any>("rgba", 0f, 255f, 0f, 1f)
        Assert.assertTrue(
            "expression should match",
            Arrays.deepEquals(expected, greenColor.toArray())
        )
    }

    @Test(expected = IllegalArgumentException::class)
    fun testThrowIllegalArgumentExceptionForPropertyValueLiteral() {
        val expression = Expression.interpolate(
            Expression.exponential(1f),
            Expression.zoom(),
            Expression.stop(17f, PropertyFactory.lineOpacity(1f)),
            Expression.stop(16.5f, PropertyFactory.lineOpacity(0.5f)),
            Expression.stop(16f, PropertyFactory.lineOpacity(0f))
        )
        expression.toArray()
    }

    @Test
    fun testRound() {
        val expected = arrayOf<Any>("round", 2.2f)
        val actual = Expression.round(2.2f).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testRoundLiteral() {
        val expected = arrayOf<Any>("round", 2.2f)
        val actual = Expression.round(Expression.literal(2.2f)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testAbs() {
        val expected = arrayOf<Any>("abs", -2.2f)
        val actual = Expression.abs(-2.2f).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testAbsLiteral() {
        val expected = arrayOf<Any>("abs", -2.2f)
        val actual = Expression.abs(Expression.literal(-2.2f)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testCeil() {
        val expected = arrayOf<Any>("ceil", 2.2f)
        val actual = Expression.ceil(2.2f).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testCeilLiteral() {
        val expected = arrayOf<Any>("ceil", 2.2f)
        val actual = Expression.ceil(Expression.literal(2.2f)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testFloor() {
        val expected = arrayOf<Any>("floor", 2.2f)
        val actual = Expression.floor(2.2f).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testFloorLiteral() {
        val expected = arrayOf<Any>("floor", 2.2f)
        val actual = Expression.floor(Expression.literal(2.2f)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testRawEmpty() {
        var raw = "[\"get\", ]"
        var expected = Expression.get("")
        org.junit.Assert.assertEquals(
            "expressions should match",
            Expression.raw(
                raw
            ),
            expected
        )
        raw = "[\"get\", key]"
        expected = Expression.get("key")
        org.junit.Assert.assertEquals("expressions should match", Expression.raw(raw), expected)
    }

    @Test
    fun testRawAndroidColors() {
        val expected = Expression.interpolate(
            Expression.linear(),
            Expression.zoom(),
            Expression.stop(
                12,
                Expression.step(
                    Expression.get("stroke-width"),
                    Expression.color(Color.BLACK),
                    Expression.stop(1f, Expression.color(Color.RED)),
                    Expression.stop(2f, Expression.color(Color.WHITE)),
                    Expression.stop(3f, Expression.color(Color.BLUE))
                )
            ),
            Expression.stop(
                15,
                Expression.step(
                    Expression.get("stroke-width"),
                    Expression.color(Color.BLACK),
                    Expression.stop(1f, Expression.color(Color.YELLOW)),
                    Expression.stop(2f, Expression.color(Color.LTGRAY)),
                    Expression.stop(3f, Expression.color(Color.CYAN))
                )
            ),
            Expression.stop(
                18,
                Expression.step(
                    Expression.get("stroke-width"),
                    Expression.color(Color.BLACK),
                    Expression.stop(1f, Expression.color(Color.WHITE)),
                    Expression.stop(2f, Expression.color(Color.GRAY)),
                    Expression.stop(3f, Expression.color(Color.GREEN))
                )
            )
        )
        org.junit.Assert.assertEquals(
            "expressions should match",
            expected,
            Expression.raw(expected.toString())
        )
    }

    @Test
    fun testRawRgbaColor() {
        val expected = Expression.interpolate(
            Expression.exponential(2f), Expression.zoom(),
            Expression.literal(5f), Expression.literal("rgba(0, 0, 0, 1)"),
            Expression.literal(10.5f), Expression.literal("rgb(255, 0, 0)"),
            Expression.literal(15), Expression.color(Color.GREEN),
            Expression.literal(20), Expression.literal(ColorUtils.colorToRgbaString(Color.BLUE))
        )
        org.junit.Assert.assertEquals(
            "expressions should match",
            expected,
            Expression.raw(expected.toString())
        )
    }

    @Test
    fun testRawMatchStrings() {
        val expected = Expression.match(
            Expression.get("property"),
            Expression.literal(""),
            Expression.stop("layer1", "image1"),
            Expression.stop("layer2", "image2")
        )
        org.junit.Assert.assertEquals(
            "expressions should match",
            expected,
            Expression.raw(expected.toString())
        )
    }

    @Test
    fun testRawMatchNumbers() {
        val expected = Expression.match(
            Expression.get("property"),
            Expression.literal(""),
            Expression.stop("layer1", 2),
            Expression.stop("layer2", 2.7)
        )
        org.junit.Assert.assertEquals(
            "expressions should match",
            expected,
            Expression.raw(expected.toString())
        )
    }

    @Test
    fun testAlphaValueInColorConversion() {
        // regression test for #12198
        val colorExpression = Expression.color(Color.parseColor("#41FF0000")) // 25.4% alpha red
        val result = colorExpression.toArray()
        org.junit.Assert.assertEquals(
            "alpha value should match",
            0.254f,
            (result[4] as Float),
            0.001f
        )
    }

    @Test
    fun testAlphaValueInStringConversion() {
        val color = ColorUtils.colorToRgbaString(Color.parseColor("#41FF0000"))
            .split(" ".toRegex()).dropLastWhile { it.isEmpty() }.toTypedArray()[3]
        val alpha = color.substring(0, color.length - 1)
        org.junit.Assert.assertEquals(
            "alpha value should match",
            0.254f,
            java.lang.Float.valueOf(alpha),
            0.001f
        )
    }

    @Test
    fun testCollator() {
        val expected = arrayOf<Any>(
            "collator",
            object : HashMap<String?, Any?>() {
                init {
                    put("case-sensitive", true)
                    put("diacritic-sensitive", true)
                    put("locale", "it-IT")
                }
            }
        )
        val actual = Expression.collator(true, true, Locale.ITALY).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testStringCollator() {
        val expected =
            (
                "[\"collator\", {\"diacritic-sensitive\": true, \"case-sensitive\": true, \"locale\": " +
                    "\"it\"}]"
                )
        val actual = Expression.collator(true, true, Locale.ITALIAN).toString()
        org.junit.Assert.assertEquals("expression should match", expected, actual)
    }

    @Test
    fun testResolvedLocale() {
        val expected = arrayOf<Any>(
            "resolved-locale",
            arrayOf<Any>(
                "collator",
                object : HashMap<String?, Any?>() {
                    init {
                        put("case-sensitive", false)
                        put("diacritic-sensitive", false)
                        put("locale", "it")
                    }
                }
            )
        )
        val actual =
            Expression.resolvedLocale(Expression.collator(false, false, Locale.ITALIAN)).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testRawCollator() {
        val expected = arrayOf<Any>(
            "collator",
            object : HashMap<String?, Any?>() {
                init {
                    put("case-sensitive", true)
                    put("diacritic-sensitive", true)
                    put("locale", "it-IT")
                }
            }
        )
        val actual = Expression.raw(
            "[\"collator\", {\"diacritic-sensitive\": true, \"case-sensitive\": true, \"locale\": " +
                "\"it-IT\"}]"
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testRawCollatorDoubleConversion() {
        val expected = Expression.collator(false, false, Locale.ITALIAN)
        val actual = Expression.raw(expected.toString()).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected.toArray(), actual))
    }

    @Test
    fun testStringNestedCollator() {
        val expected =
            (
                "[\"collator\", {\"diacritic-sensitive\": [\"==\", 2.0, 1.0], \"case-sensitive\": false," +
                    " \"locale\": \"it\"}]"
                )
        val actual = Expression.collator(
            Expression.literal(false),
            Expression.eq(Expression.literal(2), Expression.literal(1)),
            Expression.literal("it")
        ).toString()
        org.junit.Assert.assertEquals("expression should match", expected, actual)
    }

    @Test
    fun testStringReverseConversion() {
        val expected = "[\"to-string\", [\"get\", \"name_en\"]]"
        val actual = Expression.toString(Expression.get("name_en")).toString()
        org.junit.Assert.assertEquals("Reverse string conversion should match", expected, actual)
    }

    @Test
    fun testIsSupportedScriptLiteral() {
        val expected = arrayOf<Any>("is-supported-script", "ಗೌರವಾರ್ಥವಾಗಿ")
        val actual = Expression.isSupportedScript("ಗೌರವಾರ್ಥವಾಗಿ").toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testIsSupportedScriptExpressions() {
        val expected = arrayOf<Any>("is-supported-script", arrayOf<Any>("get", "property_name"))
        val actual = Expression.isSupportedScript(Expression.get("property_name")).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testFormatSingleArgument() {
        val expected = arrayOf<Any>(
            "format",
            "test",
            object : TestableExpressionHashMap() {
                init {
                    put("font-scale", 1.5f)
                    put("text-font", arrayOf<Any>("literal", arrayOf("awesome")))
                    put("text-color", arrayOf<Any>("rgb", 255f, 0f, 0f))
                }
            }
        )
        val actual = Expression.format(
            Expression.formatEntry(
                Expression.literal("test"),
                FormatOption.formatFontScale(Expression.literal(1.5)),
                FormatOption.formatTextFont(Expression.literal(arrayOf("awesome"))),
                FormatOption.formatTextColor(Expression.rgb(255, 0, 0))
            )
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    @Test
    fun testFormatMultipleArgument() {
        val expected = arrayOf<Any>(
            "format",
            "test",
            object : TestableExpressionHashMap() {
                init {
                    put("text-font", arrayOf<Any>("literal", arrayOf("awesome")))
                }
            },
            "test2",
            object : TestableExpressionHashMap() {
                init {
                    put("font-scale", 1.5f)
                }
            },
            "test3",
            object : TestableExpressionHashMap() {
            },
            "test4",
            object : TestableExpressionHashMap() {
                init {
                    put("text-color", arrayOf<Any>("rgb", 255f, 0f, 0f))
                }
            },
            "test5",
            object : TestableExpressionHashMap() {
                init {
                    put("font-scale", 1.5f)
                    put("text-font", arrayOf<Any>("literal", arrayOf("awesome")))
                    put("text-color", arrayOf<Any>("rgb", 255f, 0f, 0f))
                }
            }
        )
        val actual = Expression.format(
            Expression.formatEntry(
                Expression.literal("test"),
                FormatOption.formatTextFont(arrayOf("awesome"))
            ),
            Expression.formatEntry("test2", FormatOption.formatFontScale(1.5)),
            Expression.formatEntry(Expression.literal("test3")),
            Expression.formatEntry(
                Expression.literal("test4"),
                FormatOption.formatTextColor(Expression.rgb(255, 0, 0))
            ),
            Expression.formatEntry(
                Expression.literal("test5"),
                FormatOption.formatFontScale(Expression.literal(1.5)),
                FormatOption.formatTextFont(arrayOf("awesome")),
                FormatOption.formatTextColor(Expression.rgb(255, 0, 0))
            )
        ).toArray()
        Assert.assertTrue("expression should match", Arrays.deepEquals(expected, actual))
    }

    /**
     * This class overrides [java.util.AbstractMap.equals]
     * in order to correctly compare nodes values if they are arrays,
     * which is the case for [Expression.format]'s "text-format" argument.
     */
    private open inner class TestableExpressionHashMap : HashMap<String?, Any?>() {
        override fun equals(o: Any?): Boolean {
            if (o === this) {
                return true
            }
            if (o !is Map<*, *>) {
                return false
            }
            val m = o
            if (m.size != size) {
                return false
            }
            try {
                for ((key, value) in entries) {
                    if (value == null) {
                        if (!(m[key] == null && m.containsKey(key))) {
                            return false
                        }
                    } else {
                        if (value is Array<*>) {
                            // Use Arrays.deepEquals() if values are Object arrays.
                            if (!Arrays.deepEquals(value as Array<Any?>?, m[key] as Array<Any?>?)) {
                                return false
                            }
                        } else {
                            if (value != m[key]) {
                                return false
                            }
                        }
                    }
                }
            } catch (unused: ClassCastException) {
                return false
            } catch (unused: NullPointerException) {
                return false
            }
            return true
        }
    }
}
