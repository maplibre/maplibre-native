package org.maplibre.android.maps

import com.google.common.util.concurrent.ExecutionError
import org.maplibre.android.log.Logger
import org.maplibre.android.log.LoggerDefinition
import org.maplibre.android.maps.MapView.*
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.maplibre.android.BaseTest
import org.mockito.ArgumentMatchers
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.MockitoAnnotations

/**
 * Tests integration of MapChangeDispatcher and see if events are correctly forwarded.
 */
class MapChangeReceiverTest : BaseTest() {
    private var mapChangeEventManager: MapChangeReceiver? = null

    @Mock
    private val onCameraWillChangeListener: OnCameraWillChangeListener? = null

    @Mock
    private val onCameraDidChangeListener: OnCameraDidChangeListener? = null

    @Mock
    private val onCameraIsChangingListener: OnCameraIsChangingListener? = null

    @Mock
    private val onWillStartLoadingMapListener: OnWillStartLoadingMapListener? = null

    @Mock
    private val onDidFinishLoadingMapListener: OnDidFinishLoadingMapListener? = null

    @Mock
    private val onDidFailLoadingMapListener: OnDidFailLoadingMapListener? = null

    @Mock
    private val onWillStartRenderingFrameListener: OnWillStartRenderingFrameListener? = null

    @Mock
    private val onDidFinishRenderingFrameListener: OnDidFinishRenderingFrameListener? = null

    @Mock
    private val onDidFinishRenderingFrameWithStatsListener: OnDidFinishRenderingFrameWithStatsListener? = null

    @Mock
    private val onWillStartRenderingMapListener: OnWillStartRenderingMapListener? = null

    @Mock
    private val onDidFinishRenderingMapListener: OnDidFinishRenderingMapListener? = null

    @Mock
    private val onDidBecomeIdleListener: OnDidBecomeIdleListener? = null

    @Mock
    private val onDidFinishLoadingStyleListener: OnDidFinishLoadingStyleListener? = null

    @Mock
    private val onSourceChangedListener: OnSourceChangedListener? = null

    @Mock
    private val loggerDefinition: LoggerDefinition? = null

    @Before
    fun beforeTest() {
        MockitoAnnotations.initMocks(this)
        mapChangeEventManager = MapChangeReceiver()
    }

    @Test
    fun testOnCameraRegionWillChangeListener() {
        mapChangeEventManager!!.addOnCameraWillChangeListener(onCameraWillChangeListener)
        mapChangeEventManager!!?.onCameraWillChange(false)
        Mockito.verify(onCameraWillChangeListener)?.onCameraWillChange(false)
        mapChangeEventManager!!.removeOnCameraWillChangeListener(onCameraWillChangeListener)
        mapChangeEventManager!!?.onCameraWillChange(false)
        Mockito.verify(onCameraWillChangeListener)?.onCameraWillChange(false)
        mapChangeEventManager!!.addOnCameraWillChangeListener(onCameraWillChangeListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onCameraWillChangeListener)?.onCameraWillChange(false)
        try {
            mapChangeEventManager!!?.onCameraWillChange(false)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onCameraWillChangeListener)?.onCameraWillChange(false)
        try {
            mapChangeEventManager!!?.onCameraWillChange(false)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnCameraRegionWillChangeAnimatedListener() {
        mapChangeEventManager!!.addOnCameraWillChangeListener(onCameraWillChangeListener)
        mapChangeEventManager!!?.onCameraWillChange(true)
        Mockito.verify(onCameraWillChangeListener)?.onCameraWillChange(true)
        mapChangeEventManager!!.removeOnCameraWillChangeListener(onCameraWillChangeListener)
        mapChangeEventManager!!?.onCameraWillChange(true)
        Mockito.verify(onCameraWillChangeListener)?.onCameraWillChange(true)
        mapChangeEventManager!!.addOnCameraWillChangeListener(onCameraWillChangeListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onCameraWillChangeListener)?.onCameraWillChange(true)
        try {
            mapChangeEventManager!!?.onCameraWillChange(true)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onCameraWillChangeListener)?.onCameraWillChange(true)
        try {
            mapChangeEventManager!!?.onCameraWillChange(true)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnCameraIsChangingListener() {
        mapChangeEventManager!!.addOnCameraIsChangingListener(onCameraIsChangingListener)
        mapChangeEventManager!!?.onCameraIsChanging()
        Mockito.verify(onCameraIsChangingListener)?.onCameraIsChanging()
        mapChangeEventManager!!.removeOnCameraIsChangingListener(onCameraIsChangingListener)
        mapChangeEventManager!!?.onCameraIsChanging()
        Mockito.verify(onCameraIsChangingListener)?.onCameraIsChanging()
        mapChangeEventManager!!.addOnCameraIsChangingListener(onCameraIsChangingListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onCameraIsChangingListener)?.onCameraIsChanging()
        try {
            mapChangeEventManager!!?.onCameraIsChanging()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onCameraIsChangingListener)?.onCameraIsChanging()
        try {
            mapChangeEventManager!!?.onCameraIsChanging()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnCameraRegionDidChangeListener() {
        mapChangeEventManager!!.addOnCameraDidChangeListener(onCameraDidChangeListener)
        mapChangeEventManager!!?.onCameraDidChange(false)
        Mockito.verify(onCameraDidChangeListener)?.onCameraDidChange(false)
        mapChangeEventManager!!.removeOnCameraDidChangeListener(onCameraDidChangeListener)
        mapChangeEventManager!!?.onCameraDidChange(false)
        Mockito.verify(onCameraDidChangeListener)?.onCameraDidChange(false)
        mapChangeEventManager!!.addOnCameraDidChangeListener(onCameraDidChangeListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onCameraDidChangeListener)?.onCameraDidChange(false)
        try {
            mapChangeEventManager!!?.onCameraDidChange(false)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onCameraDidChangeListener)?.onCameraDidChange(false)
        try {
            mapChangeEventManager!!?.onCameraDidChange(false)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnCameraRegionDidChangeAnimatedListener() {
        mapChangeEventManager!!.addOnCameraDidChangeListener(onCameraDidChangeListener)
        mapChangeEventManager!!?.onCameraDidChange(true)
        Mockito.verify(onCameraDidChangeListener)?.onCameraDidChange(true)
        mapChangeEventManager!!.removeOnCameraDidChangeListener(onCameraDidChangeListener)
        mapChangeEventManager!!?.onCameraDidChange(true)
        Mockito.verify(onCameraDidChangeListener)?.onCameraDidChange(true)
        mapChangeEventManager!!.addOnCameraDidChangeListener(onCameraDidChangeListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onCameraDidChangeListener)?.onCameraDidChange(true)
        try {
            mapChangeEventManager!!?.onCameraDidChange(true)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onCameraDidChangeListener)?.onCameraDidChange(true)
        try {
            mapChangeEventManager!!?.onCameraDidChange(true)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnWillStartLoadingMapListener() {
        mapChangeEventManager!!.addOnWillStartLoadingMapListener(onWillStartLoadingMapListener)
        mapChangeEventManager!!?.onWillStartLoadingMap()
        Mockito.verify(onWillStartLoadingMapListener)?.onWillStartLoadingMap()
        mapChangeEventManager!!.removeOnWillStartLoadingMapListener(onWillStartLoadingMapListener)
        mapChangeEventManager!!?.onWillStartLoadingMap()
        Mockito.verify(onWillStartLoadingMapListener)?.onWillStartLoadingMap()
        mapChangeEventManager!!.addOnWillStartLoadingMapListener(onWillStartLoadingMapListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onWillStartLoadingMapListener)?.onWillStartLoadingMap()
        try {
            mapChangeEventManager!!?.onWillStartLoadingMap()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onWillStartLoadingMapListener)?.onWillStartLoadingMap()
        try {
            mapChangeEventManager!!?.onWillStartLoadingMap()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishLoadingMapListener() {
        mapChangeEventManager!!.addOnDidFinishLoadingMapListener(onDidFinishLoadingMapListener)
        mapChangeEventManager!!?.onDidFinishLoadingMap()
        Mockito.verify(onDidFinishLoadingMapListener)?.onDidFinishLoadingMap()
        mapChangeEventManager!!.removeOnDidFinishLoadingMapListener(onDidFinishLoadingMapListener)
        mapChangeEventManager!!?.onDidFinishLoadingMap()
        Mockito.verify(onDidFinishLoadingMapListener)?.onDidFinishLoadingMap()
        mapChangeEventManager!!.addOnDidFinishLoadingMapListener(onDidFinishLoadingMapListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishLoadingMapListener)?.onDidFinishLoadingMap()
        try {
            mapChangeEventManager!!?.onDidFinishLoadingMap()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishLoadingMapListener)?.onDidFinishLoadingMap()
        try {
            mapChangeEventManager!!?.onDidFinishLoadingMap()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFailLoadingMapListener() {
        mapChangeEventManager!!.addOnDidFailLoadingMapListener(onDidFailLoadingMapListener)
        mapChangeEventManager!!?.onDidFailLoadingMap(TEST_STRING)
        Mockito.verify(onDidFailLoadingMapListener)?.onDidFailLoadingMap(TEST_STRING)
        mapChangeEventManager!!.removeOnDidFailLoadingMapListener(onDidFailLoadingMapListener)
        mapChangeEventManager!!?.onDidFailLoadingMap(TEST_STRING)
        Mockito.verify(onDidFailLoadingMapListener)?.onDidFailLoadingMap(TEST_STRING)
        mapChangeEventManager!!.addOnDidFailLoadingMapListener(onDidFailLoadingMapListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFailLoadingMapListener)?.onDidFailLoadingMap(TEST_STRING)
        try {
            mapChangeEventManager!!?.onDidFailLoadingMap(TEST_STRING)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFailLoadingMapListener)?.onDidFailLoadingMap(TEST_STRING)
        try {
            mapChangeEventManager!!?.onDidFailLoadingMap(TEST_STRING)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnWillStartRenderingFrameListener() {
        mapChangeEventManager!!.addOnWillStartRenderingFrameListener(
            onWillStartRenderingFrameListener
        )
        mapChangeEventManager!!?.onWillStartRenderingFrame()
        Mockito.verify(onWillStartRenderingFrameListener)?.onWillStartRenderingFrame()
        mapChangeEventManager!!.removeOnWillStartRenderingFrameListener(
            onWillStartRenderingFrameListener
        )
        mapChangeEventManager!!?.onWillStartRenderingFrame()
        Mockito.verify(onWillStartRenderingFrameListener)?.onWillStartRenderingFrame()
        mapChangeEventManager!!.addOnWillStartRenderingFrameListener(
            onWillStartRenderingFrameListener
        )
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onWillStartRenderingFrameListener)?.onWillStartRenderingFrame()
        try {
            mapChangeEventManager!!?.onWillStartRenderingFrame()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onWillStartRenderingFrameListener)?.onWillStartRenderingFrame()
        try {
            mapChangeEventManager!!?.onWillStartRenderingFrame()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishRenderingFrameListener() {
        mapChangeEventManager!!.addOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        Mockito.verify(onDidFinishRenderingFrameListener)?.onDidFinishRenderingFrame(true, .0, .0)
        mapChangeEventManager!!.removeOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        Mockito.verify(onDidFinishRenderingFrameListener)?.onDidFinishRenderingFrame(true, .0, .0)
        mapChangeEventManager!!.addOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameListener
        )
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishRenderingFrameListener)
            ?.onDidFinishRenderingFrame(true, .0, .0)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishRenderingFrameListener)
            ?.onDidFinishRenderingFrame(true, .0, .0)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishRenderingFrameWithStatsListener() {
        mapChangeEventManager!!.addOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameWithStatsListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        Mockito.verify(onDidFinishRenderingFrameWithStatsListener)?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        mapChangeEventManager!!.removeOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameWithStatsListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        Mockito.verify(onDidFinishRenderingFrameWithStatsListener)?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        mapChangeEventManager!!.addOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameWithStatsListener
        )
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishRenderingFrameWithStatsListener)
            ?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishRenderingFrameWithStatsListener)
            ?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingFrame(true, TEST_RENDERING_STATS)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishRenderingFrameFullyRenderedListener() {
        mapChangeEventManager!!.addOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingFrame(false, TEST_RENDERING_STATS)
        Mockito.verify(onDidFinishRenderingFrameListener)?.onDidFinishRenderingFrame(false, .0, .0)
        mapChangeEventManager!!.removeOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingFrame(false, TEST_RENDERING_STATS)
        Mockito.verify(onDidFinishRenderingFrameListener)?.onDidFinishRenderingFrame(false, .0, .0)
        mapChangeEventManager!!.addOnDidFinishRenderingFrameListener(
            onDidFinishRenderingFrameListener
        )
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishRenderingFrameListener)
            ?.onDidFinishRenderingFrame(false, .0, .0)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingFrame(false, TEST_RENDERING_STATS)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishRenderingFrameListener)
            ?.onDidFinishRenderingFrame(false, .0, .0)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingFrame(false, TEST_RENDERING_STATS)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnWillStartRenderingMapListener() {
        mapChangeEventManager!!.addOnWillStartRenderingMapListener(onWillStartRenderingMapListener)
        mapChangeEventManager!!?.onWillStartRenderingMap()
        Mockito.verify(onWillStartRenderingMapListener)?.onWillStartRenderingMap()
        mapChangeEventManager!!.removeOnWillStartRenderingMapListener(
            onWillStartRenderingMapListener
        )
        mapChangeEventManager!!?.onWillStartRenderingMap()
        Mockito.verify(onWillStartRenderingMapListener)?.onWillStartRenderingMap()
        mapChangeEventManager!!.addOnWillStartRenderingMapListener(onWillStartRenderingMapListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onWillStartRenderingMapListener)?.onWillStartRenderingMap()
        try {
            mapChangeEventManager!!?.onWillStartRenderingMap()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onWillStartRenderingMapListener)?.onWillStartRenderingMap()
        try {
            mapChangeEventManager!!?.onWillStartRenderingMap()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishRenderingMapListener() {
        mapChangeEventManager!!.addOnDidFinishRenderingMapListener(onDidFinishRenderingMapListener)
        mapChangeEventManager!!?.onDidFinishRenderingMap(true)
        Mockito.verify(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(true)
        mapChangeEventManager!!.removeOnDidFinishRenderingMapListener(
            onDidFinishRenderingMapListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingMap(true)
        Mockito.verify(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(true)
        mapChangeEventManager!!.addOnDidFinishRenderingMapListener(onDidFinishRenderingMapListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(true)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingMap(true)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(true)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingMap(true)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishRenderingMapFullyRenderedListener() {
        mapChangeEventManager!!.addOnDidFinishRenderingMapListener(onDidFinishRenderingMapListener)
        mapChangeEventManager!!?.onDidFinishRenderingMap(false)
        Mockito.verify(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(false)
        mapChangeEventManager!!.removeOnDidFinishRenderingMapListener(
            onDidFinishRenderingMapListener
        )
        mapChangeEventManager!!?.onDidFinishRenderingMap(false)
        Mockito.verify(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(false)
        mapChangeEventManager!!.addOnDidFinishRenderingMapListener(onDidFinishRenderingMapListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(false)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingMap(false)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishRenderingMapListener)?.onDidFinishRenderingMap(false)
        try {
            mapChangeEventManager!!?.onDidFinishRenderingMap(false)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidBecomeIdleListener() {
        mapChangeEventManager!!.addOnDidBecomeIdleListener(onDidBecomeIdleListener)
        mapChangeEventManager!!?.onDidBecomeIdle()
        Mockito.verify(onDidBecomeIdleListener)?.onDidBecomeIdle()
        mapChangeEventManager!!.removeOnDidBecomeIdleListener(onDidBecomeIdleListener)
        mapChangeEventManager!!?.onDidBecomeIdle()
        Mockito.verify(onDidBecomeIdleListener)?.onDidBecomeIdle()
        mapChangeEventManager!!.addOnDidBecomeIdleListener(onDidBecomeIdleListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidBecomeIdleListener)?.onDidBecomeIdle()
        try {
            mapChangeEventManager!!?.onDidBecomeIdle()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidBecomeIdleListener)?.onDidBecomeIdle()
        try {
            mapChangeEventManager!!?.onDidBecomeIdle()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnDidFinishLoadingStyleListener() {
        mapChangeEventManager!!.addOnDidFinishLoadingStyleListener(onDidFinishLoadingStyleListener)
        mapChangeEventManager!!?.onDidFinishLoadingStyle()
        Mockito.verify(onDidFinishLoadingStyleListener)?.onDidFinishLoadingStyle()
        mapChangeEventManager!!.removeOnDidFinishLoadingStyleListener(
            onDidFinishLoadingStyleListener
        )
        mapChangeEventManager!!?.onDidFinishLoadingStyle()
        Mockito.verify(onDidFinishLoadingStyleListener)?.onDidFinishLoadingStyle()
        mapChangeEventManager!!.addOnDidFinishLoadingStyleListener(onDidFinishLoadingStyleListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onDidFinishLoadingStyleListener)?.onDidFinishLoadingStyle()
        try {
            mapChangeEventManager!!?.onDidFinishLoadingStyle()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onDidFinishLoadingStyleListener)?.onDidFinishLoadingStyle()
        try {
            mapChangeEventManager!!?.onDidFinishLoadingStyle()
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    @Test
    fun testOnSourceChangedListener() {
        mapChangeEventManager!!.addOnSourceChangedListener(onSourceChangedListener)
        mapChangeEventManager!!?.onSourceChanged(TEST_STRING)
        Mockito.verify(onSourceChangedListener)?.onSourceChangedListener(TEST_STRING)
        mapChangeEventManager!!.removeOnSourceChangedListener(onSourceChangedListener)
        mapChangeEventManager!!?.onSourceChanged(TEST_STRING)
        Mockito.verify(onSourceChangedListener)?.onSourceChangedListener(TEST_STRING)
        mapChangeEventManager!!.addOnSourceChangedListener(onSourceChangedListener)
        Logger.setLoggerDefinition(loggerDefinition)
        val exc: Exception = RuntimeException()
        Mockito.doThrow(exc).`when`(onSourceChangedListener)?.onSourceChangedListener(TEST_STRING)
        try {
            mapChangeEventManager!!?.onSourceChanged(TEST_STRING)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: RuntimeException) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(exc)
            )
        }
        val err: Error = ExecutionError("", Error())
        Mockito.doThrow(err).`when`(onSourceChangedListener)?.onSourceChangedListener(TEST_STRING)
        try {
            mapChangeEventManager!!?.onSourceChanged(TEST_STRING)
            Assert.fail("The exception should've been re-thrown.")
        } catch (throwable: ExecutionError) {
            Mockito.verify(loggerDefinition)?.e(
                ArgumentMatchers.anyString(),
                ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(err)
            )
        }
    }

    companion object {
        private const val TEST_STRING = "mapChangeRandom"
        private val TEST_RENDERING_STATS = RenderingStats();
    }
}
