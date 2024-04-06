package org.maplibre.android.maps;

/**
 * Describes the image content, e.g. where text can be fit into an image.
 * <p>
 * When sizing icons with icon-text-fit,
 * the icon size will be adjusted so that the this content box fits exactly around the text.
 */
public final class ImageContent {

  private final float left;
  private final float top;
  private final float right;
  private final float bottom;

  public ImageContent(float left,
                      float top,
                      float right,
                      float bottom) {
    this.left = left;
    this.top = top;
    this.right = right;
    this.bottom = bottom;
  }

  /**
   * Get the array for this content, sorted by left, top, right, bottom.
   *
   * @return the content array.
   */
  public float[] getContentArray() {
    float[] array = new float[4];
    array[0] = left;
    array[1] = top;
    array[2] = right;
    array[3] = bottom;
    return array;
  }

  @Override
  public boolean equals(Object obj) {
    if (!(obj instanceof ImageContent)) {
      return false;
    }
    ImageContent other = (ImageContent) obj;
    return this.left == other.left
      && this.top == other.top
      && this.right == other.right
      && this.bottom == other.bottom;
  }

  @Override
  public int hashCode() {
    int result = (left != +0.0f ? Float.floatToIntBits(left) : 0);
    result = 31 * result + (top != +0.0f ? Float.floatToIntBits(top) : 0);
    result = 31 * result + (right != +0.0f ? Float.floatToIntBits(right) : 0);
    result = 31 * result + (bottom != +0.0f ? Float.floatToIntBits(bottom) : 0);
    return result;
  }

  @Override
  public String toString() {
    return "[ " + "left: " + left + ", " + "top: " + top + ", " + "right: " + right + ", " + "bottom: " + bottom + " ]";
  }

}
