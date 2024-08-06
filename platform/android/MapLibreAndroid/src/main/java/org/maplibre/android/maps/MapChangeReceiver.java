package org.maplibre.android.maps;

import org.maplibre.android.log.Logger;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

class MapChangeReceiver implements NativeMapView.StateCallback {

  private static final String TAG = "Mbgl-MapChangeReceiver";

  private final List<MapView.OnCameraWillChangeListener> onCameraWillChangeListenerList = new CopyOnWriteArrayList<>();
  private final List<MapView.OnCameraIsChangingListener> onCameraIsChangingListenerList = new CopyOnWriteArrayList<>();
  private final List<MapView.OnCameraDidChangeListener> onCameraDidChangeListenerList = new CopyOnWriteArrayList<>();
  private final List<MapView.OnWillStartLoadingMapListener> onWillStartLoadingMapListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnDidFinishLoadingMapListener> onDidFinishLoadingMapListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnDidFailLoadingMapListener> onDidFailLoadingMapListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnWillStartRenderingFrameListener> onWillStartRenderingFrameList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnDidFinishRenderingFrameListener> onDidFinishRenderingFrameList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnWillStartRenderingMapListener> onWillStartRenderingMapListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnDidFinishRenderingMapListener> onDidFinishRenderingMapListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnDidBecomeIdleListener> onDidBecomeIdleListenerList
      = new CopyOnWriteArrayList<>();
  private final List<MapView.OnDidFinishLoadingStyleListener> onDidFinishLoadingStyleListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnSourceChangedListener> onSourceChangedListenerList = new CopyOnWriteArrayList<>();
  private final List<MapView.OnStyleImageMissingListener> onStyleImageMissingListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.OnCanRemoveUnusedStyleImageListener> onCanRemoveUnusedStyleImageListenerList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onPreCompileShaderListener> onPreCompileShaderList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onPostCompileShaderListener> onPostCompileShaderList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onShaderCompileFailedListener> onShaderCompileFailedList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onGlyphsLoadedListener> onGlyphsLoadedList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onGlyphsErrorListener> onGlyphsErrorList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onGlyphsRequestedListener> onGlyphsRequestedList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onTileRequestedListener> onTileRequestedList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onTileLoadedFromNetworkListener> onTileLoadedFromNetworkList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onTileLoadedFromDiskListener> onTileLoadedFromDiskList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onTileFailedToLoadListener> onTileFailedToLoadList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onTileFinishedLoadingListener> onTileFinishedLoadingList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onSpriteLoadedListener> onSpriteLoadedList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onSpriteErrorListener> onSpriteErrorList
    = new CopyOnWriteArrayList<>();
  private final List<MapView.onSpriteRequestedListener> onSpriteRequestedList
    = new CopyOnWriteArrayList<>();

  @Override
  public void onCameraWillChange(boolean animated) {
    try {
      if (!onCameraWillChangeListenerList.isEmpty()) {
        for (MapView.OnCameraWillChangeListener onCameraWillChangeListener : onCameraWillChangeListenerList) {
          onCameraWillChangeListener.onCameraWillChange(animated);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onCameraWillChange", err);
      throw err;
    }
  }

  @Override
  public void onCameraIsChanging() {
    try {
      if (!onCameraIsChangingListenerList.isEmpty()) {
        for (MapView.OnCameraIsChangingListener onCameraIsChangingListener : onCameraIsChangingListenerList) {
          onCameraIsChangingListener.onCameraIsChanging();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onCameraIsChanging", err);
      throw err;
    }
  }

  @Override
  public void onCameraDidChange(boolean animated) {
    try {
      if (!onCameraDidChangeListenerList.isEmpty()) {
        for (MapView.OnCameraDidChangeListener onCameraDidChangeListener : onCameraDidChangeListenerList) {
          onCameraDidChangeListener.onCameraDidChange(animated);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onCameraDidChange", err);
      throw err;
    }
  }

  @Override
  public void onWillStartLoadingMap() {
    try {
      if (!onWillStartLoadingMapListenerList.isEmpty()) {
        for (MapView.OnWillStartLoadingMapListener onWillStartLoadingMapListener : onWillStartLoadingMapListenerList) {
          onWillStartLoadingMapListener.onWillStartLoadingMap();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onWillStartLoadingMap", err);
      throw err;
    }
  }

  @Override
  public void onDidFinishLoadingMap() {
    try {
      if (!onDidFinishLoadingMapListenerList.isEmpty()) {
        for (MapView.OnDidFinishLoadingMapListener onDidFinishLoadingMapListener : onDidFinishLoadingMapListenerList) {
          onDidFinishLoadingMapListener.onDidFinishLoadingMap();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onDidFinishLoadingMap", err);
      throw err;
    }
  }

  @Override
  public void onDidFailLoadingMap(String error) {
    try {
      if (!onDidFailLoadingMapListenerList.isEmpty()) {
        for (MapView.OnDidFailLoadingMapListener onDidFailLoadingMapListener : onDidFailLoadingMapListenerList) {
          onDidFailLoadingMapListener.onDidFailLoadingMap(error);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onDidFailLoadingMap", err);
      throw err;
    }
  }

  @Override
  public void onWillStartRenderingFrame() {
    try {
      if (!onWillStartRenderingFrameList.isEmpty()) {
        for (MapView.OnWillStartRenderingFrameListener listener : onWillStartRenderingFrameList) {
          listener.onWillStartRenderingFrame();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onWillStartRenderingFrame", err);
      throw err;
    }
  }

  @Override
  public void onDidFinishRenderingFrame(boolean fully, double frameEncodingTime, double frameRenderingTime) {
    try {
      if (!onDidFinishRenderingFrameList.isEmpty()) {
        for (MapView.OnDidFinishRenderingFrameListener listener : onDidFinishRenderingFrameList) {
          listener.onDidFinishRenderingFrame(fully, frameEncodingTime, frameRenderingTime);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onDidFinishRenderingFrame", err);
      throw err;
    }
  }

  @Override
  public void onWillStartRenderingMap() {
    try {
      if (!onWillStartRenderingMapListenerList.isEmpty()) {
        for (MapView.OnWillStartRenderingMapListener listener : onWillStartRenderingMapListenerList) {
          listener.onWillStartRenderingMap();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onWillStartRenderingMap", err);
      throw err;
    }
  }

  @Override
  public void onDidFinishRenderingMap(boolean fully) {
    try {
      if (!onDidFinishRenderingMapListenerList.isEmpty()) {
        for (MapView.OnDidFinishRenderingMapListener listener : onDidFinishRenderingMapListenerList) {
          listener.onDidFinishRenderingMap(fully);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onDidFinishRenderingMap", err);
      throw err;
    }
  }

  @Override
  public void onDidBecomeIdle() {
    try {
      if (!onDidBecomeIdleListenerList.isEmpty()) {
        for (MapView.OnDidBecomeIdleListener listener : onDidBecomeIdleListenerList) {
          listener.onDidBecomeIdle();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onDidBecomeIdle", err);
      throw err;
    }
  }

  @Override
  public void onDidFinishLoadingStyle() {
    try {
      if (!onDidFinishLoadingStyleListenerList.isEmpty()) {
        for (MapView.OnDidFinishLoadingStyleListener listener : onDidFinishLoadingStyleListenerList) {
          listener.onDidFinishLoadingStyle();
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onDidFinishLoadingStyle", err);
      throw err;
    }
  }

  @Override
  public void onSourceChanged(String sourceId) {
    try {
      if (!onSourceChangedListenerList.isEmpty()) {
        for (MapView.OnSourceChangedListener onSourceChangedListener : onSourceChangedListenerList) {
          onSourceChangedListener.onSourceChangedListener(sourceId);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onSourceChanged", err);
      throw err;
    }
  }

  @Override
  public void onStyleImageMissing(String imageId) {
    try {
      if (!onStyleImageMissingListenerList.isEmpty()) {
        for (MapView.OnStyleImageMissingListener listener : onStyleImageMissingListenerList) {
          listener.onStyleImageMissing(imageId);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onStyleImageMissing", err);
      throw err;
    }
  }

  @Override
  public boolean onCanRemoveUnusedStyleImage(String imageId) {
    if (onCanRemoveUnusedStyleImageListenerList.isEmpty()) {
      return true;
    }

    try {
      if (!onCanRemoveUnusedStyleImageListenerList.isEmpty()) {
        boolean canRemove = true;
        for (MapView.OnCanRemoveUnusedStyleImageListener listener : onCanRemoveUnusedStyleImageListenerList) {
          canRemove &= listener.onCanRemoveUnusedStyleImage(imageId);
        }

        return canRemove;
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onCanRemoveUnusedStyleImage", err);
      throw err;
    }

    return true;
  }

  @Override
  public void onPreCompileShader(int id, int type) {
    try {
      if (!onPreCompileShaderList.isEmpty()) {
        for (MapView.onPreCompileShaderListener listener : onPreCompileShaderList) {
          listener.onPreCompileShader(id, type);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onPreCompileShader", err);
      throw err;
    }
  }

  @Override
  public void onPostCompileShader(int id, int type) {
    try {
      if (!onPostCompileShaderList.isEmpty()) {
        for (MapView.onPostCompileShaderListener listener : onPostCompileShaderList) {
          listener.onPostCompileShader(id, type);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onPostCompileShader", err);
      throw err;
    }
  }

  @Override
  public void onShaderCompileFailed(int id, int type) {
    try {
      if (!onShaderCompileFailedList.isEmpty()) {
        for (MapView.onShaderCompileFailedListener listener : onShaderCompileFailedList) {
          listener.onShaderCompileFailed(id, type);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onShaderCompileFailed", err);
      throw err;
    }
  }

  @Override
  public void onGlyphsLoaded(List<String> stack, int rangeStart, int rangeEnd) {
    try {
      if (!onGlyphsLoadedList.isEmpty()) {
        for (MapView.onGlyphsLoadedListener listener : onGlyphsLoadedList) {
          listener.onGlyphsLoaded(stack, rangeStart, rangeEnd);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onGlyphsLoaded", err);
      throw err;
    }
  }

  @Override
  public void onGlyphsError(List<String> stack, int rangeStart, int rangeEnd) {
    try {
      if (!onGlyphsErrorList.isEmpty()) {
        for (MapView.onGlyphsErrorListener listener : onGlyphsErrorList) {
          listener.onGlyphsError(stack, rangeStart, rangeEnd);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onGlyphsError", err);
      throw err;
    }
  }

  @Override
  public void onGlyphsRequested(List<String> stack, int rangeStart, int rangeEnd) {
    try {
      if (!onGlyphsRequestedList.isEmpty()) {
        for (MapView.onGlyphsRequestedListener listener : onGlyphsRequestedList) {
          listener.onGlyphsRequested(stack, rangeStart, rangeEnd);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onGlyphsRequested", err);
      throw err;
    }
  }

  @Override
  public void onTileRequested(int x, int y, int z, int overscaledZ) {
    try {
      if (!onTileRequestedList.isEmpty()) {
        for (MapView.onTileRequestedListener listener : onTileRequestedList) {
          listener.onTileRequested(x, y, z, overscaledZ);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onTileRequested", err);
      throw err;
    }
  }

  @Override
  public void onTileLoadedFromNetwork(int x, int y, int z, int overscaledZ) {
    try {
      if (!onTileLoadedFromNetworkList.isEmpty()) {
        for (MapView.onTileLoadedFromNetworkListener listener : onTileLoadedFromNetworkList) {
          listener.onTileLoadedFromNetwork(x, y, z, overscaledZ);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onTileLoadedFromNetwork", err);
      throw err;
    }
  }

  @Override
  public void onTileLoadedFromDisk(int x, int y, int z, int overscaledZ) {
    try {
      if (!onTileLoadedFromDiskList.isEmpty()) {
        for (MapView.onTileLoadedFromDiskListener listener : onTileLoadedFromDiskList) {
          listener.onTileLoadedFromDisk(x, y, z, overscaledZ);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onTileLoadedFromDisk", err);
      throw err;
    }
  }

  @Override
  public void onTileFailedToLoad(int x, int y, int z, int overscaledZ) {
    try {
      if (!onTileFailedToLoadList.isEmpty()) {
        for (MapView.onTileFailedToLoadListener listener : onTileFailedToLoadList) {
          listener.onTileFailedToLoad(x, y, z, overscaledZ);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onTileFailedToLoad", err);
      throw err;
    }
  }

  @Override
  public void onTileFinishedLoading(int x, int y, int z, int overscaledZ) {
    try {
      if (!onTileFinishedLoadingList.isEmpty()) {
        for (MapView.onTileFinishedLoadingListener listener : onTileFinishedLoadingList) {
          listener.onTileFinishedLoading(x, y, z, overscaledZ);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onTileFinishedLoading", err);
      throw err;
    }
  }

  @Override
  public void onSpriteLoaded(String id, String url) {
    try {
      if (!onSpriteLoadedList.isEmpty()) {
        for (MapView.onSpriteLoadedListener listener : onSpriteLoadedList) {
          listener.onSpriteLoaded(id, url);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onSpriteLoaded", err);
      throw err;
    }
  }

  @Override
  public void onSpriteError(String id, String url) {
    try {
      if (!onSpriteErrorList.isEmpty()) {
        for (MapView.onSpriteErrorListener listener : onSpriteErrorList) {
          listener.onSpriteError(id, url);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onSpriteError", err);
      throw err;
    }
  }

  @Override
  public void onSpriteRequested(String id, String url) {
    try {
      if (!onSpriteRequestedList.isEmpty()) {
        for (MapView.onSpriteRequestedListener listener : onSpriteRequestedList) {
          listener.onSpriteRequested(id, url);
        }
      }
    } catch (Throwable err) {
      Logger.e(TAG, "Exception in onSpriteRequested", err);
      throw err;
    }
  }

  void addOnCameraWillChangeListener(MapView.OnCameraWillChangeListener listener) {
    onCameraWillChangeListenerList.add(listener);
  }

  void removeOnCameraWillChangeListener(MapView.OnCameraWillChangeListener listener) {
    onCameraWillChangeListenerList.remove(listener);
  }

  void addOnCameraIsChangingListener(MapView.OnCameraIsChangingListener listener) {
    onCameraIsChangingListenerList.add(listener);
  }

  void removeOnCameraIsChangingListener(MapView.OnCameraIsChangingListener listener) {
    onCameraIsChangingListenerList.remove(listener);
  }

  void addOnCameraDidChangeListener(MapView.OnCameraDidChangeListener listener) {
    onCameraDidChangeListenerList.add(listener);
  }

  void removeOnCameraDidChangeListener(MapView.OnCameraDidChangeListener listener) {
    onCameraDidChangeListenerList.remove(listener);
  }

  void addOnWillStartLoadingMapListener(MapView.OnWillStartLoadingMapListener listener) {
    onWillStartLoadingMapListenerList.add(listener);
  }

  void removeOnWillStartLoadingMapListener(MapView.OnWillStartLoadingMapListener listener) {
    onWillStartLoadingMapListenerList.remove(listener);
  }

  void addOnDidFinishLoadingMapListener(MapView.OnDidFinishLoadingMapListener listener) {
    onDidFinishLoadingMapListenerList.add(listener);
  }

  void removeOnDidFinishLoadingMapListener(MapView.OnDidFinishLoadingMapListener listener) {
    onDidFinishLoadingMapListenerList.remove(listener);
  }

  void addOnDidFailLoadingMapListener(MapView.OnDidFailLoadingMapListener listener) {
    onDidFailLoadingMapListenerList.add(listener);
  }

  void removeOnDidFailLoadingMapListener(MapView.OnDidFailLoadingMapListener listener) {
    onDidFailLoadingMapListenerList.remove(listener);
  }

  void addOnWillStartRenderingFrameListener(MapView.OnWillStartRenderingFrameListener listener) {
    onWillStartRenderingFrameList.add(listener);
  }

  void removeOnWillStartRenderingFrameListener(MapView.OnWillStartRenderingFrameListener listener) {
    onWillStartRenderingFrameList.remove(listener);
  }

  void addOnDidFinishRenderingFrameListener(MapView.OnDidFinishRenderingFrameListener listener) {
    onDidFinishRenderingFrameList.add(listener);
  }

  void removeOnDidFinishRenderingFrameListener(MapView.OnDidFinishRenderingFrameListener listener) {
    onDidFinishRenderingFrameList.remove(listener);
  }

  void addOnWillStartRenderingMapListener(MapView.OnWillStartRenderingMapListener listener) {
    onWillStartRenderingMapListenerList.add(listener);
  }

  void removeOnWillStartRenderingMapListener(MapView.OnWillStartRenderingMapListener listener) {
    onWillStartRenderingMapListenerList.remove(listener);
  }

  void addOnDidFinishRenderingMapListener(MapView.OnDidFinishRenderingMapListener listener) {
    onDidFinishRenderingMapListenerList.add(listener);
  }

  void removeOnDidFinishRenderingMapListener(MapView.OnDidFinishRenderingMapListener listener) {
    onDidFinishRenderingMapListenerList.remove(listener);
  }

  void addOnDidBecomeIdleListener(MapView.OnDidBecomeIdleListener listener) {
    onDidBecomeIdleListenerList.add(listener);
  }

  void removeOnDidBecomeIdleListener(MapView.OnDidBecomeIdleListener listener) {
    onDidBecomeIdleListenerList.remove(listener);
  }

  void addOnDidFinishLoadingStyleListener(MapView.OnDidFinishLoadingStyleListener listener) {
    onDidFinishLoadingStyleListenerList.add(listener);
  }

  void removeOnDidFinishLoadingStyleListener(MapView.OnDidFinishLoadingStyleListener listener) {
    onDidFinishLoadingStyleListenerList.remove(listener);
  }

  void addOnSourceChangedListener(MapView.OnSourceChangedListener listener) {
    onSourceChangedListenerList.add(listener);
  }

  void removeOnSourceChangedListener(MapView.OnSourceChangedListener listener) {
    onSourceChangedListenerList.remove(listener);
  }

  void addOnStyleImageMissingListener(MapView.OnStyleImageMissingListener listener) {
    onStyleImageMissingListenerList.add(listener);
  }

  void removeOnStyleImageMissingListener(MapView.OnStyleImageMissingListener listener) {
    onStyleImageMissingListenerList.remove(listener);
  }

  void addOnCanRemoveUnusedStyleImageListener(MapView.OnCanRemoveUnusedStyleImageListener listener) {
    onCanRemoveUnusedStyleImageListenerList.add(listener);
  }

  void removeOnCanRemoveUnusedStyleImageListener(MapView.OnCanRemoveUnusedStyleImageListener listener) {
    onCanRemoveUnusedStyleImageListenerList.remove(listener);
  }

  public void addOnPreCompileShaderListener(MapView.onPreCompileShaderListener callback) {
    onPreCompileShaderList.add(callback);
  }

  public void addOnPostCompileShaderListener(MapView.onPostCompileShaderListener callback) {
    onPostCompileShaderList.add(callback);
  }

  public void addOnShaderCompileFailedListener(MapView.onShaderCompileFailedListener callback) {
    onShaderCompileFailedList.add(callback);
  }

  public void addOnGlyphsLoadedListener(MapView.onGlyphsLoadedListener callback) {
    onGlyphsLoadedList.add(callback);
  }

  public void addOnGlyphsErrorListener(MapView.onGlyphsErrorListener callback) {
    onGlyphsErrorList.add(callback);
  }

  public void addOnGlyphsRequestedListener(MapView.onGlyphsRequestedListener callback) {
    onGlyphsRequestedList.add(callback);
  }

  public void addOnTileRequestedListener(MapView.onTileRequestedListener callback) {
    onTileRequestedList.add(callback);
  }

  public void addOnTileLoadedFromNetworkListener(MapView.onTileLoadedFromNetworkListener callback) {
    onTileLoadedFromNetworkList.add(callback);
  }

  public void addOnTileLoadedFromDiskListener(MapView.onTileLoadedFromDiskListener callback) {
    onTileLoadedFromDiskList.add(callback);
  }

  public void addOnTileFailedToLoadListener(MapView.onTileFailedToLoadListener callback) {
    onTileFailedToLoadList.add(callback);
  }

  public void addOnTileFinishedLoadingListener(MapView.onTileFinishedLoadingListener callback) {
    onTileFinishedLoadingList.add(callback);
  }

  public void addOnSpriteLoadedListener(MapView.onSpriteLoadedListener callback) {
    onSpriteLoadedList.add(callback);
  }

  public void addOnSpriteErrorListener(MapView.onSpriteErrorListener callback) {
    onSpriteErrorList.add(callback);
  }

  public void addOnSpriteRequestedListener(MapView.onSpriteRequestedListener callback) {
    onSpriteRequestedList.add(callback);
  }

  public void removeOnPreCompileShaderListener(MapView.onPreCompileShaderListener callback) {
    onPreCompileShaderList.remove(callback);
  }

  public void removeOnPostCompileShaderListener(MapView.onPostCompileShaderListener callback) {
    onPostCompileShaderList.remove(callback);
  }

  public void removeOnShaderCompileFailedListener(MapView.onShaderCompileFailedListener callback) {
    onShaderCompileFailedList.remove(callback);
  }

  public void removeOnGlyphsLoadedListener(MapView.onGlyphsLoadedListener callback) {
    onGlyphsLoadedList.remove(callback);
  }

  public void removeOnGlyphsErrorListener(MapView.onGlyphsErrorListener callback) {
    onGlyphsErrorList.remove(callback);
  }

  public void removeOnGlyphsRequestedListener(MapView.onGlyphsRequestedListener callback) {
    onGlyphsRequestedList.remove(callback);
  }

  public void removeOnTileRequestedListener(MapView.onTileRequestedListener callback) {
    onTileRequestedList.remove(callback);
  }

  public void removeOnTileLoadedFromNetworkListener(MapView.onTileLoadedFromNetworkListener callback) {
    onTileLoadedFromNetworkList.remove(callback);
  }

  public void removeOnTileLoadedFromDiskListener(MapView.onTileLoadedFromDiskListener callback) {
    onTileLoadedFromDiskList.remove(callback);
  }

  public void removeOnTileFailedToLoadListener(MapView.onTileFailedToLoadListener callback) {
    onTileFailedToLoadList.remove(callback);
  }

  public void removeOnTileFinishedLoadingListener(MapView.onTileFinishedLoadingListener callback) {
    onTileFinishedLoadingList.remove(callback);
  }

  public void removeOnSpriteLoadedListener(MapView.onSpriteLoadedListener callback) {
    onSpriteLoadedList.remove(callback);
  }

  public void removeOnSpriteErrorListener(MapView.onSpriteErrorListener callback) {
    onSpriteErrorList.remove(callback);
  }
  
  public void removeOnSpriteRequestedListener(MapView.onSpriteRequestedListener callback) {
    onSpriteRequestedList.remove(callback);
  }

  void clear() {
    onCameraWillChangeListenerList.clear();
    onCameraIsChangingListenerList.clear();
    onCameraDidChangeListenerList.clear();
    onWillStartLoadingMapListenerList.clear();
    onDidFinishLoadingMapListenerList.clear();
    onDidFailLoadingMapListenerList.clear();
    onWillStartRenderingFrameList.clear();
    onDidFinishRenderingFrameList.clear();
    onWillStartRenderingMapListenerList.clear();
    onDidFinishRenderingMapListenerList.clear();
    onDidBecomeIdleListenerList.clear();
    onDidFinishLoadingStyleListenerList.clear();
    onSourceChangedListenerList.clear();
    onStyleImageMissingListenerList.clear();
    onCanRemoveUnusedStyleImageListenerList.clear();
    onPreCompileShaderList.clear();
    onPostCompileShaderList.clear();
    onShaderCompileFailedList.clear();
    onGlyphsLoadedList.clear();
    onGlyphsErrorList.clear();
    onGlyphsRequestedList.clear();
    onTileRequestedList.clear();
    onTileLoadedFromNetworkList.clear();
    onTileLoadedFromDiskList.clear();
    onTileFailedToLoadList.clear();
    onTileFinishedLoadingList.clear();
    onSpriteLoadedList.clear();
    onSpriteErrorList.clear();
    onSpriteRequestedList.clear();
  }
}