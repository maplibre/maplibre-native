package org.maplibre.android.style.layers;

import androidx.annotation.NonNull;

public class LayoutPropertyValue<T> extends PropertyValue<T> {

  public LayoutPropertyValue(@NonNull String name, T value) {
    super(name, value);
  }

}
