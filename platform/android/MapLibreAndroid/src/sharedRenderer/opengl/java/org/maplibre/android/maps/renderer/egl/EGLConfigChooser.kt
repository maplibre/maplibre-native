package org.maplibre.android.maps.renderer.egl

import android.opengl.GLSurfaceView
import android.os.Build
import org.maplibre.android.constants.MapLibreConstants
import org.maplibre.android.log.Logger
import javax.microedition.khronos.egl.EGL10
import javax.microedition.khronos.egl.EGL10.EGL_ALPHA_MASK_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_ALPHA_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_BLUE_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_BUFFER_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_COLOR_BUFFER_TYPE
import javax.microedition.khronos.egl.EGL10.EGL_CONFIG_CAVEAT
import javax.microedition.khronos.egl.EGL10.EGL_DEPTH_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_GREEN_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_NONE
import javax.microedition.khronos.egl.EGL10.EGL_RED_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_RENDERABLE_TYPE
import javax.microedition.khronos.egl.EGL10.EGL_RGB_BUFFER
import javax.microedition.khronos.egl.EGL10.EGL_SAMPLES
import javax.microedition.khronos.egl.EGL10.EGL_SAMPLE_BUFFERS
import javax.microedition.khronos.egl.EGL10.EGL_STENCIL_SIZE
import javax.microedition.khronos.egl.EGL10.EGL_SURFACE_TYPE
import javax.microedition.khronos.egl.EGL10.EGL_WINDOW_BIT
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.egl.EGLDisplay

/**
 * Selects the right EGLConfig needed for `mapbox-gl-native`
 */
class EGLConfigChooser
    @JvmOverloads
    constructor(
        private val translucentSurface: Boolean = false,
    ) : GLSurfaceView.EGLConfigChooser {
        override fun chooseConfig(
            egl: EGL10,
            display: EGLDisplay,
        ): EGLConfig? {
            val configAttribs = getConfigAttributes()

            // Determine number of possible configurations
            val numConfigs = getNumberOfConfigurations(egl, display, configAttribs)
            if (numConfigs[0] < 1) {
                Logger.e(TAG, "eglChooseConfig() returned no configs.")
            }

            // Get all possible configurations
            val possibleConfigurations = getPossibleConfigurations(egl, display, configAttribs, numConfigs)

            // Choose best match
            val config = chooseBestMatchConfig(egl, display, possibleConfigurations)
            if (config == null) {
                Logger.e(TAG, "No config chosen")
            }

            return config
        }

        private fun getNumberOfConfigurations(
            egl: EGL10,
            display: EGLDisplay,
            configAttributes: IntArray,
        ): IntArray {
            val numConfigs = IntArray(1)
            if (!egl.eglChooseConfig(display, configAttributes, null, 0, numConfigs)) {
                Logger.e(
                    TAG,
                    String.format(
                        MapLibreConstants.MAPLIBRE_LOCALE,
                        "eglChooseConfig(NULL) returned error %d",
                        egl.eglGetError(),
                    ),
                )
            }
            return numConfigs
        }

        private fun getPossibleConfigurations(
            egl: EGL10,
            display: EGLDisplay,
            configAttributes: IntArray,
            numConfigs: IntArray,
        ): Array<EGLConfig?> {
            val configs = arrayOfNulls<EGLConfig>(numConfigs[0])
            if (!egl.eglChooseConfig(display, configAttributes, configs, numConfigs[0], numConfigs)) {
                Logger.e(
                    TAG,
                    String.format(
                        MapLibreConstants.MAPLIBRE_LOCALE,
                        "eglChooseConfig() returned error %d",
                        egl.eglGetError(),
                    ),
                )
            }
            return configs
        }

        // Quality
        internal enum class BufferFormat(
            val value: Int,
        ) {
            Format16Bit(3),
            Format32BitNoAlpha(1),
            Format32BitAlpha(2),
            Format24Bit(0),
            Unknown(4),
        }

        internal enum class DepthStencilFormat(
            val value: Int,
        ) {
            Format16Depth8Stencil(1),
            Format24Depth8Stencil(0),
        }

        private class Config(
            val bufferFormat: BufferFormat,
            val depthStencilFormat: DepthStencilFormat,
            val isCaveat: Boolean,
            val index: Int,
            val config: EGLConfig,
        ) : Comparable<Config> {
            override fun compareTo(other: Config): Int =
                compareValuesBy(
                    this,
                    other,
                    { it.bufferFormat.value },
                    { it.depthStencilFormat.value },
                    { it.isCaveat },
                    { it.index },
                )
        }

        @Suppress("CyclomaticComplexMethod", "ComplexMethod")
        private fun chooseBestMatchConfig(
            egl: EGL10,
            display: EGLDisplay,
            configs: Array<EGLConfig?>,
        ): EGLConfig? {
            val matches = mutableListOf<Config>()

            var i = 0
            for (config in configs) {
                if (config == null) {
                    continue
                }

                i++

                val caveat = getConfigAttr(egl, display, config, EGL_CONFIG_CAVEAT)
                val bits = getConfigAttr(egl, display, config, EGL_BUFFER_SIZE)
                val red = getConfigAttr(egl, display, config, EGL_RED_SIZE)
                val green = getConfigAttr(egl, display, config, EGL_GREEN_SIZE)
                val blue = getConfigAttr(egl, display, config, EGL_BLUE_SIZE)
                val alpha = getConfigAttr(egl, display, config, EGL_ALPHA_SIZE)

                @Suppress("UNUSED_VARIABLE")
                val alphaMask = getConfigAttr(egl, display, config, EGL_ALPHA_MASK_SIZE)
                val depth = getConfigAttr(egl, display, config, EGL_DEPTH_SIZE)
                val stencil = getConfigAttr(egl, display, config, EGL_STENCIL_SIZE)
                val sampleBuffers = getConfigAttr(egl, display, config, EGL_SAMPLE_BUFFERS)
                val samples = getConfigAttr(egl, display, config, EGL_SAMPLES)

                var configOk = (depth == 24) || (depth == 16)
                configOk = configOk and (stencil == 8)
                configOk = configOk and (sampleBuffers == 0)
                configOk = configOk and (samples == 0)

                // Filter our configs first for depth, stencil and anti-aliasing
                if (configOk) {
                    // Work out the config's buffer format
                    val bufferFormat =
                        if ((bits == 16) && (red == 5) && (green == 6) && (blue == 5) && (alpha == 0)) {
                            BufferFormat.Format16Bit
                        } else if ((bits == 32) && (red == 8) && (green == 8) && (blue == 8) && (alpha == 0)) {
                            BufferFormat.Format32BitNoAlpha
                        } else if ((bits == 32) && (red == 8) && (green == 8) && (blue == 8) && (alpha == 8)) {
                            BufferFormat.Format32BitAlpha
                        } else if ((bits == 24) && (red == 8) && (green == 8) && (blue == 8) && (alpha == 0)) {
                            BufferFormat.Format24Bit
                        } else {
                            BufferFormat.Unknown
                        }

                    // Work out the config's depth stencil format
                    val depthStencilFormat =
                        if ((depth == 16) && (stencil == 8)) {
                            DepthStencilFormat.Format16Depth8Stencil
                        } else {
                            DepthStencilFormat.Format24Depth8Stencil
                        }

                    val isCaveat = caveat != EGL_NONE

                    // Ignore formats we don't recognise
                    if (bufferFormat != BufferFormat.Unknown) {
                        matches.add(Config(bufferFormat, depthStencilFormat, isCaveat, i, config))
                    }
                }
            }

            // Sort
            matches.sort()

            if (matches.isEmpty()) {
                Logger.e(TAG, "No matching configurations after filtering")
                return null
            }

            val bestMatch = matches[0]

            if (bestMatch.isCaveat) {
                Logger.w(TAG, "Chosen config has a caveat.")
            }

            return bestMatch.config
        }

        private fun getConfigAttr(
            egl: EGL10,
            display: EGLDisplay,
            config: EGLConfig,
            attributeName: Int,
        ): Int {
            val attributeValue = IntArray(1)
            if (!egl.eglGetConfigAttrib(display, config, attributeName, attributeValue)) {
                Logger.e(
                    TAG,
                    String.format(
                        MapLibreConstants.MAPLIBRE_LOCALE,
                        "eglGetConfigAttrib(%d) returned error %d",
                        attributeName,
                        egl.eglGetError(),
                    ),
                )
            }
            return attributeValue[0]
        }

        private fun getConfigAttributes(): IntArray {
            val emulator = inEmulator() || inGenymotion()
            Logger.i(TAG, "In emulator: $emulator")

            // Get all configs at least RGB 565 with 16 depth and 8 stencil
            return intArrayOf(
                EGL_CONFIG_CAVEAT,
                EGL_NONE,
                EGL_SURFACE_TYPE,
                EGL_WINDOW_BIT,
                EGL_BUFFER_SIZE,
                16,
                EGL_RED_SIZE,
                5,
                EGL_GREEN_SIZE,
                6,
                EGL_BLUE_SIZE,
                5,
                EGL_ALPHA_SIZE,
                if (translucentSurface) 8 else 0,
                EGL_DEPTH_SIZE,
                16,
                EGL_STENCIL_SIZE,
                8,
                if (emulator) EGL_NONE else EGL_COLOR_BUFFER_TYPE,
                EGL_RGB_BUFFER,
                EGL_RENDERABLE_TYPE,
                EGL_OPENGL_ES3_BIT,
                EGL_NONE,
            )
        }

        /**
         * Detect if we are in emulator.
         */
        private fun inEmulator(): Boolean =
            Build.FINGERPRINT.startsWith("generic") ||
                Build.FINGERPRINT.startsWith("unknown") ||
                Build.MODEL.contains("google_sdk") ||
                Build.MODEL.contains("Emulator") ||
                Build.MODEL.contains("Android SDK built for x86") ||
                (Build.BRAND.startsWith("generic") && Build.DEVICE.startsWith("generic")) ||
                Build.PRODUCT == "google_sdk" ||
                System.getProperty("ro.kernel.qemu") != null

        /**
         * Detect if we are in genymotion
         */
        private fun inGenymotion(): Boolean = Build.MANUFACTURER.contains("Genymotion")

        private companion object {
            private const val TAG = "Mbgl-EGLConfigChooser"

            /**
             * Requires API level 18
             *
             * @see android.opengl.EGL15.EGL_OPENGL_ES3_BIT
             */
            private const val EGL_OPENGL_ES3_BIT = 0x0040
        }
    }
