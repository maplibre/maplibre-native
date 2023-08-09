package org.maplibre.android.annotations;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.mapbox.geojson.Feature;
import com.mapbox.geojson.Point;
import com.mapbox.geojson.Polygon;
import org.maplibre.android.geometry.LatLng;
import org.maplibre.android.style.layers.PropertyFactory;

import java.util.ArrayList;
import java.util.List;

/**
 * Builder class from which a fill is created.
 */
public class FillOptions extends Options<Fill> {

    private boolean isDraggable;
    private JsonElement data;
    private Polygon geometry;
    private Float fillOpacity;
    private String fillColor;
    private String fillOutlineColor;
    private String fillPattern;

    static final String PROPERTY_FILL_OPACITY = "fill-opacity";
    static final String PROPERTY_FILL_COLOR = "fill-color";
    static final String PROPERTY_FILL_OUTLINE_COLOR = "fill-outline-color";
    static final String PROPERTY_FILL_PATTERN = "fill-pattern";
    private static final String PROPERTY_IS_DRAGGABLE = "is-draggable";

    /**
     * Set fill-opacity to initialise the fill with.
     * <p>
     * The opacity of the entire fill layer. In contrast to the {@link PropertyFactory#fillColor}, this value will also affect the 1px stroke around the fill, if the stroke is used.
     * </p>
     *
     * @param fillOpacity the fill-opacity value
     * @return this
     */
    public FillOptions withFillOpacity(Float fillOpacity) {
        this.fillOpacity = fillOpacity;
        return this;
    }

    /**
     * Get the current configured  fill-opacity for the fill
     * <p>
     * The opacity of the entire fill layer. In contrast to the {@link PropertyFactory#fillColor}, this value will also affect the 1px stroke around the fill, if the stroke is used.
     * </p>
     *
     * @return fillOpacity value
     */
    public Float getFillOpacity() {
        return fillOpacity;
    }

    /**
     * Set fill-color to initialise the fill with.
     * <p>
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     * </p>
     *
     * @param fillColor the fill-color value
     * @return this
     */
    public FillOptions withFillColor(String fillColor) {
        this.fillColor = fillColor;
        return this;
    }

    /**
     * Get the current configured  fill-color for the fill
     * <p>
     * The color of the filled part of this layer. This color can be specified as `rgba` with an alpha component and the color's opacity will not affect the opacity of the 1px stroke, if it is used.
     * </p>
     *
     * @return fillColor value
     */
    public String getFillColor() {
        return fillColor;
    }

    /**
     * Set fill-outline-color to initialise the fill with.
     * <p>
     * The outline color of the fill. Matches the value of {@link PropertyFactory#fillColor} if unspecified.
     * </p>
     *
     * @param fillOutlineColor the fill-outline-color value
     * @return this
     */
    public FillOptions withFillOutlineColor(String fillOutlineColor) {
        this.fillOutlineColor = fillOutlineColor;
        return this;
    }

    /**
     * Get the current configured  fill-outline-color for the fill
     * <p>
     * The outline color of the fill. Matches the value of {@link PropertyFactory#fillColor} if unspecified.
     * </p>
     *
     * @return fillOutlineColor value
     */
    public String getFillOutlineColor() {
        return fillOutlineColor;
    }

    /**
     * Set fill-pattern to initialise the fill with.
     * <p>
     * Name of image in sprite to use for drawing image fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     * </p>
     *
     * @param fillPattern the fill-pattern value
     * @return this
     */
    public FillOptions withFillPattern(String fillPattern) {
        this.fillPattern = fillPattern;
        return this;
    }

    /**
     * Get the current configured  fill-pattern for the fill
     * <p>
     * Name of image in sprite to use for drawing image fills. For seamless patterns, image width and height must be a factor of two (2, 4, 8, ..., 512). Note that zoom-dependent expressions will be evaluated only at integer zoom levels.
     * </p>
     *
     * @return fillPattern value
     */
    public String getFillPattern() {
        return fillPattern;
    }

    /**
     * Set a list of lists of LatLng for the fill, which represents the locations of the fill on the map
     *
     * @param latLngs a list of a lists of the locations of the line in a longitude and latitude pairs
     * @return this
     */
    public FillOptions withLatLngs(List<List<LatLng>> latLngs) {
        List<List<Point>> points = new ArrayList<>();
        for (List<LatLng> innerLatLngs : latLngs) {
            List<Point> innerList = new ArrayList<>();
            for (LatLng latLng : innerLatLngs) {
                innerList.add(Point.fromLngLat(latLng.getLongitude(), latLng.getLatitude()));
            }
            points.add(innerList);
        }
        geometry = Polygon.fromLngLats(points);
        return this;
    }

    /**
     * Get a list of lists of LatLng for the fill, which represents the locations of the fill on the map
     *
     * @return a list of a lists of the locations of the line in a longitude and latitude pairs
     */
    public List<List<LatLng>> getLatLngs() {
        List<List<LatLng>> points = new ArrayList<>();
        if (geometry != null) {
            for (List<Point> coordinates : geometry.coordinates()) {
                List<LatLng> innerList = new ArrayList<>();
                for (Point point : coordinates) {
                    innerList.add(new LatLng(point.latitude(), point.longitude()));
                }
                points.add(innerList);
            }
        }
        return points;
    }

    /**
     * Set the geometry of the fill, which represents the location of the fill on the map
     *
     * @param geometry the location of the fill
     * @return this
     */
    public FillOptions withGeometry(Polygon geometry) {
        this.geometry = geometry;
        return this;
    }

    /**
     * Get the geometry of the fill, which represents the location of the fill on the map
     *
     * @return the location of the fill
     */
    public Polygon getGeometry() {
        return geometry;
    }

    /**
     * Returns whether this fill is draggable, meaning it can be dragged across the screen when touched and moved.
     *
     * @return draggable when touched
     */
    public boolean getDraggable() {
        return isDraggable;
    }

    /**
     * Set whether this fill should be draggable,
     * meaning it can be dragged across the screen when touched and moved.
     *
     * @param draggable should be draggable
     */
    public FillOptions withDraggable(boolean draggable) {
        isDraggable = draggable;
        return this;
    }

    /**
     * Set the arbitrary json data of the annotation.
     *
     * @param jsonElement the arbitrary json element data
     */
    public FillOptions withData(@Nullable JsonElement jsonElement) {
        this.data = jsonElement;
        return this;
    }

    /**
     * Get the arbitrary json data of the annotation.
     *
     * @return the arbitrary json object data if set, else null
     */
    @Nullable
    public JsonElement getData() {
        return data;
    }

    @Override
    Fill build(long id, AnnotationManager<?, Fill, ?, ?, ?, ?> annotationManager) {
        if (geometry == null) {
            throw new RuntimeException("geometry field is required");
        }
        JsonObject jsonObject = new JsonObject();
        jsonObject.addProperty(PROPERTY_FILL_OPACITY, fillOpacity);
        jsonObject.addProperty(PROPERTY_FILL_COLOR, fillColor);
        jsonObject.addProperty(PROPERTY_FILL_OUTLINE_COLOR, fillOutlineColor);
        jsonObject.addProperty(PROPERTY_FILL_PATTERN, fillPattern);
        Fill fill = new Fill(id, annotationManager, jsonObject, geometry);
        fill.setDraggable(isDraggable);
        fill.setData(data);
        return fill;
    }

    /**
     * Creates FillOptions out of a Feature.
     *
     * @param feature feature to be converted
     */
    @Nullable
    static FillOptions fromFeature(@NonNull Feature feature) {
        if (feature.geometry() == null) {
            throw new RuntimeException("geometry field is required");
        }
        if (!(feature.geometry() instanceof Polygon)) {
            return null;
        }

        FillOptions options = new FillOptions();
        options.geometry = (Polygon) feature.geometry();
        if (feature.hasProperty(PROPERTY_FILL_OPACITY)) {
            options.fillOpacity = feature.getProperty(PROPERTY_FILL_OPACITY).getAsFloat();
        }
        if (feature.hasProperty(PROPERTY_FILL_COLOR)) {
            options.fillColor = feature.getProperty(PROPERTY_FILL_COLOR).getAsString();
        }
        if (feature.hasProperty(PROPERTY_FILL_OUTLINE_COLOR)) {
            options.fillOutlineColor = feature.getProperty(PROPERTY_FILL_OUTLINE_COLOR).getAsString();
        }
        if (feature.hasProperty(PROPERTY_FILL_PATTERN)) {
            options.fillPattern = feature.getProperty(PROPERTY_FILL_PATTERN).getAsString();
        }
        if (feature.hasProperty(PROPERTY_IS_DRAGGABLE)) {
            options.isDraggable = feature.getProperty(PROPERTY_IS_DRAGGABLE).getAsBoolean();
        }
        return options;
    }
}
