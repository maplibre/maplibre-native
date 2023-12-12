package org.maplibre.android.utils

import android.os.Parcel
import android.os.Parcelable
import org.junit.Assert.*
import org.mockito.ArgumentMatchers
import org.mockito.Mockito.*
import org.mockito.stubbing.Answer

object MockParcel {
    @JvmOverloads
    fun obtain(`object`: Parcelable, describeContentsValue: Int = 0): Parcelable? {
        testDescribeContents(`object`, describeContentsValue)
        testParcelableArray(`object`)
        return testParcelable(`object`)
    }

    fun testParcelable(`object`: Parcelable): Parcelable? {
        val parcel = ParcelMocker.obtain(`object`)
        `object`.writeToParcel(parcel, 0)
        parcel.setDataPosition(0)
        return try {
            val field = `object`.javaClass.getDeclaredField("CREATOR")
            field.isAccessible = true
            val creatorClass = field.type
            val fieldValue = field[`object`]
            val myMethod = creatorClass.getDeclaredMethod("createFromParcel", Parcel::class.java)
            myMethod.invoke(fieldValue, parcel) as Parcelable
        } catch (exception: Exception) {
            null
        }
    }

    fun testParcelableArray(`object`: Parcelable) {
        val objects = arrayOf(`object`)
        val parcel = ParcelMocker.obtain(objects)
        parcel.writeParcelableArray(objects, 0)
        parcel.setDataPosition(0)
        val parcelableArray = parcel.readParcelableArray(`object`.javaClass.classLoader)
        assertArrayEquals("parcel should match initial object", objects, parcelableArray)
    }

    fun testDescribeContents(`object`: Parcelable, describeContentsValue: Int) {
        if (describeContentsValue == 0) {
            assertEquals("""
Expecting a describeContents() value of 0 for a ${`object`.javaClass.simpleName} instance.
You can provide a different value for describeContentValue through the obtain method.""",
                    0,
                    `object`.describeContents().toLong())
        } else {
            assertEquals("Expecting a describeContents() value of $describeContentsValue",
                    describeContentsValue.toLong(),
                    `object`.describeContents().toLong())
        }
    }

    private class ParcelMocker private constructor(private val `object`: Any) {
        private val objects: MutableList<Any>
        private val mockedParcel: Parcel
        private var position = 0

        init {
            mockedParcel = mock(Parcel::class.java)
            objects = ArrayList()
            setupMock()
        }

        private fun setupMock() {
            setupWrites()
            setupReads()
            setupOthers()
        }

        private fun setupWrites() {
            val writeValueAnswer: Answer<Void> = Answer { invocation ->
                val parameter = invocation.arguments[0]
                objects.add(parameter)
                null
            }
            val writeArrayAnswer: Answer<Void> = Answer { invocation ->
                val parameters = invocation.arguments[0] as Array<Any>
                objects.add(parameters.size)
                for (o in parameters) {
                    objects.add(o)
                }
                null
            }
            val writeIntArrayAnswer: Answer<Void> = Answer { invocation ->
                val parameters = invocation.arguments[0] as IntArray
                if (parameters != null) {
                    objects.add(parameters.size)
                    for (o in parameters) {
                        objects.add(o)
                    }
                } else {
                    objects.add(-1)
                }
                null
            }
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeByte(ArgumentMatchers.anyByte())
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeLong(ArgumentMatchers.anyLong())
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeString(ArgumentMatchers.anyString())
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeInt(ArgumentMatchers.anyInt())
            doAnswer(writeIntArrayAnswer).`when`(mockedParcel).writeIntArray(ArgumentMatchers.any(IntArray::class.java))
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeDouble(ArgumentMatchers.anyDouble())
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeFloat(ArgumentMatchers.anyFloat())
            doAnswer(writeValueAnswer).`when`(mockedParcel).writeParcelable(ArgumentMatchers.any(Parcelable::class.java), ArgumentMatchers.eq(0))
            doAnswer(writeArrayAnswer).`when`(mockedParcel).writeParcelableArray(ArgumentMatchers.any(Array<Parcelable>::class.java), ArgumentMatchers.eq(0))
        }

        private fun setupReads() {
            `when`(mockedParcel.readInt()).then { objects[position++] as Int }
            `when`(mockedParcel.readByte()).thenAnswer { objects[position++] as Byte }
            `when`(mockedParcel.readLong()).thenAnswer { objects[position++] as Long }
            `when`(mockedParcel.readString()).thenAnswer { objects[position++] as String }
            `when`(mockedParcel.readDouble()).thenAnswer { objects[position++] as Double }
            `when`(mockedParcel.readFloat()).thenAnswer { objects[position++] as Float }
            `when`<Any?>(mockedParcel.readParcelable(Parcelable::class.java.classLoader)).thenAnswer { objects[position++] as Parcelable }
            `when`(mockedParcel.readParcelableArray(Parcelable::class.java.classLoader)).thenAnswer {
                val size = objects[position++] as Int
                val field = `object`.javaClass.getDeclaredField("CREATOR")
                field.isAccessible = true
                val creatorClass = field.type
                val fieldValue = field[`object`]
                val myMethod = creatorClass.getDeclaredMethod("newArray", Int::class.javaPrimitiveType)
                val array = myMethod.invoke(fieldValue, size) as Array<Any>
                for (i in 0 until size) {
                    array[i] = objects[position++]
                }
                array
            }
            `when`(mockedParcel.createIntArray()).then(Answer {
                val size = objects[position++] as Int
                if (size == -1) {
                    return@Answer null
                }
                val array = IntArray(size)
                for (i in 0 until size) {
                    array[i] = objects[position++] as Int
                }
                array
            })
        }

        private fun setupOthers() {
            doAnswer { invocation ->
                position = invocation.arguments[0] as Int
                null
            }.`when`(mockedParcel).setDataPosition(ArgumentMatchers.anyInt())
        }

        companion object {
            fun obtain(target: Parcelable): Parcel {
                val parcel = ParcelMocker(target).mockedParcel
                target.writeToParcel(parcel, 0)
                parcel.setDataPosition(0)
                return parcel
            }

            fun obtain(targets: Array<Parcelable>): Parcel {
                require(targets.size != 0) { "The passed argument may not be empty" }
                val parcel = ParcelMocker(targets[0]).mockedParcel
                parcel.writeParcelableArray(targets, 0)
                parcel.setDataPosition(0)
                return parcel
            }
        }
    }
}
