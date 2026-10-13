package org.maplibre.android

import org.junit.Assert.assertFalse
import org.junit.Assert.assertSame
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import org.junit.runners.Parameterized
import java.util.concurrent.CountDownLatch
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicReference

@RunWith(Parameterized::class)
class LibraryLoaderClassInitializationTest(
    private val peerClassName: String,
) {
    /**
     * JNI_OnLoad registers these peers while LibraryLoader.load holds its class monitor.
     * Initializing a peer on another thread must not acquire that monitor.
     */
    @Test
    fun classInitializationDoesNotWaitForLibraryLoader() {
        val classLoader = IsolatedMapLibreClassLoader(requireNotNull(javaClass.classLoader))
        val libraryLoaderClass = Class.forName("org.maplibre.android.LibraryLoader", true, classLoader)
        val noOpLoaderClass = Class.forName("org.maplibre.android.NoOpLibraryLoader", true, classLoader)
        val noOpLoader = noOpLoaderClass.getDeclaredConstructor().newInstance()
        libraryLoaderClass.getMethod("setLibraryLoader", libraryLoaderClass).invoke(null, noOpLoader)

        val completed = CountDownLatch(1)
        val failure = AtomicReference<Throwable>()
        val worker =
            Thread({
                try {
                    val peerClass = Class.forName(peerClassName, true, classLoader)
                    assertSame(classLoader, peerClass.classLoader)
                } catch (error: Throwable) {
                    failure.set(error)
                } finally {
                    completed.countDown()
                }
            }, "initialize-maplibre-peer").apply { isDaemon = true }

        val completedWhileLoaderLocked =
            try {
                synchronized(libraryLoaderClass) {
                    worker.start()
                    completed.await(TIMEOUT_SECONDS, TimeUnit.SECONDS)
                }
            } finally {
                // Release the loader monitor before joining, including on a failing baseline.
                worker.join(TimeUnit.SECONDS.toMillis(TIMEOUT_SECONDS))
            }

        assertFalse("Class initialization worker did not finish after releasing the loader monitor", worker.isAlive)
        failure.get()?.let { throw AssertionError("Could not initialize $peerClassName", it) }
        assertTrue("$peerClassName initialization waited for LibraryLoader.load's monitor", completedWhileLoaderLocked)
    }

    companion object {
        private const val TIMEOUT_SECONDS = 5L

        @JvmStatic
        @Parameterized.Parameters(name = "{0}")
        fun peerClasses(): List<Array<String>> =
            listOf(
                "org.maplibre.android.maps.NativeMapView",
                "org.maplibre.android.maps.renderer.MapRenderer",
                "org.maplibre.android.net.NativeConnectivityListener",
                "org.maplibre.android.offline.OfflineManager",
                "org.maplibre.android.offline.OfflineRegion",
                "org.maplibre.android.style.layers.Layer",
                "org.maplibre.android.style.sources.Source",
            ).map { arrayOf(it) }
    }
}
