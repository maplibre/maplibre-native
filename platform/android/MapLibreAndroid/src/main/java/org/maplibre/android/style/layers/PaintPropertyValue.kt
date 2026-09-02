package org.maplibre.android.style.layers;

import androidx.annotation.NonNull;

public class PaintPropertyValue<T> extends PropertyValue<T> {

  public PaintPropertyValue(@NonNull String name, T value) {
    super(name, value);
  }

}
