package org.maplibre.android.testapp.model.customlayer

import androidx.annotation.Keep

@Keep
object ExampleCustomLayer {
    init {
        System.loadLibrary("example-custom-layer")
    }

    external fun createContext(): Long
    external fun setColor(red: Float, green: Float, blue: Float, alpha: Float)
    var InitializeFunction: Long = 0
    var RenderFunction: Long = 0
    var ContextLostFunction: Long = 0
    var DeinitializeFunction: Long = 0
}
