package org.maplibre.android.maps;

import androidx.annotation.Keep;

@Keep
public class Image {
  private final byte[] buffer;
  private final float pixelRatio;
  private final String name;
  private final int width;
  private final int height;
  private final boolean sdf;
  private final float[] content;
  private final float[] stretchX;
  private final float[] stretchY;

  public Image(byte[] buffer, float pixelRatio, String name, int width, int height, boolean sdf) {
    this(buffer, pixelRatio, name, width, height, sdf, null, null, null);
  }

  public Image(byte[] buffer, float pixelRatio, String name, int width, int height, boolean sdf,
               float[] stretchX, float[] stretchY, float[] content) {
    this.buffer = buffer;
    this.pixelRatio = pixelRatio;
    this.name = name;
    this.width = width;
    this.height = height;
    this.sdf = sdf;
    this.content = content;
    this.stretchX = stretchX;
    this.stretchY = stretchY;
  }
}
