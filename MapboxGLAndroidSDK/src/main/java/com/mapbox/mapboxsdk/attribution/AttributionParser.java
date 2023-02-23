package com.mapbox.mapboxsdk.attribution;

import android.content.Context;
import android.text.Html;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.style.URLSpan;

import androidx.annotation.NonNull;

import java.lang.ref.WeakReference;
import java.util.LinkedHashSet;
import java.util.Set;

/**
 * Responsible for parsing attribution data coming from Sources and MapSnapshot.
 * <p>
 * Exposes multiple configuration options to manipulate data being parsed.
 * Use the Options object to build these configurations.
 * </p>
 */
public class AttributionParser {

  private final WeakReference<Context> context;
  private final Set<Attribution> attributions = new LinkedHashSet<>();
  private final String attributionData;
  private final boolean withImproveMap;
  private final boolean withCopyrightSign;
  private final boolean withMapboxAttribution;

  AttributionParser(WeakReference<Context> context, String attributionData, boolean withImproveMap,
                    boolean withCopyrightSign, boolean withMapboxAttribution) {
    this.context = context;
    this.attributionData = attributionData;
    this.withImproveMap = withImproveMap;
    this.withCopyrightSign = withCopyrightSign;
    this.withMapboxAttribution = withMapboxAttribution;
  }

  /**
   * Get parsed attributions.
   *
   * @return the attributions
   */
  @NonNull
  public Set<Attribution> getAttributions() {
    return attributions;
  }

  /**
   * Get parsed attribution string.
   *
   * @return the parsed attribution string
   */
  @NonNull
  public String createAttributionString() {
    return createAttributionString(false);
  }

  /**
   * Get parsed attribution string.
   *
   * @param shortenedOutput if attribution string should contain shortened output
   * @return the parsed attribution string
   */
  @NonNull
  public String createAttributionString(boolean shortenedOutput) {
    StringBuilder stringBuilder = new StringBuilder(withCopyrightSign ? "" : "© ");
    int counter = 0;
    for (Attribution attribution : attributions) {
      counter++;
      stringBuilder.append(!shortenedOutput ? attribution.getTitle() : attribution.getTitleAbbreviated());
      if (counter != attributions.size()) {
        stringBuilder.append(" / ");
      }
    }
    return stringBuilder.toString();
  }

  /**
   * Main attribution for configuration
   */
  protected void parse() {
    parseAttributions();
  }

  /**
   * Parse attributions
   */
  private void parseAttributions() {
    SpannableStringBuilder htmlBuilder = (SpannableStringBuilder) fromHtml(attributionData);
    URLSpan[] urlSpans = htmlBuilder.getSpans(0, htmlBuilder.length(), URLSpan.class);
    for (URLSpan urlSpan : urlSpans) {
      parseUrlSpan(htmlBuilder, urlSpan);
    }
  }

  /**
   * Parse an URLSpan containing an attribution.
   *
   * @param htmlBuilder the html builder
   * @param urlSpan     the url span to be parsed
   */
  private void parseUrlSpan(@NonNull SpannableStringBuilder htmlBuilder, @NonNull URLSpan urlSpan) {
    String url = urlSpan.getURL();
    if (isUrlValid(url)) {
      String anchor = parseAnchorValue(htmlBuilder, urlSpan);
      attributions.add(new Attribution(anchor, url));
    }
  }

  /**
   * Invoked to validate if an url is valid to be included in the final attribution.
   *
   * @param url the url to be validated
   * @return if the url is valid
   */
  private boolean isUrlValid(@NonNull String url) {
    return isValidForImproveThisMap(url) && isValidForMapbox(url);
  }

  /**
   * Invoked to validate if an url is valid for the improve map configuration.
   *
   * @param url the url to be validated
   * @return if the url is valid for improve this map
   */
  private boolean isValidForImproveThisMap(@NonNull String url) {
    return withImproveMap || !(Attribution.IMPROVE_MAP_URLS.contains(url));
  }

  /**
   * Invoked to validate if an url is valid for the Mapbox configuration.
   *
   * @param url the url to be validated
   * @return if the url is valid for Mapbox
   */
  private boolean isValidForMapbox(@NonNull String url) {
    return withMapboxAttribution || !url.equals(Attribution.MAPBOX_URL);
  }

  /**
   * Parse the attribution by parsing the anchor value of html href tag.
   *
   * @param htmlBuilder the html builder
   * @param urlSpan     the current urlSpan
   * @return the parsed anchor value
   */
  @NonNull
  private String parseAnchorValue(SpannableStringBuilder htmlBuilder, URLSpan urlSpan) {
    int start = htmlBuilder.getSpanStart(urlSpan);
    int end = htmlBuilder.getSpanEnd(urlSpan);
    int length = end - start;
    char[] charKey = new char[length];
    htmlBuilder.getChars(start, end, charKey, 0);
    return stripCopyright(String.valueOf(charKey));
  }

  /**
   * Utility to strip the copyright sign from an attribution
   *
   * @param anchor the attribution string to strip
   * @return the stripped attribution string without the copyright sign
   */
  @NonNull
  private String stripCopyright(@NonNull String anchor) {
    if (!withCopyrightSign && anchor.startsWith("© ")) {
      anchor = anchor.substring(2, anchor.length());
    }
    return anchor;
  }

  /**
   * Convert a string to a spanned html representation.
   *
   * @param html the string to convert
   * @return the spanned html representation
   */
  private static Spanned fromHtml(String html) {
    Spanned result;
    if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
      result = Html.fromHtml(html, Html.FROM_HTML_MODE_LEGACY);
    } else {
      result = Html.fromHtml(html);
    }
    return result;
  }

  /**
   * Builder to configure using an AttributionParser.
   * <p>
   * AttributionData, set with {@link #withAttributionData(String...)}, is the only required property to build
   * the underlying AttributionParser. Other properties include trimming the copyright sign, hiding
   * attribution as improve this map and Mapbox.
   * </p>
   */
  public static class Options {
    private final WeakReference<Context> context;
    private boolean withImproveMap = true;
    private boolean withCopyrightSign = true;
    private boolean withMapboxAttribution = true;
    private String[] attributionDataStringArray;

    public Options(@NonNull Context context) {
      this.context = new WeakReference<>(context);
    }

    @NonNull
    public Options withAttributionData(String... attributionData) {
      this.attributionDataStringArray = attributionData;
      return this;
    }

    @NonNull
    public Options withImproveMap(boolean withImproveMap) {
      this.withImproveMap = withImproveMap;
      return this;
    }

    @NonNull
    public Options withCopyrightSign(boolean withCopyrightSign) {
      this.withCopyrightSign = withCopyrightSign;
      return this;
    }

    @NonNull
    public Options withMapboxAttribution(boolean withMapboxAttribution) {
      this.withMapboxAttribution = withMapboxAttribution;
      return this;
    }

    @NonNull
    public AttributionParser build() {
      if (attributionDataStringArray == null) {
        throw new IllegalStateException("Using builder without providing attribution data");
      }

      String fullAttributionString = parseAttribution(attributionDataStringArray);
      AttributionParser attributionParser = new AttributionParser(
        context,
        fullAttributionString,
        withImproveMap,
        withCopyrightSign,
        withMapboxAttribution
      );
      attributionParser.parse();
      return attributionParser;
    }

    private String parseAttribution(String[] attribution) {
      StringBuilder builder = new StringBuilder();
      for (String attr : attribution) {
        if (!attr.isEmpty()) {
          builder.append(attr);
        }
      }
      return builder.toString();
    }
  }
}
