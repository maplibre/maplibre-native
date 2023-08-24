package org.maplibre.android.annotations;

import androidx.annotation.Nullable;

import com.google.gson.JsonArray;

class ConvertUtils {

  @Nullable
  static JsonArray convertArray(Float[] value) {
    if (value != null) {
      JsonArray jsonArray = new JsonArray();
      for (Float element : value) {
        jsonArray.add(element);
      }
      return jsonArray;
    } else {
      return null;
    }
  }

  @Nullable
  static JsonArray convertArray(String[] value) {
    if (value != null) {
      JsonArray jsonArray = new JsonArray();
      for (String element : value) {
        jsonArray.add(element);
      }
      return jsonArray;
    } else {
      return null;
    }
  }

  @Nullable
  static Float[] toFloatArray(JsonArray jsonArray) {
    if (jsonArray != null) {
      Float[] array = new Float[jsonArray.size()];
      for (int i = 0; i < jsonArray.size(); i++) {
        array[i] = jsonArray.get(i).getAsFloat();
      }
      return array;
    } else {
      return null;
    }
  }

  @Nullable
  static String[] toStringArray(JsonArray jsonArray) {
    if (jsonArray != null) {
      String[] array = new String[jsonArray.size()];
      for (int i = 0; i < jsonArray.size(); i++) {
        array[i] = jsonArray.get(i).getAsString();
      }
      return array;
    } else {
      return null;
    }
  }
}
