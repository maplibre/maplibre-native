package org.maplibre.android.maps;

import android.graphics.Bitmap;
import android.graphics.PointF;
import android.graphics.RectF;

import androidx.annotation.IntRange;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.maplibre.geojson.Feature;
import org.maplibre.geojson.Geometry;
import org.maplibre.android.annotations.Marker;
import org.maplibre.android.annotations.Polygon;
import org.maplibre.android.annotations.Polyline;
import org.maplibre.android.camera.CameraPosition;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.geometry.LatLngBounds;
import org.maplibre.android.geometry.ProjectedMeters;
import org.maplibre.android.style.expressions.Expression;
import org.maplibre.android.style.layers.Layer;
import org.maplibre.android.style.layers.TransitionOptions;
import org.maplibre.android.style.light.Light;
import org.maplibre.android.style.sources.Source;

import java.util.List;

interface NativeMap {

  //
  // Lifecycle API
  //

  void resizeView(int width, int height);

  void onLowMemory();

  void destroy();

  boolean isDestroyed();

  //
  // Camera API
  //

  void jumpTo(@NonNull LatLng center, double zoom, double pitch, double bearing, double[] padding);

  void easeTo(@NonNull LatLng center, double zoom, double bearing, double pitch, double[] padding, long duration,
              boolean easingInterpolator);

  void flyTo(@NonNull LatLng center, double zoom, double bearing, double pitch, double[] padding, long duration);

  void moveBy(double deltaX, double deltaY, long duration);

  @NonNull
  CameraPosition getCameraPosition();

  CameraPosition getCameraForLatLngBounds(@NonNull LatLngBounds bounds, int[] padding, double bearing, double pitch);

  CameraPosition getCameraForGeometry(@NonNull Geometry geometry, int[] padding, double bearing, double pitch);

  void resetPosition();

  void setLatLng(@NonNull LatLng latLng, long duration);

  LatLng getLatLng();

  void setLatLngBounds(@Nullable LatLngBounds latLngBounds);

  void setVisibleCoordinateBounds(@NonNull LatLng[] coordinates, @NonNull RectF padding,
                                  double direction, long duration);

  void setPitch(double pitch, long duration);

  double getPitch();

  void setZoom(double zoom, @NonNull PointF focalPoint, long duration);

  double getZoom();

  void setMinZoom(double zoom);

  double getMinZoom();

  void setMaxZoom(double zoom);

  double getMaxZoom();

  void setMinPitch(double pitch);

  double getMinPitch();

  void setMaxPitch(double pitch);

  double getMaxPitch();

  void resetZoom();

  void rotateBy(double sx, double sy, double ex, double ey, long duration);

  void setBearing(double degrees, long duration);

  void setBearing(double degrees, double fx, double fy, long duration);

  double getBearing();

  void resetNorth();

  void cancelTransitions();

  //
  // Style API
  //

  void setStyleUri(String url);

  @NonNull
  String getStyleUri();

  void setStyleJson(String newStyleJson);

  @NonNull
  String getStyleJson();

  boolean isFullyLoaded();

  void addLayer(@NonNull Layer layer);

  void addLayerBelow(@NonNull Layer layer, @NonNull String below);

  void addLayerAbove(@NonNull Layer layer, @NonNull String above);

  void addLayerAt(@NonNull Layer layer, @IntRange(from = 0) int index);

  @NonNull
  List<Layer> getLayers();

  Layer getLayer(String layerId);

  boolean removeLayer(@NonNull String layerId);

  boolean removeLayer(@NonNull Layer layer);

  boolean removeLayerAt(@IntRange(from = 0) int index);

  void addSource(@NonNull Source source);

  @NonNull
  List<Source> getSources();

  Source getSource(@NonNull String sourceId);

  boolean removeSource(@NonNull String sourceId);

  boolean removeSource(@NonNull Source source);

  void setTransitionOptions(@NonNull TransitionOptions transitionOptions);

  @NonNull
  TransitionOptions getTransitionOptions();

  void addImages(Image[] images);

  Bitmap getImage(String name);

  void removeImage(String name);

  Light getLight();

  //
  // Content padding API
  //

  void setContentPadding(double[] padding);

  double[] getContentPadding();

  //
  // Query API
  //

  @NonNull
  List<Feature> queryRenderedFeatures(@NonNull PointF coordinates,
                                      @Nullable String[] layerIds,
                                      @Nullable Expression filter);

  @NonNull
  List<Feature> queryRenderedFeatures(@NonNull RectF coordinates,
                                      @Nullable String[] layerIds,
                                      @Nullable Expression filter);

  //
  // Projection API
  //

  double getMetersPerPixelAtLatitude(double lat);

  ProjectedMeters projectedMetersForLatLng(@NonNull LatLng latLng);

  LatLng latLngForProjectedMeters(@NonNull ProjectedMeters projectedMeters);

  @NonNull
  PointF pixelForLatLng(@NonNull LatLng latLng);

  void pixelsForLatLngs(@NonNull double[] input, @NonNull double[] output);

  void getVisibleCoordinateBounds(@NonNull double[] output);

  LatLng latLngForPixel(@NonNull PointF pixel);

  void latLngsForPixels(@NonNull double[] input, @NonNull double[] output);

  //
  // Utils API
  //

  void setOnFpsChangedListener(@NonNull MapLibreMap.OnFpsChangedListener listener);

  void setDebug(boolean debug);

  boolean getDebug();

  void setReachability(boolean status);

  void setApiBaseUrl(String baseUrl);

  void setPrefetchTiles(boolean enable);

  boolean getPrefetchTiles();

  void setPrefetchZoomDelta(@IntRange(from = 0) int delta);

  @IntRange(from = 0)
  int getPrefetchZoomDelta();

  void setGestureInProgress(boolean inProgress);

  float getPixelRatio();

  void triggerRepaint();

  void setSwapBehaviorFlush(boolean flush);

  //
  // Deprecated Annotations API
  //

  long addMarker(Marker marker);

  @NonNull
  long[] addMarkers(@NonNull List<Marker> markers);

  long addPolyline(Polyline polyline);

  @NonNull
  long[] addPolylines(@NonNull List<Polyline> polylines);

  long addPolygon(Polygon polygon);

  @NonNull
  long[] addPolygons(@NonNull List<Polygon> polygons);

  void updateMarker(@NonNull Marker marker);

  void updatePolygon(@NonNull Polygon polygon);

  void updatePolyline(@NonNull Polyline polyline);

  void removeAnnotation(long id);

  void removeAnnotations(long[] ids);

  double getTopOffsetPixelsForAnnotationSymbol(String symbolName);

  void addAnnotationIcon(String symbol, int width, int height, float scale, byte[] pixels);

  void removeAnnotationIcon(String symbol);

  @NonNull
  long[] queryPointAnnotations(RectF rectF);

  @NonNull
  long[] queryShapeAnnotations(RectF rectF);

  @NonNull
  RectF getDensityDependantRectangle(RectF rectangle);

  long getNativePtr();

  void addSnapshotCallback(@NonNull MapLibreMap.SnapshotReadyCallback callback);
}