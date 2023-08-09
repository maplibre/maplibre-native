package org.maplibre.android.annotations;

import static org.maplibre.android.constants.GeometryConstants.MAX_MERCATOR_LATITUDE;
import static org.maplibre.android.constants.GeometryConstants.MIN_MERCATOR_LATITUDE;

import android.graphics.PointF;

import androidx.annotation.ColorInt;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.UiThread;

import com.google.gson.JsonNull;
import com.google.gson.JsonObject;
import com.mapbox.android.gestures.MoveDistancesObject;
import com.mapbox.geojson.Geometry;
import com.mapbox.geojson.Point;
import com.mapbox.geojson.Polygon;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.maps.Projection;
import org.maplibre.android.style.layers.PropertyFactory;
import org.maplibre.android.utils.ColorUtils;

import java.util.ArrayList;
import java.util.List;

@UiThread
public class Fill extends AbstractAnnotation<Polygon> {

    private final AnnotationManager<?, Fill, ?, ?, ?, ?> annotationManager;

    /**
     * Create a fill.
     *
     * @param id         the id of the fill
     * @param jsonObject the features of the annotation
     * @param geometry   the geometry of the annotation
     */
    Fill(long id, AnnotationManager<?, Fill, ?, ?, ?, ?> annotationManager, JsonObject jsonObject, Polygon geometry) {
        super(id, jsonObject, geometry);
        this.annotationManager = annotationManager;
    }

    @Override
    void setUsedDataDrivenProperties() {
        if (!(jsonObject.get(FillOptions.PROPERTY_FILL_OPACITY) instanceof JsonNull)) {
            annotationManager.enableDataDrivenProperty(FillOptions.PROPERTY_FILL_OPACITY);
        }
        if (!(jsonObject.get(FillOptions.PROPERTY_FILL_COLOR) instanceof JsonNull)) {
            annotationManager.enableDataDrivenProperty(FillOptions.PROPERTY_FILL_COLOR);
        }
        if (!(jsonObject.get(FillOptions.PROPERTY_FILL_OUTLINE_COLOR) instanceof JsonNull)) {
            annotationManager.enableDataDrivenProperty(FillOptions.PROPERTY_FILL_OUTLINE_COLOR);
        }
        if (!(jsonObject.get(FillOptions.PROPERTY_FILL_PATTERN) instanceof JsonNull)) {
            annotationManager.enableDataDrivenProperty(FillOptions.PROPERTY_FILL_PATTERN);
        }
    }

    /**
     * Set a list of lists of LatLng for the fill, which represents the locations of the fill on the map
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param latLngs a list of a lists of the locations of the polygon in a latitude and longitude pairs
     */
    public void setLatLngs(List<List<LatLng>> latLngs) {
        List<List<Point>> points = new ArrayList<>();
        for (List<LatLng> innerLatLngs : latLngs) {
            List<Point> innerList = new ArrayList<>();
            for (LatLng latLng : innerLatLngs) {
                innerList.add(Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude()));
            }
            points.add(innerList);
        }
        geometry = Polygon.fromLngLats(points);
    }

    /**
     * Get a list of lists of LatLng for the fill, which represents the locations of the fill on the map
     *
     * @return a list of a lists of the locations of the polygon in a latitude and longitude pairs
     */
    @NonNull
    public List<List<LatLng>> getLatLngs() {
        Polygon polygon = (Polygon) geometry;
        List<List<LatLng>> latLngs = new ArrayList<>();
        List<List<Point>> coordinates = polygon.coordinates();
        if (coordinates != null) {
            for (List<Point> innerPoints : coordinates) {
                List<LatLng> innerList = new ArrayList<>();
                for (Point point : innerPoints) {
                    innerList.add(new LatLng(point.latitude(), point.longitude()));
                }
                latLngs.add(innerList);
            }
        }
        return latLngs;
    }

    // Property accessors

    /**
     * Get the FillOpacity property
     * <p>
     * The opacity of the entire fill layer. In contrast to the {@link PropertyFactory#fillColor}, this value will also affect the 1px stroke around the fill, if the stroke is used.
     * </p>
     *
     * @return property wrapper value around Float
     */
    public Float getFillOpacity() {
        return jsonObject.get(FillOptions.PROPERTY_FILL_OPACITY).getAsFloat();
    }

    /**
     * Set the FillOpacity property
     * <p>
     * The opacity of the entire fill layer. In contrast to the {@link PropertyFactory#fillColor}, this value will also affect the 1px stroke around the fill, if the stroke is used.
     * </p>
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param value constant property value for Float
     */
    public void setFillOpacity(Float value) {
        jsonObject.addProperty(FillOptions.PROPERTY_FILL_OPACITY, value);
    }

    /**
     * Get the FillColor property
     * <p>
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     * </p>
     *
     * @return color value for String
     */
    @ColorInt
    public int getFillColorAsInt() {
        return ColorUtils.rgbaToColor(jsonObject.get(FillOptions.PROPERTY_FILL_COLOR).getAsString());
    }

    /**
     * Get the FillColor property
     * <p>
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     * </p>
     *
     * @return color value for String
     */
    public String getFillColor() {
        return jsonObject.get(FillOptions.PROPERTY_FILL_COLOR).getAsString();
    }

    /**
     * Set the FillColor property
     * <p>
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     * </p>
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param color value for String
     */
    public void setFillColor(@ColorInt int color) {
        jsonObject.addProperty(FillOptions.PROPERTY_FILL_COLOR, ColorUtils.colorToRgbaString(color));
    }

    /**
     * Set the FillColor property
     * <p>
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     * </p>
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param color value for String
     */
    public void setFillColor(@NonNull String color) {
        jsonObject.addProperty("fill-color", color);
    }

    /**
     * Get the FillOutlineColor property
     * <p>
     * The outline color of the fill. Matches the value of {@link PropertyFactory#fillColor} if unspecified.
     * </p>
     *
     * @return color value for String
     */
    @ColorInt
    public int getFillOutlineColorAsInt() {
        return ColorUtils.rgbaToColor(jsonObject.get(FillOptions.PROPERTY_FILL_OUTLINE_COLOR).getAsString());
    }

    /**
     * Get the FillOutlineColor property
     * <p>
     * The outline color of the fill. Matches the value of {@link PropertyFactory#fillColor} if unspecified.
     * </p>
     *
     * @return color value for String
     */
    public String getFillOutlineColor() {
        return jsonObject.get(FillOptions.PROPERTY_FILL_OUTLINE_COLOR).getAsString();
    }

    /**
     * Set the FillOutlineColor property
     * <p>
     * The outline color of the fill. Matches the value of {@link PropertyFactory#fillColor} if unspecified.
     * </p>
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param color value for String
     */
    public void setFillOutlineColor(@ColorInt int color) {
        jsonObject.addProperty(FillOptions.PROPERTY_FILL_OUTLINE_COLOR, ColorUtils.colorToRgbaString(color));
    }

    /**
     * Set the FillOutlineColor property
     * <p>
     * The outline color of the fill. Matches the value of {@link PropertyFactory#fillColor} if unspecified.
     * </p>
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param color value for String
     */
    public void setFillOutlineColor(@NonNull String color) {
        jsonObject.addProperty("fill-outline-color", color);
    }

    /**
     * Get the FillPattern property
     * <p>
     * Name of image in sprite to use for drawing image fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     * </p>
     *
     * @return property wrapper value around String
     */
    public String getFillPattern() {
        return jsonObject.get(FillOptions.PROPERTY_FILL_PATTERN).getAsString();
    }

    /**
     * Set the FillPattern property
     * <p>
     * Name of image in sprite to use for drawing image fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     * </p>
     * <p>
     * To update the fill on the map use {@link FillManager#update(AbstractAnnotation)}.
     * <p>
     *
     * @param value constant property value for String
     */
    public void setFillPattern(String value) {
        jsonObject.addProperty(FillOptions.PROPERTY_FILL_PATTERN, value);
    }

    @Override
    @Nullable
    Geometry getOffsetGeometry(@NonNull Projection projection, @NonNull MoveDistancesObject moveDistancesObject,
                               float touchAreaShiftX, float touchAreaShiftY) {
        List<List<Point>> originalPoints = geometry.coordinates();
        if (originalPoints != null) {
            List<List<Point>> resultingPoints = new ArrayList<>(originalPoints.size());
            for (List<Point> points : originalPoints) {
                List<Point> innerPoints = new ArrayList<>();
                for (Point jsonPoint : points) {
                    PointF pointF = projection.toScreenLocation(new LatLng(jsonPoint.latitude(), jsonPoint.longitude()));
                    pointF.x -= moveDistancesObject.getDistanceXSinceLast();
                    pointF.y -= moveDistancesObject.getDistanceYSinceLast();

                    LatLng latLng = projection.fromScreenLocation(pointF);
                    if (latLng.getLatitude() > MAX_MERCATOR_LATITUDE || latLng.getLatitude() < MIN_MERCATOR_LATITUDE) {
                        return null;
                    }
                    innerPoints.add(Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude()));
                }
                resultingPoints.add(innerPoints);
            }

            return Polygon.fromLngLats(resultingPoints);
        }

        return null;
    }

    @Override
    String getName() {
        return "Fill";
    }
}
