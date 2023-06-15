package org.maplibre.android.util;

import android.os.Parcel;
import android.os.Parcelable;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.android.WellKnownTileServer;
import org.maplibre.android.exceptions.MapLibreConfigurationException;

/**
 * Tile server options - baseUrl and similar properties
 */
public class TileServerOptions implements Parcelable {

  /**
   * Inner class responsible for recreating Parcels into objects.
   */
  public static final Creator<TileServerOptions> CREATOR = new Creator<TileServerOptions>() {
    public TileServerOptions createFromParcel(@NonNull Parcel in) {
      return new TileServerOptions(in);
    }

    public TileServerOptions[] newArray(int size) {
      return new TileServerOptions[size];
    }
  };

  @Keep
  private String baseURL;
  @Keep
  private String uriSchemeAlias;
  @Keep
  private String sourceTemplate;
  @Keep
  private String sourceDomainName;
  @Keep
  @Nullable
  private String sourceVersionPrefix;
  @Keep
  private String styleTemplate;
  @Keep
  private String styleDomainName;
  @Keep
  @Nullable
  private String styleVersionPrefix;
  @Keep
  private String spritesTemplate;
  @Keep
  private String spritesDomainName;
  @Keep
  @Nullable
  private String spritesVersionPrefix;
  @Keep
  private String glyphsTemplate;
  @Keep
  private String glyphsDomainName;
  @Keep
  @Nullable
  private String glyphsVersionPrefix;
  @Keep
  private String tileTemplate;
  @Keep
  private String tileDomainName;
  @Keep
  @Nullable
  private String tileVersionPrefix;
  @Keep
  private String apiKeyParameterName;
  @Keep
  private boolean apiKeyRequired;
  @Keep
  private String defaultStyle;
  @Keep
  private DefaultStyle[] defaultStyles;

  /**
   * Construct a new TileServerOptions given string arguments
   *
   * @param baseURL              tile server base url
   * @param uriSchemeAlias       scheme alias such as maplibre://
   * @param sourceTemplate       tile source url template
   * @param sourceDomainName     tile source domain name
   * @param sourceVersionPrefix  tile source version prefix
   * @param styleTemplate        style url template
   * @param styleDomainName      the name of style domain in canonical url
   * @param styleVersionPrefix   version prefix
   * @param spritesTemplate      sprites url template
   * @param spritesDomainName    the name of the sprites domain in canonical url
   * @param spritesVersionPrefix the sprite version prefix
   * @param glyphsTemplate       glyphs url template
   * @param glyphsDomainName     the name of the glyphs domain in canonical url
   * @param glyphsVersionPrefix  the glyphs version prefix
   * @param tileTemplate         tile url template
   * @param tileDomainName       the name of the tile domain in canonical url
   * @param tileVersionPrefix    the tile version prefix
   * @param apiKeyParameterName  the name of api key parameter
   * @param apiKeyRequired       indicates if API key is required
   * @param defaultStyle         the name of the default style
   * @param defaultStyles        the list of default styles
   */
  @Keep
  public TileServerOptions(
          String baseURL,
          String uriSchemeAlias,
          String sourceTemplate,
          String sourceDomainName,
          @Nullable String sourceVersionPrefix,
          String styleTemplate,
          String styleDomainName,
          @Nullable String styleVersionPrefix,
          String spritesTemplate,
          String spritesDomainName,
          @Nullable String spritesVersionPrefix,
          String glyphsTemplate,
          String glyphsDomainName,
          @Nullable String glyphsVersionPrefix,
          String tileTemplate,
          String tileDomainName,
          @Nullable String tileVersionPrefix,
          String apiKeyParameterName,
          boolean apiKeyRequired,
          String defaultStyle,
          DefaultStyle[] defaultStyles
  ) {
    setBaseURL(baseURL);
    setUriSchemeAlias(uriSchemeAlias);
    setSourceTemplate(sourceTemplate);
    setSourceDomainName(sourceDomainName);
    setSourceVersionPrefix(sourceVersionPrefix);
    setStyleTemplate(styleTemplate);
    setStyleDomainName(styleDomainName);
    setStyleVersionPrefix(styleVersionPrefix);
    setSpritesTemplate(spritesTemplate);
    setSpritesDomainName(spritesDomainName);
    setSpritesVersionPrefix(spritesVersionPrefix);
    setGlyphsTemplate(glyphsTemplate);
    setGlyphsDomainName(glyphsDomainName);
    setGlyphsVersionPrefix(glyphsVersionPrefix);
    setTileTemplate(tileTemplate);
    setTileDomainName(tileDomainName);
    setTileVersionPrefix(tileVersionPrefix);
    setApiKeyParameterName(apiKeyParameterName);
    setDefaultStyles(defaultStyles);
    setDefaultStyle(defaultStyle);
    setApiKeyRequired(apiKeyRequired);
  }

  public void setBaseURL(String baseURL) {
    this.baseURL = baseURL;
  }

  public String getBaseURL() {
    return this.baseURL;
  }

  public void setUriSchemeAlias(String uriSchemeAlias) {
    this.uriSchemeAlias = uriSchemeAlias;
  }

  public String getUriSchemeAlias() {
    return this.uriSchemeAlias;
  }

  public void setSourceTemplate(String sourceTemplate) {
    this.sourceTemplate = sourceTemplate;
  }

  public String getSourceTemplate() {
    return this.sourceTemplate;
  }

  public void setSourceDomainName(String domainName) {
    this.sourceDomainName = domainName;
  }

  public String getSourceDomainName() {
    return this.sourceDomainName;
  }

  public void setSourceVersionPrefix(String sourceVersionPrefix) {
    this.sourceVersionPrefix = sourceVersionPrefix;
  }

  public String getSourceVersionPrefix() {
    return this.sourceVersionPrefix;
  }

  public void setStyleTemplate(String styleTemplate) {
    this.styleTemplate = styleTemplate;
  }

  public String getStyleTemplate() {
    return this.styleTemplate;
  }

  public void setStyleDomainName(String styleDomainName) {
    this.styleDomainName = styleDomainName;
  }

  public String getStyleDomainName() {
    return this.styleDomainName;
  }

  public void setStyleVersionPrefix(String styleVersionPrefix) {
    this.styleVersionPrefix = styleVersionPrefix;
  }

  public String getStyleVersionPrefix() {
    return this.styleVersionPrefix;
  }

  public void setSpritesTemplate(String spritesTemplate) {
    this.spritesTemplate = spritesTemplate;
  }

  public String getSpritesTemplate() {
    return this.spritesTemplate;
  }

  public void setSpritesDomainName(String spritesDomainName) {
    this.spritesDomainName = spritesDomainName;
  }

  public String getSpritesDomainName() {
    return this.spritesDomainName;
  }

  public void setSpritesVersionPrefix(String spritesVersionPrefix) {
    this.spritesVersionPrefix = spritesVersionPrefix;
  }

  public String getSpritesVersionPrefix() {
    return this.spritesVersionPrefix;
  }

  public void setGlyphsTemplate(String glyphsTemplate) {
    this.glyphsTemplate = glyphsTemplate;
  }

  public String getGlyphsTemplate() {
    return this.glyphsTemplate;
  }

  public void setGlyphsDomainName(String glyphsDomainName) {
    this.glyphsDomainName = glyphsDomainName;
  }

  public String getGlyphsDomainName() {
    return this.glyphsDomainName;
  }

  public void setGlyphsVersionPrefix(String glyphsVersionPrefix) {
    this.glyphsVersionPrefix = glyphsVersionPrefix;
  }

  public String getGlyphsVersionPrefix() {
    return this.glyphsVersionPrefix;
  }

  public void setTileTemplate(String tileTemplate) {
    this.tileTemplate = tileTemplate;
  }

  public String getTileTemplate() {
    return this.tileTemplate;
  }

  public void setTileDomainName(String tileDomainName) {
    this.tileDomainName = tileDomainName;
  }

  public String getTileDomainName() {
    return this.tileDomainName;
  }

  public void setTileVersionPrefix(String tileVersionPrefix) {
    this.tileVersionPrefix = tileVersionPrefix;
  }

  public String getTileVersionPrefix() {
    return this.tileVersionPrefix;
  }

  public void setApiKeyParameterName(String apiKeyParameterName) {
    this.apiKeyParameterName = apiKeyParameterName;
  }

  public String getApiKeyParameterName() {
    return this.apiKeyParameterName;
  }

  public void setApiKeyRequired(boolean isRequired) {
    this.apiKeyRequired = isRequired;
  }

  public boolean getApiKeyRequired() {
    return this.apiKeyRequired;
  }

  public void setDefaultStyles(DefaultStyle[] defaultStyles) {
    this.defaultStyles = defaultStyles;
  }

  public DefaultStyle[] getDefaultStyles() {
    return this.defaultStyles;
  }

  public void setDefaultStyle(String defaultStyleName) {
    this.defaultStyle = defaultStyleName;
  }

  public String getDefaultStyle() {
    return this.defaultStyle;
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
   * Constructs a new tile server options tuple given a parcel.
   *
   * @param in the parcel containing the tile server options values
   */
  protected TileServerOptions(Parcel in) {
    setBaseURL(in.readString());
    setUriSchemeAlias(in.readString());
    setSourceTemplate(in.readString());
    setSourceDomainName(in.readString());
    setSourceVersionPrefix(in.readString());
    setStyleTemplate(in.readString());
    setStyleDomainName(in.readString());
    setStyleVersionPrefix(in.readString());
    setSpritesTemplate(in.readString());
    setSpritesDomainName(in.readString());
    setSpritesVersionPrefix(in.readString());
    setGlyphsTemplate(in.readString());
    setGlyphsDomainName(in.readString());
    setGlyphsVersionPrefix(in.readString());
    setTileTemplate(in.readString());
    setTileDomainName(in.readString());
    setTileVersionPrefix(in.readString());
    setApiKeyParameterName(in.readString());
    setApiKeyRequired(in.readByte() != 0);
    setDefaultStyle(in.readString());
    in.createTypedArray(DefaultStyle.CREATOR);
  }

  /**
   * Flatten this object in to a Parcel.
   *
   * @param out   The Parcel in which the object should be written.
   * @param flags Additional flags about how the object should be written
   */
  @Override
  public void writeToParcel(@NonNull Parcel out, int flags) {
    out.writeString(baseURL);
    out.writeString(uriSchemeAlias);
    out.writeString(sourceTemplate);
    out.writeString(sourceDomainName);
    out.writeString(sourceVersionPrefix);
    out.writeString(styleTemplate);
    out.writeString(styleDomainName);
    out.writeString(styleVersionPrefix);
    out.writeString(spritesTemplate);
    out.writeString(spritesDomainName);
    out.writeString(spritesVersionPrefix);
    out.writeString(glyphsTemplate);
    out.writeString(glyphsDomainName);
    out.writeString(glyphsVersionPrefix);
    out.writeString(tileTemplate);
    out.writeString(tileDomainName);
    out.writeString(tileVersionPrefix);
    out.writeString(apiKeyParameterName);
    out.writeByte((byte) (apiKeyRequired ? 1 : 0));
    out.writeString(defaultStyle);
    out.writeTypedArray(defaultStyles, 0);
  }

  public static TileServerOptions get(WellKnownTileServer tileServer) {
    switch (tileServer) {
      case Mapbox:
        return mapboxConfiguration();
      case MapTiler:
        return mapTilerConfiguration();
      case MapLibre:
        return mapLibreConfiguration();
      default:
        throw new MapLibreConfigurationException("Unknown tile server");
    }
  }

  @Keep
  @NonNull
  private static native TileServerOptions defaultConfiguration();

  @Keep
  @NonNull
  private static native TileServerOptions mapboxConfiguration();

  @Keep
  @NonNull
  private static native TileServerOptions mapTilerConfiguration();

  @Keep
  @NonNull
  private static native TileServerOptions mapLibreConfiguration();
}
