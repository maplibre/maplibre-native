package org.maplibre.android.location

import android.content.Context
import org.maplibre.android.location.engine.LocationEngine
import org.maplibre.android.location.engine.LocationEngineRequest
import org.maplibre.android.maps.Style

/**
 * A class which holds the various options for activating the Maps SDK's [LocationComponent] to eventually
 * show the device location on the map.
 */
class LocationComponentActivationOptions private constructor(
    private val context: Context,
    private val style: Style,
    private val locationEngine: LocationEngine?,
    private val locationEngineRequest: LocationEngineRequest?,
    private val locationComponentOptions: LocationComponentOptions?,
    private val styleRes: Int,
    private val useDefaultLocationEngine: Boolean,
    private val useSpecializedLocationLayer: Boolean,
) {
    /**
     * The application's current context
     *
     * @return the application's current context
     */
    fun context(): Context = context

    /**
     * The proxy object for current map style. More info at [Style]
     *
     * @return the map's fully loaded Style object
     */
    fun style(): Style = style

    /**
     * The [LocationEngine] which the [LocationComponent] should use
     *
     * @return the engine
     */
    fun locationEngine(): LocationEngine? = locationEngine

    /**
     * The location request which the [LocationComponent] should use
     *
     * @return the LocationEngineRequest object
     */
    fun locationEngineRequest(): LocationEngineRequest? = locationEngineRequest

    /**
     * A built [LocationComponentOptions] object, which holds the various [LocationComponent] styling options
     *
     * @return the options for styling the actual LocationComponent
     */
    fun locationComponentOptions(): LocationComponentOptions? = locationComponentOptions

    /**
     * The LocationComponent style resource (e.g. R.style.style_name)
     *
     * @return the style resource.
     */
    fun styleRes(): Int = styleRes

    /**
     * True if you want to initialize and use the built-in location engine or false if there should be no
     * location engine initialized
     *
     * @return whether the default LocationEngine is used
     */
    fun useDefaultLocationEngine(): Boolean = useDefaultLocationEngine

    /**
     * True if you want to initialize and use the specialized location layer to render the location puck.
     *
     * The specialized layer is not based on runtime styling. This brings significant performance improvements,
     * but since it's not based on the runtime styling,
     * it's not fully compatible with the traditional implementation. The incompatibilities are:
     *
     *  - Constants like [LocationComponentConstants.BACKGROUND_LAYER], [LocationComponentConstants.ACCURACY_LAYER]
     *    or [LocationComponentConstants.LOCATION_SOURCE] are ignored.
     *    The only usable and valid for relative positioning layer ID is
     *    [LocationComponentConstants.FOREGROUND_LAYER].
     *  - All options that alter the image ID, like [LocationComponentOptions.foregroundName], are ignored.
     *    Use [LocationComponentOptions.foregroundDrawable] to alter the image rendered as the puck.
     *  - The LocationComponent's pulsing effect. Any of the [LocationComponentOptions]'
     *    pulsing methods such as [LocationComponentOptions.pulseEnabled] or
     *    [LocationComponentOptions.pulseColor], are useless when using the
     *    the specialized location layer.
     *
     * @return whether the default specialized location layer is used
     */
    fun useSpecializedLocationLayer(): Boolean = useSpecializedLocationLayer

    /**
     * Builder class for constructing a new instance of [LocationComponentActivationOptions].
     *
     * Constructor for the [LocationComponentActivationOptions] builder class.
     * While other activation options are optional, the activation process always requires
     * context and a fully-loaded map [Style] object, which is why the two are in this
     * constructor.
     */
    class Builder(
        private val context: Context,
        private val style: Style,
    ) {
        private var locationEngine: LocationEngine? = null
        private var locationEngineRequest: LocationEngineRequest? = null
        private var locationComponentOptions: LocationComponentOptions? = null
        private var styleRes = 0

        /**
         * Set to true as the default in case a true/false value isn't declared via the builder's
         * [LocationComponentActivationOptions.useDefaultLocationEngine] method.
         *
         * Please be aware that this activation boolean is ignored when a non-null
         * [LocationEngine] is set via the builder's `locationEngine()` method.
         */
        private var useDefaultLocationEngine = true

        private var useSpecializedLocationLayer = false

        /**
         * Deliver your own [LocationEngine] to the LocationComponent.
         *
         * The true/false [useDefaultLocationEngine] activation option is ignored when a non-null
         * [LocationEngine] is set via this `locationEngine()` method.
         *
         * @param locationEngine a [LocationEngine] object
         * @return the [Builder] object being constructed
         */
        fun locationEngine(locationEngine: LocationEngine?): Builder {
            this.locationEngine = locationEngine
            return this
        }

        /**
         * @param locationEngineRequest the location request which the [LocationComponent] should use
         * @return the [Builder] object being constructed
         */
        fun locationEngineRequest(locationEngineRequest: LocationEngineRequest?): Builder {
            this.locationEngineRequest = locationEngineRequest
            return this
        }

        /**
         * @param locationComponentOptions a built [LocationComponentOptions] object,
         *                                 which holds the various [LocationComponent] styling options
         * @return the [Builder] object being constructed
         */
        fun locationComponentOptions(locationComponentOptions: LocationComponentOptions?): Builder {
            this.locationComponentOptions = locationComponentOptions
            return this
        }

        /**
         * @param styleRes the LocationComponent style resource (e.g. R.style.style_name)
         * @return the [Builder] object being constructed
         */
        fun styleRes(styleRes: Int): Builder {
            this.styleRes = styleRes
            return this
        }

        /**
         * @param useDefaultLocationEngine true if you want to initialize and use the
         *                                 built-in location engine or false if there
         *                                 should be no location engine initialized
         *                                 This is ignored when null is set as the parameter
         *                                 for [locationEngine].
         * @return the [Builder] object being constructed
         */
        fun useDefaultLocationEngine(useDefaultLocationEngine: Boolean): Builder {
            this.useDefaultLocationEngine = useDefaultLocationEngine
            return this
        }

        /**
         * True if you want to initialize and use the specialized location layer to render the location puck.
         *
         * The specialized layer is not based on runtime styling. This brings significant performance improvements,
         * but since it's not based on the runtime styling,
         * it's not fully compatible with the traditional implementation. The incompatibilities are:
         *
         *  - Constants like [LocationComponentConstants.BACKGROUND_LAYER], [LocationComponentConstants.ACCURACY_LAYER]
         *    or [LocationComponentConstants.LOCATION_SOURCE] are ignored.
         *    The only usable and valid for relative positioning layer ID is
         *    [LocationComponentConstants.FOREGROUND_LAYER].
         *  - All options that alter the image ID, like [LocationComponentOptions.foregroundName], are ignored.
         *    Use [LocationComponentOptions.foregroundDrawable] to alter the image rendered as the puck.
         *  - The LocationComponent's pulsing effect. Any of the [LocationComponentOptions]'
         *    pulsing methods such as [LocationComponentOptions.pulseEnabled] or
         *    [LocationComponentOptions.pulseColor], are useless when using the
         *    the specialized location layer.
         *
         * @param useSpecializedLocationLayer true if you want to initialize and use the
         *                                    specialized location layer. Defaults to false.
         * @return the [Builder] object being constructed
         */
        fun useSpecializedLocationLayer(useSpecializedLocationLayer: Boolean): Builder {
            this.useSpecializedLocationLayer = useSpecializedLocationLayer
            return this
        }

        /**
         * Method which actually builds the [LocationComponentActivationOptions] object while
         * taking the various options into account.
         *
         * @return a built [LocationComponentActivationOptions] object
         */
        fun build(): LocationComponentActivationOptions {
            require(styleRes == 0 || locationComponentOptions == null) {
                "You've provided both a style resource and a LocationComponentOptions object to the " +
                    "LocationComponentActivationOptions builder. You can't use both and " +
                    "you must choose one of the two to style the LocationComponent."
            }
            require(style.isFullyLoaded) {
                "Style in LocationComponentActivationOptions isn't fully loaded. Wait for the " +
                    "map to fully load before passing the Style object to " +
                    "LocationComponentActivationOptions."
            }
            return LocationComponentActivationOptions(
                context,
                style,
                locationEngine,
                locationEngineRequest,
                locationComponentOptions,
                styleRes,
                useDefaultLocationEngine,
                useSpecializedLocationLayer,
            )
        }
    }

    companion object {
        /**
         * Convenience method to retrieve a [LocationComponentActivationOptions] object which is ready to build with
         *
         * @return a builder object
         */
        @JvmStatic
        fun builder(
            context: Context,
            fullyLoadedMapStyle: Style,
        ): Builder = Builder(context, fullyLoadedMapStyle)
    }
}
