package org.maplibre.android.style.layers;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import org.maplibre.android.geometry.LatLng;

/**
 * A convenience layer for rendering a 3D glTF/GLB model on the map.
 * <p>
 * This layer uses {@link CustomDrawableLayer} under the hood with a native
 * {@code GltfModelLayerHost} that loads and renders the model at a given
 * geographic position.
 * </p>
 *
 * <pre>
 * // Load from raw bytes (e.g. read from assets):
 * byte[] data = getAssets().open("model.glb").readAllBytes();
 * GltfModelLayer layer = new GltfModelLayer(
 *     "my-model",
 *     data,
 *     new LatLng(37.78, -122.41),
 *     0.1f
 * );
 * maplibreMap.getStyle().addLayer(layer);
 * </pre>
 */
public class GltfModelLayer extends CustomDrawableLayer {

    /**
     * Create a new glTF model layer from a file path on device storage.
     *
     * @param id        unique layer id
     * @param modelPath absolute path to a .gltf or .glb file on device storage
     * @param position  geographic location to place the model
     * @param scale     scale factor (model-space units to map units)
     */
    public GltfModelLayer(@NonNull String id,
                          @NonNull String modelPath,
                          @NonNull LatLng position,
                          float scale) {
        this(id, modelPath, position, scale, 0f, 0f, 0f);
    }

    /**
     * Create a new glTF model layer from a file path with rotation.
     *
     * @param id        unique layer id
     * @param modelPath absolute path to a .gltf or .glb file on device storage
     * @param position  geographic location to place the model
     * @param scale     scale factor
     * @param rotationX rotation around X axis in degrees
     * @param rotationY rotation around Y axis in degrees
     * @param rotationZ rotation around Z axis in degrees
     */
    public GltfModelLayer(@NonNull String id,
                          @NonNull String modelPath,
                          @NonNull LatLng position,
                          float scale,
                          float rotationX,
                          float rotationY,
                          float rotationZ) {
        this(id, modelPath, position, scale, rotationX, rotationY, rotationZ, false, false, false);
    }

    /**
     * Create a new glTF model layer from a file path with rotation and optional axis mirroring.
     * Mirroring flips the model by negating scale on the chosen axes (before rotation is applied).
     *
     * @param mirrorX if true, negate scale on the first horizontal (map-plane) axis
     * @param mirrorY if true, negate scale on the second horizontal (map-plane) axis
     * @param mirrorZ if true, negate scale on the vertical axis
     */
    public GltfModelLayer(@NonNull String id,
                          @NonNull String modelPath,
                          @NonNull LatLng position,
                          float scale,
                          float rotationX,
                          float rotationY,
                          float rotationZ,
                          boolean mirrorX,
                          boolean mirrorY,
                          boolean mirrorZ) {
        super(id, nativeCreateHost(
                modelPath,
                position.getLatitude(),
                position.getLongitude(),
                scale,
                rotationX,
                rotationY,
                rotationZ,
                mirrorX,
                mirrorY,
                mirrorZ));
    }

    /**
     * Create a new glTF model layer from in-memory model data.
     *
     * @param id        unique layer id
     * @param modelData raw bytes of a .gltf or .glb model
     * @param position  geographic location to place the model
     * @param scale     scale factor (model-space units to map units)
     */
    public GltfModelLayer(@NonNull String id,
                          @NonNull byte[] modelData,
                          @NonNull LatLng position,
                          float scale) {
        this(id, modelData, position, scale, 0f, 0f, 0f);
    }

    /**
     * Create a new glTF model layer from in-memory model data with rotation.
     *
     * @param id        unique layer id
     * @param modelData raw bytes of a .gltf or .glb model
     * @param position  geographic location to place the model
     * @param scale     scale factor
     * @param rotationX rotation around X axis in degrees
     * @param rotationY rotation around Y axis in degrees
     * @param rotationZ rotation around Z axis in degrees
     */
    public GltfModelLayer(@NonNull String id,
                          @NonNull byte[] modelData,
                          @NonNull LatLng position,
                          float scale,
                          float rotationX,
                          float rotationY,
                          float rotationZ) {
        this(id, modelData, position, scale, rotationX, rotationY, rotationZ, false, false, false);
    }

    /**
     * Create a new glTF model layer from in-memory model data with rotation and optional axis mirroring.
     *
     * @param mirrorX if true, negate scale on the first horizontal (map-plane) axis
     * @param mirrorY if true, negate scale on the second horizontal (map-plane) axis
     * @param mirrorZ if true, negate scale on the vertical axis
     */
    public GltfModelLayer(@NonNull String id,
                          @NonNull byte[] modelData,
                          @NonNull LatLng position,
                          float scale,
                          float rotationX,
                          float rotationY,
                          float rotationZ,
                          boolean mirrorX,
                          boolean mirrorY,
                          boolean mirrorZ) {
        super(id, nativeCreateHostFromData(
                modelData,
                position.getLatitude(),
                position.getLongitude(),
                scale,
                rotationX,
                rotationY,
                rotationZ,
                mirrorX,
                mirrorY,
                mirrorZ));
    }

    @Keep
    private static native long nativeCreateHost(String modelPath,
                                                double latitude,
                                                double longitude,
                                                float scale,
                                                float rotationX,
                                                float rotationY,
                                                float rotationZ,
                                                boolean mirrorX,
                                                boolean mirrorY,
                                                boolean mirrorZ);

    @Keep
    private static native long nativeCreateHostFromData(byte[] modelData,
                                                        double latitude,
                                                        double longitude,
                                                        float scale,
                                                        float rotationX,
                                                        float rotationY,
                                                        float rotationZ,
                                                        boolean mirrorX,
                                                        boolean mirrorY,
                                                        boolean mirrorZ);
}
