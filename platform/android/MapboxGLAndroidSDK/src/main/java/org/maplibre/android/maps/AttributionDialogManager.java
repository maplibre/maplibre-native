package org.maplibre.android.maps;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.android.MapStrictMode;
import org.maplibre.android.MapLibre;
import org.maplibre.android.R;
import org.maplibre.android.attribution.Attribution;
import org.maplibre.android.attribution.AttributionParser;
import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.style.sources.Source;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Responsible for managing attribution interactions on the map.
 * <p>
 * When the user clicks the attribution icon, {@link AttributionDialogManager#onClick(View)} will be invoked.
 * An attribution dialog will be shown to the user with contents based on the attributions found in the map style.
 * </p>
 */
public class AttributionDialogManager implements View.OnClickListener, DialogInterface.OnClickListener {
  private static final String MAP_FEEDBACK_URL = "https://apps.mapbox.com/feedback";
  private static final String MAP_FEEDBACK_URL_OLD = "https://www.mapbox.com/map-feedback";
  private static final String MAP_FEEDBACK_URL_LOCATION_FRAGMENT_FORMAT = "/%f/%f/%f/%f/%d";
  private static final String MAP_FEEDBACK_STYLE_URI_REGEX = "^(.*://[^:^/]*)/(.*)/(.*)";

  @NonNull
  private final Context context;
  @NonNull
  private final MapLibreMap maplibreMap;
  private Set<Attribution> attributionSet;
  private AlertDialog dialog;

  public AttributionDialogManager(@NonNull Context context, @NonNull MapLibreMap maplibreMap) {
    this.context = context;
    this.maplibreMap = maplibreMap;
  }

  // Called when someone presses the attribution icon on the map
  @Override
  public void onClick(@NonNull View view) {
    attributionSet = new AttributionBuilder(maplibreMap, view.getContext()).build();

    boolean isActivityFinishing = false;
    if (context instanceof Activity) {
      isActivityFinishing = ((Activity) context).isFinishing();
    }

    // check is hosting activity isn't finishing
    // https://github.com/mapbox/mapbox-gl-native/issues/11238
    if (!isActivityFinishing) {
      showAttributionDialog(getAttributionTitles());
    }
  }

  protected void showAttributionDialog(@NonNull String[] attributionTitles) {
    AlertDialog.Builder builder = new AlertDialog.Builder(context);
    builder.setTitle(R.string.maplibre_attributionsDialogTitle);
    builder.setAdapter(new ArrayAdapter<>(context, R.layout.maplibre_attribution_list_item, attributionTitles), this);
    dialog = builder.show();
  }

  private String[] getAttributionTitles() {
    List<String> titles = new ArrayList<>();
    for (Attribution attribution : attributionSet) {
      titles.add(attribution.getTitle());
    }
    return titles.toArray(new String[titles.size()]);
  }

  // Called when someone selects an attribution from the dialog
  @Override
  public void onClick(DialogInterface dialog, int which) {
    showMapAttributionWebPage(which);
  }

  public void onStop() {
    if (dialog != null && dialog.isShowing()) {
      dialog.dismiss();
    }
  }

  private boolean isLatestEntry(int attributionKeyIndex) {
    return attributionKeyIndex == getAttributionTitles().length - 1;
  }

  private void showMapAttributionWebPage(int which) {
    Attribution[] attributions = attributionSet.toArray(new Attribution[attributionSet.size()]);
    String url = attributions[which].getUrl();
    if (url.contains(MAP_FEEDBACK_URL_OLD) || url.contains(MAP_FEEDBACK_URL)) {
      url = buildMapFeedbackMapUrl(MapLibre.getApiKey());
    }
    showWebPage(url);
  }

  @NonNull
  String buildMapFeedbackMapUrl(@Nullable String apiKey) {
    // TODO Add Android Maps SDK version to the query parameter, currently the version API is not available.
    // TODO Keep track of this issue at [#15632](https://github.com/mapbox/mapbox-gl-native/issues/15632)

    Uri.Builder builder = Uri.parse(MAP_FEEDBACK_URL).buildUpon();

    CameraPosition cameraPosition = maplibreMap.getCameraPosition();
    if (cameraPosition != null) {
      builder.encodedFragment(String.format(Locale.getDefault(), MAP_FEEDBACK_URL_LOCATION_FRAGMENT_FORMAT,
              cameraPosition.target.getLongitude(), cameraPosition.target.getLatitude(),
              cameraPosition.zoom, cameraPosition.bearing, (int) cameraPosition.tilt));
    }

    String packageName = context.getApplicationContext().getPackageName();
    if (packageName != null) {
      builder.appendQueryParameter("referrer", packageName);
    }

    if (apiKey != null) {
      //TODO:PP
      builder.appendQueryParameter("access_token", apiKey);
    }

    Style style = maplibreMap.getStyle();
    if (style != null) {
      String styleUri = style.getUri();
      Pattern pattern = Pattern.compile(MAP_FEEDBACK_STYLE_URI_REGEX);
      Matcher matcher = pattern.matcher(styleUri);

      if (matcher.find()) {
        String styleOwner = matcher.group(2);
        String styleId = matcher.group(3);

        builder.appendQueryParameter("owner", styleOwner)
                .appendQueryParameter("id", styleId);
      }
    }

    return builder.build().toString();
  }

  private void showWebPage(@NonNull String url) {
    try {
      Intent intent = new Intent(Intent.ACTION_VIEW);
      intent.setData(Uri.parse(url));
      context.startActivity(intent);
    } catch (ActivityNotFoundException exception) {
      // explicitly handling if the device hasn't have a web browser installed. #8899
      Toast.makeText(context, R.string.maplibre_attributionErrorNoBrowser, Toast.LENGTH_LONG).show();
      MapStrictMode.strictModeViolation(exception);
    }
  }

  private static class AttributionBuilder {

    private final MapLibreMap maplibreMap;
    @NonNull
    private final WeakReference<Context> context;

    AttributionBuilder(MapLibreMap maplibreMap, Context context) {
      this.maplibreMap = maplibreMap;
      this.context = new WeakReference<>(context);
    }

    private Set<Attribution> build() {
      Context context = this.context.get();
      if (context == null) {
        return Collections.emptySet();
      }

      List<String> attributions = new ArrayList<>();
      String attribution;

      Style style = maplibreMap.getStyle();
      if (style != null) {
        for (Source source : style.getSources()) {
          attribution = source.getAttribution();
          if (!attribution.isEmpty()) {
            attributions.add(attribution);
          }
        }
      }

      return new AttributionParser.Options(context)
        .withCopyrightSign(true)
        .withImproveMap(true)
        .withAttributionData(attributions.toArray(new String[attributions.size()]))
        .build().getAttributions();
    }
  }
}
