package org.maplibre.android.exceptions

import org.maplibre.android.annotations.IconFactory

/**
 * A TooManyIconsException is thrown by IconFactory when it
 * cannot create a Icon because there are already too many icons created.
 *
 * You should try to reuse Icon objects whenever possible.
 *
 * @see IconFactory
 */
class TooManyIconsException :
    RuntimeException("Cannot create an Icon because there are already too many. Try reusing Icon objects whenever possible.")
