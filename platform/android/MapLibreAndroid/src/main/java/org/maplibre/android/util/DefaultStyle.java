package org.maplibre.android.util;

import android.os.Parcel;
import android.os.Parcelable;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

/**
 * Default style definition
 */
public class DefaultStyle implements Parcelable {

  /**
   * Inner class responsible for recreating Parcels into objects.
   */
  public static final Creator<DefaultStyle> CREATOR = new Creator<DefaultStyle>() {
    public DefaultStyle createFromParcel(@NonNull Parcel in) {
      return new DefaultStyle(in);
    }

    public DefaultStyle[] newArray(int size) {
      return new DefaultStyle[size];
    }
  };

  @Keep
  private String url;
  @Keep
  private String name;
  @Keep
  private int version;

  /**
   * Construct a new Default style definition
   *
   * @param url                  canonical style url
   * @param name                 style name
   * @param version              style version
   */
  @Keep
  public DefaultStyle(
          String url,
          String name,
          int version
  ) {
    setUrl(url);
    setName(name);
    setVersion(version);
  }

  public void setUrl(String url) {
    this.url = url;
  }

  public String getUrl() {
    return this.url;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getName() {
    return this.name;
  }

  public void setVersion(int version) {
    this.version = version;
  }

  public int getVersion() {
    return this.version;
  }


  /**
   * Describe the kinds of special objects contained in this Parcelable instance's marshaled representation.
   *
   * @return a bitmask indicating the set of special object types marshaled by this Parcelable object instance.
   */
  @Override
  public int describeContents() {
    return 0;
  }

  /**
   * Constructs a new default style tuple given a parcel.
   *
   * @param in the parcel containing the tile server options values
   */
  protected DefaultStyle(Parcel in) {
    setUrl(in.readString());
    setName(in.readString());
    setVersion(in.readInt());
  }

  /**
   * Flatten this object in to a Parcel.
   *
   * @param out   The Parcel in which the object should be written.
   * @param flags Additional flags about how the object should be written
   */
  @Override
  public void writeToParcel(@NonNull Parcel out, int flags) {
    out.writeString(url);
    out.writeString(name);
    out.writeInt(version);
  }
}
