package org.maplibre.android

import android.content.Context
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Assert.fail
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.Parameterized
import org.mockito.Mockito
import java.lang.reflect.InvocationTargetException

@RunWith(Parameterized::class)
class LibraryLoaderConstructorTest(
    private val peerClassName: String,
    private val signature: String,
) {
    @Test
    fun constructorLoadsLibraryBeforeNativeAccess() {
        val classLoader = IsolatedMapLibreClassLoader(requireNotNull(javaClass.classLoader))
        val libraryLoaderClass = Class.forName("org.maplibre.android.LibraryLoader", true, classLoader)

        fun installLoader(name: String) {
            val loader = Class.forName(name, true, classLoader).getDeclaredConstructor().newInstance()
            libraryLoaderClass.getMethod("setLibraryLoader", libraryLoaderClass).invoke(null, loader)
        }
        installLoader("org.maplibre.android.NoOpLibraryLoader")

        // Complete class initialization before installing the sentinel: only the constructor
        // under test, rather than its static initializer, may request the native library.
        val peerClass = Class.forName(peerClassName, true, classLoader)
        val constructor =
            peerClass.declaredConstructors
                .single {
                    it.parameterTypes.joinToString(",") { type -> type.simpleName } == signature
                }.apply { isAccessible = true }
        val arguments =
            constructor.parameterTypes
                .map { type ->
                    when (type) {
                        java.lang.Long.TYPE -> {
                            0L
                        }

                        String::class.java -> {
                            "test"
                        }

                        ByteArray::class.java -> {
                            byteArrayOf()
                        }

                        Context::class.java -> {
                            Mockito.mock(Context::class.java).also {
                                Mockito.`when`(it.applicationContext).thenReturn(it)
                            }
                        }

                        else -> {
                            Mockito.mock(type)
                        }
                    }
                }.toTypedArray()

        installLoader("org.maplibre.android.ThrowingLibraryLoader")
        libraryLoaderClass.getDeclaredField("loaded").apply { isAccessible = true }.setBoolean(null, false)
        try {
            constructor.newInstance(*arguments)
            fail("$peerClassName($signature) did not request the native library")
        } catch (error: InvocationTargetException) {
            val cause = error.targetException
            assertTrue("Expected the loader sentinel, got $cause", cause is AssertionError)
            assertEquals(LIBRARY_LOAD_SENTINEL, cause.message)
        }
    }

    companion object {
        @JvmStatic
        @Parameterized.Parameters(name = "{0}({1})")
        fun constructors(): List<Array<String>> =
            listOf(
                arrayOf("org.maplibre.android.maps.NativeMapView", "Context,NativeMapOptions,ViewCallback,StateCallback,MapRenderer"),
                arrayOf("org.maplibre.android.TestMapRenderer", "Context"),
                arrayOf("org.maplibre.android.net.NativeConnectivityListener", ""),
                arrayOf("org.maplibre.android.net.NativeConnectivityListener", "long"),
                arrayOf("org.maplibre.android.offline.OfflineManager", "Context"),
                arrayOf("org.maplibre.android.offline.OfflineRegion", "long,FileSource,long,OfflineRegionDefinition,byte[]"),
                arrayOf("org.maplibre.android.style.layers.BackgroundLayer", "String"),
                arrayOf("org.maplibre.android.style.layers.BackgroundLayer", "long"),
                arrayOf("org.maplibre.android.style.sources.GeoJsonSource", "String"),
                arrayOf("org.maplibre.android.style.sources.GeoJsonSource", "long"),
            )
    }
}
