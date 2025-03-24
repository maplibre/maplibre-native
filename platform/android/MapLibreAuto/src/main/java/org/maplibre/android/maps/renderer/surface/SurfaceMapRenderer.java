package org.maplibre.android.maps.renderer.surface;

import android.content.Context;
import android.util.Log;
import android.view.Surface;
import android.view.View;

import androidx.annotation.NonNull;

import com.grab.mapsdk.common.log.Logger;

import org.maplibre.android.maps.renderer.MapRenderer;


public class SurfaceMapRenderer extends MapRenderer {

    private static final String TAG = "SurfaceMapRenderer";

    private SurfaceRenderThread renderThread;

    public SurfaceMapRenderer(@NonNull Context context) {
        super(context, null);
        renderThread = new SurfaceRenderThread(this);
        renderThread.start();
    }

    public void onSurfaceCreated(Surface surface, int width, int height) {
        Logger.d(TAG, "onSurfaceCreated w:" + width + " h:" + height);
        onSurfaceCreated(surface);
        renderThread.onSurfaceAvailable(surface, width, height);
    }

    /**
     * Overridden to provide package access
     */
    @Override
    public void onSurfaceCreated(Surface surface) {
        super.onSurfaceCreated(surface);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        Log.d(TAG, "onSurfaceChanged w:" + width + " h:" + height);
        renderThread.onSurfaceSizeChanged(width, height);
        super.onSurfaceChanged(width, height);
    }

    /**
     * Overridden to provide package access
     */
    @Override
    public void onSurfaceDestroyed() {
        renderThread.onSurfaceDestroyed();
        super.onSurfaceDestroyed();
    }

    /**
     * Overridden to provide package access
     */
    @Override
    public void onDrawFrame() {
        super.onDrawFrame();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void requestRender() {
        renderThread.requestRender();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void queueEvent(Runnable runnable) {
        renderThread.queueEvent(runnable);
    }

    @Override
    public void waitForEmpty() {
        renderThread.waitForEmpty();
    }

    /**
     * {@inheritDoc}
     */
    public void onStop() {
        renderThread.onPause();
    }

    @Override
    public View getView() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    public void onStart() {
        renderThread.onResume();
    }

    /**
     * {@inheritDoc}
     */
    public void onDestroy() {
        renderThread.onDestroy();
    }

    @Override
    public void setRenderingRefreshMode(RenderingRefreshMode renderingRefreshMode) {

    }

    @Override
    public RenderingRefreshMode getRenderingRefreshMode() {
        return null;
    }

}
