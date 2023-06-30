package org.maplibre.android.maps;


/**
 * Describes the image stretch areas.
 */
public final class ImageStretches {

  private final float first;
  private final float second;

  public ImageStretches(float first,
                        float second) {
    this.first = first;
    this.second = second;
  }

  /**
   * The first stretchable part in pixel units.
   */
  public float getFirst() {
    return first;
  }

  /**
   * The second stretchable part in pixel units.
   */
  public float getSecond() {
    return second;
  }

  @Override
  public boolean equals(Object obj) {
    if (!(obj instanceof ImageStretches)) {
      return false;
    }
    ImageStretches other = (ImageStretches) obj;
    return this.first == other.first
      && this.second == other.second;
  }

  @Override
  public int hashCode() {
    int result = (first != +0.0f ? Float.floatToIntBits(first) : 0);
    result = 31 * result + (second != +0.0f ? Float.floatToIntBits(second) : 0);
    return result;
  }

  @Override
  public String toString() {
    return "[ " + "first: " + first + ", " + "second: " + second + " ]";
  }

}
