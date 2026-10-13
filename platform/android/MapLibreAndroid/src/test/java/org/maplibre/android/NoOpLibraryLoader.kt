package org.maplibre.android

import android.content.Context
import android.view.View
import org.maplibre.android.maps.renderer.MapRenderer

/** Loaded with the isolated SDK classes so its LibraryLoader superclass has the same identity. */
class NoOpLibraryLoader : LibraryLoader() {
    override fun load(name: String) = Unit
}

internal const val LIBRARY_LOAD_SENTINEL = "LibraryLoader constructor test sentinel"

class ThrowingLibraryLoader : LibraryLoader() {
    override fun load(name: String): Unit = throw AssertionError(LIBRARY_LOAD_SENTINEL)
}

/** Exercises MapRenderer's constructor without constructing an Android surface. */
class TestMapRenderer(
    context: Context,
) : MapRenderer(context, "") {
    override fun getView(): View? = null

    override fun setRenderingRefreshMode(mode: RenderingRefreshMode) = Unit

    override fun getRenderingRefreshMode(): RenderingRefreshMode = RenderingRefreshMode.WHEN_DIRTY

    override fun requestRender() = Unit

    override fun queueEvent(runnable: Runnable) = Unit

    override fun waitForEmpty() = Unit
}

/** Each case needs fresh class initializers and loader state, without changing other tests. */
internal class IsolatedMapLibreClassLoader(
    parent: ClassLoader,
) : ClassLoader(parent) {
    override fun loadClass(
        name: String,
        resolve: Boolean,
    ): Class<*> =
        synchronized(this) {
            val loaded =
                findLoadedClass(name) ?: if (name.startsWith("org.maplibre.android.")) {
                    findClass(name)
                } else {
                    super.loadClass(name, false)
                }
            if (resolve) resolveClass(loaded)
            loaded
        }

    override fun findClass(name: String): Class<*> {
        val resource = name.replace('.', '/') + ".class"
        val bytes =
            parent.getResourceAsStream(resource)?.use { it.readBytes() }
                ?: throw ClassNotFoundException(name)
        return defineClass(name, bytes, 0, bytes.size)
    }
}
