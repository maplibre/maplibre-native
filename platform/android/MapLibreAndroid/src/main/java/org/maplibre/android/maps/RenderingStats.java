package org.maplibre.android.maps;

import androidx.annotation.NonNull;

import java.util.List;
import java.util.Map;
import java.util.Set;

public class RenderingStats {
  /// Frame CPU encoding time (seconds)
  public double encodingTime = 0.0;
  /// Frame CPU rendering time (seconds)
  public double renderingTime = 0.0;

  /// Number of frames rendered
  public int numFrames = 0;
  /// Number of draw calls (`glDrawElements`, `drawIndexedPrimitives`, etc.) executed during the most recent frame
  public int numDrawCalls = 0;
  /// Total number of draw calls executed during all the frames
  public int totalDrawCalls = 0;

  /// Total number of textures created
  public int numCreatedTextures = 0;
  /// Net textures
  public int numActiveTextures = 0;
  /// Net texture bindings
  public int numTextureBindings = 0;
  /// Number of times a texture was updated
  public int numTextureUpdates = 0;
  /// Number of bytes used in texture updates
  public long textureUpdateBytes = 0;

  /// Number of buffers created
  public long totalBuffers = 0;
  /// Number of SDK-specific buffers created
  public long totalBufferObjs = 0;
  /// Number of times a buffer is updated
  public long bufferUpdates = 0;
  /// Number of times an SDK-specific buffer is updated
  public long bufferObjUpdates = 0;
  /// Sum of update sizes
  public long bufferUpdateBytes = 0;

  /// Number of active buffers
  public int numBuffers = 0;
  /// Number of active offscreen frame buffers
  public int numFrameBuffers = 0;

  /// Number of active index buffers
  public int numIndexBuffers = 0;
  /// Sum of index buffers update sizes
  public long indexUpdateBytes = 0;

  /// Number of active vertex buffers
  public int numVertexBuffers = 0;
  /// Sum of vertex buffers update sizes
  public long vertexUpdateBytes = 0;

  /// Number of active uniform buffers
  public int numUniformBuffers = 0;
  /// Number of times a uniform buffer is updated
  public int numUniformUpdates = 0;
  /// Sum of uniform buffers update sizes
  public long uniformUpdateBytes = 0;

  /// Total texture memory
  public int memTextures = 0;
  /// Total buffer memory
  public int memBuffers = 0;
  /// Total index buffer memory
  public int memIndexBuffers = 0;
  /// Total vertex buffer memory
  public int memVertexBuffers = 0;
  /// Total uniform buffer memory
  public int memUniformBuffers = 0;

  /// Number of stencil buffer clears
  public int stencilClears = 0;
  /// Number of stencil buffer updates
  public int stencilUpdates = 0;
  private Map<SourceLayerID, List<FeatureInfo>> renderedFeatures;

  public Map<SourceLayerID, List<FeatureInfo>> getRenderedFeatures() {
    return renderedFeatures;
  }

  public static class NDCBound {
    public final double minX;
    public final double maxX;
    public final double minY;
    public final double maxY;

    public NDCBound(double minX, double maxX, double minY, double maxY) {
      this.minX = minX;
      this.maxX = maxX;
      this.minY = minY;
      this.maxY = maxY;
    }
  }

  public static class TileID {
    public final int x;
    public final int y;
    public final int z;
    public final int overscaledZ;
    public final int wrap;

    public TileID(int z, int x, int y, int overscaledZ, int wrap) {
      this.z = z;
      this.x = x;
      this.y = y;
      this.overscaledZ = overscaledZ;
      this.wrap = wrap;
    }
  }

  public static class FeatureInfo {
    public final String featureID;
    public final NDCBound ndcBound;
    public final Set<TileID> tileIDs;

    public FeatureInfo(String featureID, NDCBound ndcBound, Set<TileID> tileIDs) {
      this.featureID = featureID;
      this.ndcBound = ndcBound;
      this.tileIDs = tileIDs;
    }
  }

  public static class SourceLayerID {
    public final String sourceID;
    public final String layerID;

    public SourceLayerID(String sourceID, String layerID) {
      this.sourceID = sourceID;
      this.layerID = layerID;
    }

    @Override
    public boolean equals(Object o) {
      if (this == o) {
        return true;
      }
      if (o == null || getClass() != o.getClass()) {
        return false;
      }
      SourceLayerID that = (SourceLayerID) o;
      return sourceID.equals(that.sourceID) && layerID.equals(that.layerID);
    }

    @Override
    public int hashCode() {
      return 31 * sourceID.hashCode() + layerID.hashCode();
    }

    @NonNull
    @Override
    public String toString() {
      return sourceID + '/' + layerID;
    }
  }
}
